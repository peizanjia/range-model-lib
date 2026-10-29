#include "rml_structural_piezo_axial_setup.h"
#include "rml_model.h"
#include <rbl_error.h>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <cmath>
#include <set>
#include <tuple>

namespace {
constexpr int maximumRecipe = 8*1024*1024;
constexpr int maximumRuntime = 32*1024*1024;
constexpr uint maximumValues = 1000000;
constexpr uint maximumContainer = 100000;
constexpr uint maximumDepth = 64;
constexpr int maximumString = 65536;
void require(bool ok,const char *message)
{ if(!ok) throw RError(RError::Type::InvalidInput,R_ERROR_REF,message); }
void sha(const QString &value)
{ require(QRegularExpression(QStringLiteral("^[0-9a-f]{64}$")).match(value).hasMatch(),"Invalid axial piezo SHA-256 source identity."); }
QByteArray scalarBytes(const QJsonValue &value)
{
    const QByteArray array=QJsonDocument(QJsonArray{value}).toJson(QJsonDocument::Compact);
    return array.mid(1,array.size()-2);
}

//! Parse before constructing containers: Qt's object parser alone silently
//! accepts duplicate keys and cannot retain IEEE signed-zero source spelling.
//! No whitespace or alternate escapes are accepted outside canonical strings.
class RecipeReader
{
    const QByteArray &bytes;
    int position=0;
    uint values=0;
    char peek() const { return position<bytes.size()?bytes[position]:'\0'; }
    bool take(char c) { if(peek()!=c) return false; ++position; return true; }
    void expect(char c) { require(take(c),"Malformed canonical axial recipe JSON."); }
    QString string()
    {
        const int start=position; expect('"'); bool escaped=false;
        while(position<bytes.size()) {
            const unsigned char c=static_cast<unsigned char>(bytes[position++]);
            require(position-start<=maximumString*6+2,"Axial recipe string exceeds its bound.");
            if(!escaped && c=='"') {
                const QByteArray token=bytes.mid(start,position-start);
                QJsonParseError error; const auto document=QJsonDocument::fromJson("["+token+"]",&error);
                require(error.error==QJsonParseError::NoError && document.isArray() && document.array().size()==1 && document.array()[0].isString(),"Invalid axial recipe JSON string.");
                const QString result=document.array()[0].toString();
                require(result.toUtf8().size()<=maximumString && scalarBytes(result)==token,"Noncanonical axial recipe JSON string.");
                for(int i=0;i<result.size();++i) {
                    require(result[i].unicode()!=0,"Axial recipe string contains a NUL character.");
                    if(result[i].isHighSurrogate()) { require(i+1<result.size() && result[i+1].isLowSurrogate(),"Invalid axial recipe Unicode."); ++i; }
                    else require(!result[i].isLowSurrogate(),"Invalid axial recipe Unicode.");
                }
                return result;
            }
            require(c>=32,"Invalid axial recipe JSON string control character.");
            if(escaped) escaped=false; else if(c=='\\') escaped=true;
        }
        require(false,"Truncated axial recipe JSON string."); return {};
    }
    QJsonValue number()
    {
        const int start=position;
        take('-');
        if(!take('0')) {
            require(peek()>='1' && peek()<='9',"Invalid axial recipe JSON number.");
            do { ++position; } while(peek()>='0' && peek()<='9');
        }
        if(take('.')) {
            require(peek()>='0' && peek()<='9',"Invalid axial recipe JSON fraction.");
            do { ++position; } while(peek()>='0' && peek()<='9');
        }
        if(take('e') || take('E')) {
            if(!take('+')) take('-');
            require(peek()>='0' && peek()<='9',"Invalid axial recipe JSON exponent.");
            do { ++position; } while(peek()>='0' && peek()<='9');
        }
        const QByteArray token=bytes.mid(start,position-start); bool ok=false;
        const double value=token.toDouble(&ok);
        require(ok && std::isfinite(value),"Axial recipe JSON requires finite binary64 values.");
        require(token=="-0" || scalarBytes(value)==token,"Noncanonical axial recipe JSON number.");
        return value;
    }
    QJsonValue value(uint depth)
    {
        require(depth<=maximumDepth && ++values<=maximumValues,"Axial recipe JSON exceeds its depth or value bound.");
        if(peek()=='"') return string();
        if(take('{')) {
            QJsonObject object; QString previous; bool first=true; uint count=0;
            if(take('}')) return object;
            do {
                require(++count<=maximumContainer,"Axial recipe object exceeds its entry bound.");
                require(peek()=='"',"Axial recipe object requires string keys."); const QString key=string();
                require(first || previous<key,"Axial recipe keys must be unique and sorted."); previous=key; first=false;
                expect(':'); object.insert(key,value(depth+1));
            } while(take(','));
            expect('}'); return object;
        }
        if(take('[')) {
            QJsonArray array; uint count=0; if(take(']')) return array;
            do { require(++count<=maximumContainer,"Axial recipe array exceeds its entry bound."); array.append(value(depth+1)); } while(take(','));
            expect(']'); return array;
        }
        for(const auto &literal:{QByteArray("true"),QByteArray("false"),QByteArray("null")})
            if(bytes.mid(position,literal.size())==literal) { position+=literal.size(); if(literal=="null") return QJsonValue(QJsonValue::Null); return literal=="true"; }
        return number();
    }
public:
    explicit RecipeReader(const QByteArray &source):bytes(source) {}
    QJsonObject read()
    {
        require(!bytes.isEmpty() && bytes.size()<=maximumRecipe,"Axial recipe exceeds its bounded source size.");
        const auto result=value(0);
        require(position==bytes.size() && result.isObject(),"Axial recipe must be a canonical whole JSON object.");
        return result.toObject();
    }
};
void recipe(const QByteArray &bytes)
{
    const QJsonObject object=RecipeReader(bytes).read();
    const std::set<QString> keys={"family","controller","excitation","design","materials","members","ports","supports","initial","workpoint","damping",
        "units","dt","steps","backgroundForces","initialDisplacements","initialVelocities","template","replaceLegacyConditions","frame"};
    require(object.value("family").isString() && object.value("family").toString()=="axialRod","Axial recipe requires family axialRod.");
    for(auto it=object.constBegin();it!=object.constEnd();++it) {
        require(keys.count(it.key())==1,"Unknown axial recipe declaration.");
        if(it.key()=="family") continue;
        if(it.key()=="dt") { require(it.value().isDouble() && it.value().toDouble()>0,"Axial recipe dt must be a positive finite number."); continue; }
        if(it.key()=="steps") { const double n=it.value().toDouble(-1); require(it.value().isDouble() && n>=1 && n<=maximumValues && std::floor(n)==n,"Axial recipe steps must be a bounded positive integer."); continue; }
        if(it.key()=="replaceLegacyConditions") { require(it.value().isBool(),"Axial recipe condition replacement flag must be boolean."); continue; }
        if(it.key()=="materials" || it.key()=="members" || it.key()=="ports" || it.key()=="supports" || it.key()=="backgroundForces" || it.key()=="initialDisplacements" || it.key()=="initialVelocities") {
            require(it.value().isArray(),"Axial recipe physical declarations must be arrays.");
            const auto array=it.value().toArray();
            if(it.key()=="materials" || it.key()=="members") require(!array.isEmpty(),"Axial recipe materials/members declaration cannot be empty.");
            for(const auto &entry:array) require(entry.isObject() && !entry.toObject().isEmpty(),"Axial recipe physical entry must be a nonempty object.");
        }
        else require(it.value().isObject(),"Axial recipe configuration declaration must be an object.");
    }
}
bool defaultDynamicsRestart(const RStructuralDynamicsRestartData &s)
{
    const RStructuralDynamicsRestartData defaults;
    const auto fields=[](const RStructuralDynamicsRestartData &v) {
        return std::tie(v.available,v.completedSteps,v.time,v.fixedTimeStep,v.meshNodeCount,v.meshElementCount,
            v.physicalDofCount,v.meshFingerprint,v.dofFingerprint,v.systemFingerprint,v.q,v.v,v.a,v.externalForce,
            v.cumulativeExternalWork,v.cumulativeDampingDissipation,v.initialMechanicalEnergy,v.supportResponseAvailable,
            v.supportFingerprint,v.supportReactions,v.supportPowers,v.cumulativeSupportWork,v.controlAvailable,
            v.controlProtocolVersion,v.controlFingerprint,v.controlChannelIds,v.controlLastCommands,v.controlPowers,
            v.cumulativeControlWork,v.controlControllerState,v.controlDeterministicSeed);
    };
    return fields(s)==fields(defaults);
}
void source(const RStructuralPiezoAxialSetup &s,const RModel &model)
{
    if(!s.enabled) return;
    const auto &problem=model.getProblemSetup(); const auto &truss=problem.getStructuralTrussDynamicsSetup();
    require(truss.enabled && !truss.sourceFingerprint.isEmpty(),"Axial experiment requires a bound enabled native truss source.");
    truss.validate(model);
    const auto &d=problem.getStructuralDynamicsSetup(); d.validate();
    require((model.getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_STRESS)!=0 && problem.getMassMatrixType()==RMassMatrixType::Lumped,"Axial experiment requires STRESS and lumped native truss mass.");
    require(d.getModalDampingRatios().empty(),"Axial experiment damping must be declared in its recipe.");
    require(truss.heldCommands.empty(),"Axial experiment requires its own excitation/controller schedule.");
    const auto &layered=problem.getStructuralPiezoLayeredRestart(); layered.validate();
    require(!problem.getStructuralPiezoLayeredSetup().hasNonDefaultData() && !layered.available,"Axial and layered experiments are exclusive.");
    const auto &thermal=problem.getStructuralThermalControlSetup();
    require(!thermal.enabled && thermal.thermalSubstepSeconds==0 && thermal.thermalSubstepsPerMechanicalMacrostep==1 && thermal.mechanicalMacrostepsPerControllerSample==1 && thermal.channels.empty() && thermal.sparseObservationNodeIds.empty(),"Axial experiment cannot use thermal control source data.");
    const auto &control=problem.getStructuralControlSetup();
    require(!control.enabled && control.controlProtocolVersion==1 && control.controllerId.isEmpty() && control.deterministicSeed==0 && control.channelConditioning.empty() && !problem.getStructuralNativeControlSpec().hasNonDefaultData() && !problem.getStructuralNativeControlState().hasNonDefaultData(),"Axial experiment requires its own controller source and restart.");
    require(!problem.getStructuralTrussDynamicsRestartData().hasNonDefaultData() && defaultDynamicsRestart(problem.getStructuralDynamicsRestartData()),"Axial experiment requires its own complete accepted restart.");
    const auto &catalog=problem.getStructuralPiezoReadoutSetup(); catalog.validate(model);
    require(!truss.activeMembers.empty() && catalog.ports.size()==truss.activeMembers.size(),"Axial experiment requires a complete electrical readout catalog.");
    for(const auto &port:catalog.ports) if(port.role!=RStructuralPiezoPortRole::Sensor) {
        for(const auto &member:truss.activeMembers) if(member.portId==port.portId)
            require(member.kind==RStructuralTrussElectricalKind::Voltage,"Axial experiment actuators require voltage ports.");
    }
}
void blob(QByteArray &bytes,const QByteArray &value)
{ const quint64 size=quint64(value.size()); for(uint i=0;i<8;++i) bytes.append(char((size>>(8*i))&255)); bytes.append(value); }
}

bool RStructuralPiezoAxialSetup::hasNonDefaultData() const
{ return enabled || !sourceFingerprint.isEmpty() || !recipeConfiguration.isEmpty(); }
void RStructuralPiezoAxialSetup::validate() const
{
    if(!enabled) { require(!hasNonDefaultData(),"Disabled axial experiment contains hidden source data."); return; }
    if(!sourceFingerprint.isEmpty()) sha(sourceFingerprint);
    recipe(recipeConfiguration);
}
void RStructuralPiezoAxialSetup::validate(const RModel &model) const
{
    validate(); source(*this,model);
    if(!sourceFingerprint.isEmpty()) require(sourceFingerprint==computeSourceFingerprint(model),"Axial experiment is bound to changed source data.");
}
QByteArray RStructuralPiezoAxialSetup::identityBytes() const
{
    validate(); if(!hasNonDefaultData()) return {};
    QByteArray bytes; blob(bytes,"range-fea/piezo-axial/source/v1/canonical-si-json"); bytes.append(char(1)); blob(bytes,recipeConfiguration); return bytes;
}
QString RStructuralPiezoAxialSetup::computeSourceFingerprint(const RModel &model) const
{
    validate(); source(*this,model); if(!hasNonDefaultData()) return {};
    QByteArray bytes; blob(bytes,identityBytes());
    blob(bytes,model.getProblemSetup().getStructuralTrussDynamicsSetup().computeSourceFingerprint(model).toLatin1());
    return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
}
void RStructuralPiezoAxialSetup::bindToModel(const RModel &model)
{
    auto candidate=*this; candidate.sourceFingerprint.clear();
    candidate.sourceFingerprint=candidate.computeSourceFingerprint(model); candidate.validate(model); *this=std::move(candidate);
}
bool RStructuralPiezoAxialRestart::hasNonDefaultData() const
{ return available || !sourceFingerprint.isEmpty() || !runtimeState.isEmpty(); }
void RStructuralPiezoAxialRestart::validate() const
{
    require(runtimeState.size()<=maximumRuntime,"Axial restart exceeds its bounded state size.");
    if(available) { sha(sourceFingerprint); require(!runtimeState.isEmpty(),"Available axial restart requires opaque accepted runtime bytes."); }
    else require(!hasNonDefaultData(),"Unavailable axial restart contains hidden state.");
}
