#include "rml_structural_native_control.h"
#include "rml_model.h"
#include <rbl_error.h>
#include <QCryptographicHash>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <set>
#include <tuple>

namespace {
constexpr size_t maxEntries = 4096;
constexpr int maxEnvelope = 16*1024*1024;
void require(bool ok, const char *message)
{
    if (!ok) throw RError(RError::Type::InvalidInput,R_ERROR_REF,message);
}
void finite(double value) { require(std::isfinite(value),"Non-finite native control value."); }
bool sameBits(double first,double second) { return std::memcmp(&first,&second,sizeof(double))==0; }
void bounded(size_t size) { require(size <= maxEntries,"Native control catalog exceeds its bound."); }
bool digest(const QString &value)
{
    if (value.size()!=64) return false;
    for (QChar c:value) if (!((c>='0'&&c<='9')||(c>='a'&&c<='f'))) return false;
    return true;
}
void id(const QString &value)
{
    require(!value.isEmpty() && value.trimmed()==value && value.toUtf8().size()<=4096,
            "Native control identity must be nonempty, unpadded and bounded.");
    for (int i=0;i<value.size();++i) {
        const auto c=value[i];
        require(c.unicode()>=0x20 && c.unicode()!=0x7f && c!='"' && c!='\\',"Invalid native control identity character.");
        if(c.isHighSurrogate()) { require(i+1<value.size() && value[i+1].isLowSurrogate(),"Invalid native control UTF-16."); ++i; }
        else require(!c.isLowSurrogate(),"Invalid native control UTF-16.");
    }
}
bool measurement(RStructuralNativeMeasurementQuantity q) { return uint(q)<=6; }
bool action(RStructuralNativeActionQuantity q) { return q==RStructuralNativeActionQuantity::Force || q==RStructuralNativeActionQuantity::CableReferenceLength || q==RStructuralNativeActionQuantity::Voltage || q==RStructuralNativeActionQuantity::Charge; }
void target(uint node,uint member,const QString &port,uint axis,uint kind)
{
    if(kind==0) require(node!=0 && node!=RConstants::eod && member==0 && port.isEmpty() && axis<3,"Invalid native control node target.");
    else if(kind==1) require(node==0 && member!=0 && member!=RConstants::eod && port.isEmpty() && axis==0,"Invalid native control member target.");
    else { require(node==0 && member==0 && axis==0,"Invalid native control port target."); id(port); }
}
uint targetKind(RStructuralNativeMeasurementQuantity q) { return q==RStructuralNativeMeasurementQuantity::MemberAxialForce ? 1 : (q==RStructuralNativeMeasurementQuantity::PortVoltage || q==RStructuralNativeMeasurementQuantity::PortCharge ? 2 : 0); }
uint targetKind(RStructuralNativeActionQuantity q) { return q==RStructuralNativeActionQuantity::Force ? 0 : (q==RStructuralNativeActionQuantity::CableReferenceLength ? 1 : 2); }
struct Writer {
    QByteArray bytes;
    void integer(quint64 v) { for(uint i=0;i<8;++i) bytes.append(char((v>>(8*i))&255)); }
    void real(double v) { static_assert(sizeof(double)==8 && std::numeric_limits<double>::is_iec559,"IEEE binary64 required"); quint64 bits; std::memcpy(&bits,&v,8); integer(bits); }
    void blob(const QByteArray &v) { integer(quint64(v.size())); bytes.append(v); }
    void text(const QString &v) { blob(v.toUtf8()); }
};
void frameBytes(Writer &w,const RStructuralNativeControlFrame &f)
{
    w.integer(f.completedFE); w.integer(f.controlTick); w.integer(f.fePhase); w.integer(f.samplePhase);
    w.real(f.sampleTime); w.real(f.nextFETime); w.real(f.nextControlTime); w.real(f.physicalDt);
    w.integer(f.samples.size()); for(const auto &v:f.samples) { w.text(v.sensorId); w.integer(uint(v.quantity)); w.real(v.value); }
}
bool defaultFrame(const RStructuralNativeControlFrame &f) { Writer a,b; frameBytes(a,f); frameBytes(b,{}); return a.bytes==b.bytes; }
void validateFrame(const RStructuralNativeControlFrame &f)
{
    bounded(f.samples.size()); finite(f.sampleTime); finite(f.nextFETime); finite(f.nextControlTime); finite(f.physicalDt);
    require(f.fePhase==0 && f.samplePhase==0,"Unsupported native control frame phase.");
    std::set<QString> ids;
    for(const auto &v:f.samples) { id(v.sensorId); require(measurement(v.quantity) && ids.insert(v.sensorId).second,"Invalid native control frame sample."); finite(v.value); }
}
}

QString rStructuralNativeMeasurementUnit(RStructuralNativeMeasurementQuantity q)
{
    switch(q) {
    case RStructuralNativeMeasurementQuantity::NodeDisplacement:return "m";
    case RStructuralNativeMeasurementQuantity::NodeVelocity:return "m/s";
    case RStructuralNativeMeasurementQuantity::NodeAcceleration:return "m/s^2";
    case RStructuralNativeMeasurementQuantity::MemberAxialForce:case RStructuralNativeMeasurementQuantity::SupportReaction:return "N";
    case RStructuralNativeMeasurementQuantity::PortVoltage:return "V";
    case RStructuralNativeMeasurementQuantity::PortCharge:return "C";
    } require(false,"Unsupported native control measurement quantity."); return {};
}
QString rStructuralNativeActionUnit(RStructuralNativeActionQuantity q)
{
    switch(q) {
    case RStructuralNativeActionQuantity::Force:return "N";
    case RStructuralNativeActionQuantity::CableReferenceLength:return "m";
    case RStructuralNativeActionQuantity::Voltage:return "V";
    case RStructuralNativeActionQuantity::Charge:return "C";
    } require(false,"Unsupported native control action quantity."); return {};
}

QByteArray RStructuralNativeControlSpec::identityBytes() const
{
    Writer w; w.text("range-fea/native-control/spec/v1/binary64");
    w.integer(schemaVersion); w.integer(uint(source)); w.text(providerId); w.text(providerRevision); w.text(implementationSha256);
    w.blob(dependencyEnvironmentManifest); w.text(dependencyEnvironmentSha256);
    w.integer(controlEveryFESteps); w.integer(samplePhase); w.integer(deterministicSeed);
    w.integer(sensors.size()); for(const auto &s:sensors) { w.text(s.sensorId); w.integer(uint(s.quantity)); w.integer(s.physicalNodeId); w.integer(s.physicalMemberId); w.text(s.portId); w.integer(s.axis); }
    w.integer(actions.size()); for(const auto &a:actions) {
        w.text(a.actionId); w.integer(uint(a.quantity)); w.integer(a.physicalNodeId); w.integer(a.physicalMemberId); w.text(a.portId); w.integer(a.axis);
        w.real(a.minimum); w.real(a.maximum); w.real(a.rateLimit); w.real(a.initialCommand); w.integer(a.delayTicks); w.integer(a.holdTicks); w.real(a.noiseAmplitude);
    }
    w.integer(piLoops.size()); for(const auto &p:piLoops) { w.text(p.actionId); w.text(p.sensorId); w.real(p.referenceValue); w.real(p.proportionalGain); w.real(p.integralGain); w.real(p.bias); w.real(p.initialIntegral); }
    return w.bytes;
}
bool RStructuralNativeControlSpec::hasNonDefaultData() const { return identityBytes()!=RStructuralNativeControlSpec().identityBytes(); }
void RStructuralNativeControlSpec::validate() const
{
    require(schemaVersion==1,"Unsupported native control spec schema.");
    require(source==RStructuralNativeControlSource::FrozenTable || source==RStructuralNativeControlSource::LocalV3,"Unsupported native control source.");
    if(source==RStructuralNativeControlSource::FrozenTable) { require(!hasNonDefaultData(),"Frozen-table native control spec must be fully default."); return; }
    id(providerId); id(providerRevision);
    require(digest(implementationSha256) && digest(dependencyEnvironmentSha256),"Invalid native provider digest.");
    require(!dependencyEnvironmentManifest.isEmpty() && dependencyEnvironmentManifest.size()<=maxEnvelope,"Invalid native provider environment manifest.");
    require(QString::fromLatin1(QCryptographicHash::hash(dependencyEnvironmentManifest,QCryptographicHash::Sha256).toHex())==dependencyEnvironmentSha256,"Native provider environment digest mismatch.");
    require(controlEveryFESteps==1 && samplePhase==0 && deterministicSeed==0,"Unsupported native control timing or seed.");
    bounded(sensors.size()); bounded(actions.size()); bounded(piLoops.size());
    require(!sensors.empty() && !actions.empty() && piLoops.size()==actions.size(),"Native PI source requires sensors and exactly one loop per action.");
    std::set<QString> sensorIds,actionIds;
    std::set<std::tuple<uint,uint,uint,QString,uint>> targets;
    for(const auto &s:sensors) { id(s.sensorId); require(measurement(s.quantity) && sensorIds.insert(s.sensorId).second,"Duplicate or invalid native sensor."); target(s.physicalNodeId,s.physicalMemberId,s.portId,s.axis,targetKind(s.quantity)); }
    for(const auto &a:actions) {
        id(a.actionId); require(action(a.quantity) && actionIds.insert(a.actionId).second,"Duplicate or invalid native action.");
        target(a.physicalNodeId,a.physicalMemberId,a.portId,a.axis,targetKind(a.quantity));
        require(targets.emplace(targetKind(a.quantity),a.physicalNodeId,a.physicalMemberId,a.portId,a.axis).second,"Native actions alias the same physical input.");
        require(!std::isnan(a.minimum) && a.minimum!=std::numeric_limits<double>::infinity() && !std::isnan(a.maximum) && a.maximum!=-std::numeric_limits<double>::infinity() && a.minimum<=a.maximum,"Invalid native action amplitude bounds.");
        require(!std::isnan(a.rateLimit) && a.rateLimit>=0.0,"Invalid native action rate bound."); finite(a.initialCommand); finite(a.noiseAmplitude);
        require(a.initialCommand>=a.minimum && a.initialCommand<=a.maximum,"Native initial command violates its bounds.");
        require(a.delayTicks==0 && a.holdTicks==1 && a.noiseAmplitude==0.0,"Unsupported native conditioning option.");
        if(a.quantity==RStructuralNativeActionQuantity::Force) require(a.initialCommand==0.0,"Native force is an increment initialized to zero.");
        if(a.quantity==RStructuralNativeActionQuantity::CableReferenceLength) require(a.initialCommand>0.0,"Native cable reference length must be positive.");
    }
    std::set<QString> loops;
    for(size_t i=0;i<piLoops.size();++i) {
        const auto &p=piLoops[i]; id(p.actionId); id(p.sensorId);
        require(actionIds.count(p.actionId) && sensorIds.count(p.sensorId) && loops.insert(p.actionId).second,"PI loops must uniquely cover the action catalog and reference existing sensors.");
        finite(p.referenceValue); finite(p.proportionalGain); finite(p.integralGain); finite(p.bias); finite(p.initialIntegral);
    }
}
void RStructuralNativeControlSpec::validate(const RModel &model) const
{
    validate(); if(source==RStructuralNativeControlSource::FrozenTable) return;
    const auto &truss=model.getProblemSetup().getStructuralTrussDynamicsSetup();
    require(truss.enabled && truss.heldCommands.empty(),"Local native control requires enabled native trusses and an empty frozen schedule.");
    std::set<uint> nodes,members,cables; std::map<QString,const RStructuralTrussActiveMember*> ports;
    for(uint i=0;i<model.getNNodes();++i) nodes.insert(model.getNodePhysicalId(i));
    for(const auto &m:truss.passiveMembers) members.insert(m.physicalElementId);
    for(const auto &m:truss.activeMembers) { members.insert(m.physicalElementId); ports.emplace(m.portId,&m); }
    for(const auto &m:truss.cableMembers) { members.insert(m.physicalElementId); cables.insert(m.physicalElementId); }
    std::set<uint> physicalMembers;
    for(uint i=0;i<model.getNElements();++i) if(model.getElement(i).getType()==R_ELEMENT_TRUSS1) physicalMembers.insert(model.getElementPhysicalId(i));
    const auto check=[&](uint node,uint member,const QString &port,uint kind) {
        require(kind==0 ? nodes.count(node)>0 : (kind==1 ? members.count(member)>0 && physicalMembers.count(member)>0 : ports.count(port)>0 && physicalMembers.count(ports.at(port)->physicalElementId)>0),"Native control target is missing from the physical source.");
    };
    for(const auto &s:sensors) check(s.physicalNodeId,s.physicalMemberId,s.portId,targetKind(s.quantity));
    for(const auto &a:actions) {
        check(a.physicalNodeId,a.physicalMemberId,a.portId,targetKind(a.quantity));
        if(a.quantity==RStructuralNativeActionQuantity::CableReferenceLength) {
            require(cables.count(a.physicalMemberId)>0,"Reference-length action must own a cable.");
            for(const auto &m:truss.cableMembers) if(m.physicalElementId==a.physicalMemberId) require(sameBits(a.initialCommand,m.initialReferenceLength),"Native cable action initial value differs from source.");
        } else if(targetKind(a.quantity)==2) {
            const auto &m=*ports.at(a.portId);
            require((a.quantity==RStructuralNativeActionQuantity::Voltage && m.kind==RStructuralTrussElectricalKind::Voltage) || (a.quantity==RStructuralNativeActionQuantity::Charge && m.kind==RStructuralTrussElectricalKind::ConductorCharge),"Native electrical action conflicts with the frozen port mode.");
            require(sameBits(a.initialCommand,m.initialValue),"Native electrical action initial value differs from source.");
        }
    }
}

void RStructuralNativeControlState::clear() { *this=RStructuralNativeControlState(); }
QByteArray RStructuralNativeControlState::fingerprintBytes() const
{
    Writer w; w.text("range-fea/native-control/state/v1/binary64"); w.integer(available); w.integer(schemaVersion); w.text(sourceFingerprint); w.text(providerRevision);
    w.integer(completedFE); w.integer(completedControlTicks); w.integer(fePhase); w.integer(samplePhase);
    w.blob(lastControllerInputState); w.blob(controllerState); frameBytes(w,lastFrame);
    w.integer(lastActions.size()); for(const auto &a:lastActions) { w.text(a.actionId); w.integer(uint(a.quantity)); w.real(a.previousApplied); w.real(a.requested); w.real(a.conditioned); w.real(a.applied); w.integer(a.limitFlags); }
    w.integer(heldValues.size()); for(double v:heldValues) w.real(v);
    w.integer(lastAppliedValues.size()); for(double v:lastAppliedValues) w.real(v);
    return w.bytes;
}
bool RStructuralNativeControlState::hasNonDefaultData() const { return fingerprintBytes()!=RStructuralNativeControlState().fingerprintBytes(); }
void RStructuralNativeControlState::validate() const
{
    require(schemaVersion==1,"Unsupported native control state schema.");
    if(!available) { require(!hasNonDefaultData(),"Unavailable native control state must be fully default."); return; }
    require(digest(sourceFingerprint),"Invalid native control state source fingerprint."); id(providerRevision);
    require(completedFE==completedControlTicks && fePhase==0 && samplePhase==0,"Invalid native N=1 control counters or phases.");
    require(!controllerState.isEmpty() && controllerState.size()<=maxEnvelope && lastControllerInputState.size()<=maxEnvelope,"Invalid native controller envelope.");
    bounded(lastActions.size()); bounded(heldValues.size()); bounded(lastAppliedValues.size());
    for(double v:heldValues) finite(v); for(double v:lastAppliedValues) finite(v); validateFrame(lastFrame);
    std::set<QString> ids;
    for(const auto &a:lastActions) { id(a.actionId); require(action(a.quantity) && ids.insert(a.actionId).second && (a.limitFlags&~quint32(3))==0,"Invalid native action record."); finite(a.previousApplied); finite(a.requested); finite(a.conditioned); finite(a.applied); }
    if(completedFE==0) require(lastControllerInputState.isEmpty() && defaultFrame(lastFrame) && lastActions.empty(),"Zero-step native state retains a previous proposal.");
    else require(!lastControllerInputState.isEmpty() && lastFrame.completedFE==completedFE-1 && lastFrame.controlTick==completedControlTicks-1 && lastFrame.physicalDt>0.0,"Invalid native accepted frame or input envelope.");
}
void RStructuralNativeControlState::validate(const RStructuralNativeControlSpec &spec) const
{
    spec.validate(); validate();
    if(spec.source==RStructuralNativeControlSource::FrozenTable) { require(!hasNonDefaultData(),"Frozen-table source cannot retain native controller state."); return; }
    if(!available) return;
    require(providerRevision==spec.providerRevision,"Native state provider revision differs from its spec.");
    require(heldValues.size()==spec.actions.size() && lastAppliedValues.size()==spec.actions.size(),"Native held input catalog differs from its spec.");
    for(size_t i=0;i<spec.actions.size();++i) {
        const auto &a=spec.actions[i];
        require(sameBits(heldValues[i],lastAppliedValues[i]) && heldValues[i]>=a.minimum && heldValues[i]<=a.maximum,"Native held input violates source bounds.");
        if(a.quantity==RStructuralNativeActionQuantity::CableReferenceLength) require(heldValues[i]>0.0,"Invalid held cable reference length.");
        if(completedFE==0) require(sameBits(heldValues[i],a.initialCommand),"Zero-step native held command differs from source initialization.");
    }
    if(completedFE==0) return;
    require(lastFrame.samples.size()==spec.sensors.size() && lastActions.size()==spec.actions.size(),"Native last proposal catalog differs from source.");
    for(size_t i=0;i<spec.sensors.size();++i) require(lastFrame.samples[i].sensorId==spec.sensors[i].sensorId && lastFrame.samples[i].quantity==spec.sensors[i].quantity,"Native frame sensor order differs from source.");
    for(size_t i=0;i<spec.actions.size();++i) {
        const auto &a=spec.actions[i]; const auto &r=lastActions[i];
        require(r.actionId==a.actionId && r.quantity==a.quantity && sameBits(r.applied,lastAppliedValues[i]) && sameBits(r.conditioned,r.requested),"Native action record differs from held input catalog.");
        require(r.previousApplied>=a.minimum && r.previousApplied<=a.maximum,"Native previous applied command violates amplitude bounds.");
        if(a.quantity==RStructuralNativeActionQuantity::CableReferenceLength) require(r.previousApplied>0.0 && r.applied>0.0,"Invalid accepted cable reference length.");
        const double clipped=std::max(a.minimum,std::min(a.maximum,r.requested));
        quint32 flags=clipped!=r.requested ? R_NATIVE_CONTROL_AMPLITUDE_LIMITED : R_NATIVE_CONTROL_NOT_LIMITED;
        double conditioned=clipped;
        if(std::isfinite(a.rateLimit)) {
            const double delta=a.rateLimit*lastFrame.physicalDt; finite(delta);
            const double low=std::max(a.minimum,r.previousApplied-delta);
            const double high=std::min(a.maximum,r.previousApplied+delta);
            conditioned=std::max(low,std::min(high,clipped));
            if(conditioned!=clipped) flags|=R_NATIVE_CONTROL_RATE_LIMITED;
        }
        require(r.applied==conditioned && r.limitFlags==flags,"Native action conditioning value or limit bits are inconsistent.");
    }
}
