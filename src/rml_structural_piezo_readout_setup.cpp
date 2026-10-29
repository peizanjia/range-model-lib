#include "rml_structural_piezo_readout_setup.h"
#include "rml_model.h"
#include <rbl_error.h>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <set>

namespace {
constexpr size_t maximumCount = 1000000u;
void require(bool ok, const char *message)
{
    if (!ok) throw RError(RError::Type::InvalidInput,R_ERROR_REF,message);
}
void bounded(size_t n) { require(n<=maximumCount,"Piezo readout catalog exceeds its bounded count."); }
void finite(double v) { require(std::isfinite(v),"Piezo readout source contains a non-finite value."); }
void id(const QString &v)
{
    require(!v.isEmpty() && v.trimmed()==v && v.toUtf8().size()<=4096,"Invalid piezo readout port identity.");
    for (int i=0;i<v.size();++i) {
        const auto c=v[i];
        require(c.unicode()>=0x20 && c.unicode()!=0x7f && c!='"' && c!='\\',"Invalid piezo readout identity character.");
        if(c.isHighSurrogate()) { require(i+1<v.size() && v[i+1].isLowSurrogate(),"Invalid piezo readout UTF-16."); ++i; }
        else require(!c.isLowSurrogate(),"Invalid piezo readout UTF-16.");
    }
}
struct Writer {
    QByteArray bytes;
    void integer(quint64 v) { for(uint i=0;i<8;++i) bytes.append(char((v>>(8*i))&255)); }
    void real(double v) {
        static_assert(sizeof(double)==8 && std::numeric_limits<double>::is_iec559,"IEEE binary64 required");
        quint64 bits; std::memcpy(&bits,&v,8); integer(bits);
    }
    void text(const QString &v) { const auto b=v.toUtf8(); integer(quint64(b.size())); bytes.append(b); }
};
}

RStructuralPiezoReadoutQuantity rStructuralPiezoReadoutQuantity(RStructuralPiezoReadoutMode mode)
{
    switch(mode) {
    case RStructuralPiezoReadoutMode::None:return RStructuralPiezoReadoutQuantity::None;
    case RStructuralPiezoReadoutMode::OpenCircuitVoltage:
    case RStructuralPiezoReadoutMode::IdealChargeAmplifier:return RStructuralPiezoReadoutQuantity::Voltage;
    case RStructuralPiezoReadoutMode::ShortCircuitCharge:
    case RStructuralPiezoReadoutMode::ActuatorCharge:return RStructuralPiezoReadoutQuantity::Charge;
    }
    require(false,"Invalid piezo readout mode."); return RStructuralPiezoReadoutQuantity::None;
}

QString rStructuralPiezoReadoutUnit(RStructuralPiezoReadoutMode mode)
{
    switch(rStructuralPiezoReadoutQuantity(mode)) {
    case RStructuralPiezoReadoutQuantity::None:return {};
    case RStructuralPiezoReadoutQuantity::Voltage:return QStringLiteral("V");
    case RStructuralPiezoReadoutQuantity::Charge:return QStringLiteral("C");
    }
    return {};
}

bool RStructuralPiezoReadoutSetup::hasNonDefaultData() const
{
    return !ports.empty() || workpoint.available || !workpoint.nodes.empty() || !workpoint.ports.empty();
}

void RStructuralPiezoReadoutSetup::validate() const
{
    bounded(ports.size()); bounded(workpoint.nodes.size()); bounded(workpoint.ports.size());
    std::set<QString> ids;
    for(const auto &p:ports) {
        id(p.portId); require(ids.insert(p.portId).second,"Duplicate piezo readout port identity.");
        require(p.role==RStructuralPiezoPortRole::Actuator || p.role==RStructuralPiezoPortRole::Sensor ||
                p.role==RStructuralPiezoPortRole::ActuatorSensor,"Invalid piezo port role.");
        require(p.readout==RStructuralPiezoReadoutMode::None || p.readout==RStructuralPiezoReadoutMode::OpenCircuitVoltage ||
                p.readout==RStructuralPiezoReadoutMode::ShortCircuitCharge || p.readout==RStructuralPiezoReadoutMode::IdealChargeAmplifier ||
                p.readout==RStructuralPiezoReadoutMode::ActuatorCharge,"Invalid piezo readout mode.");
        const bool sensor=p.role==RStructuralPiezoPortRole::Sensor;
        const bool sensorMode=p.readout==RStructuralPiezoReadoutMode::OpenCircuitVoltage ||
                              p.readout==RStructuralPiezoReadoutMode::ShortCircuitCharge ||
                              p.readout==RStructuralPiezoReadoutMode::IdealChargeAmplifier;
        require(sensor==sensorMode,"Piezo port role conflicts with its electrical readout.");
        if(p.role==RStructuralPiezoPortRole::ActuatorSensor)
            require(p.readout==RStructuralPiezoReadoutMode::ActuatorCharge,"Combined actuator/sensor requires actuator charge readout.");
        finite(p.Cf); finite(p.Qref); finite(p.VoutBias);
        require(p.amplifierPolarity==1 || p.amplifierPolarity==-1,"Ideal charge amplifier polarity must be +1 or -1.");
        if(p.readout==RStructuralPiezoReadoutMode::IdealChargeAmplifier)
            require(p.Cf>0.0,"Ideal charge amplifier feedback capacitance must be positive.");
        else require(p.Cf==0.0 && p.amplifierPolarity==-1 && p.Qref==0.0 && p.VoutBias==0.0,
                     "Non-amplifier readout contains irrelevant amplifier parameters.");
    }
    require(workpoint.available || (workpoint.nodes.empty() && workpoint.ports.empty()),"Unavailable piezo workpoint contains hidden source data.");
    if(workpoint.available) require(!workpoint.nodes.empty(),"Explicit piezo workpoint requires physical nodes.");
    std::set<uint> nodes;
    for(const auto &n:workpoint.nodes) {
        require(n.physicalNodeId!=0 && n.physicalNodeId!=RConstants::eod && nodes.insert(n.physicalNodeId).second,
                "Invalid or duplicate piezo workpoint physical node identity.");
        for(double v:n.q0) finite(v);
        for(double v:n.f0) finite(v);
    }
    ids.clear();
    for(const auto &p:workpoint.ports) {
        id(p.portId); require(ids.insert(p.portId).second,"Duplicate piezo workpoint port identity.");
        finite(p.V0); finite(p.Q0);
    }
}

void RStructuralPiezoReadoutSetup::validate(const RModel &model) const
{
    validate();
    if(!hasNonDefaultData()) return;
    const auto &truss=model.getProblemSetup().getStructuralTrussDynamicsSetup();
    require(truss.enabled,"Piezo readout source requires enabled native truss dynamics.");
    bounded(truss.activeMembers.size());
    std::map<QString,const RStructuralTrussActiveMember*> active;
    for(const auto &m:truss.activeMembers) {
        id(m.portId); require(active.emplace(m.portId,&m).second,"Duplicate active piezo port identity.");
        require(m.kind==RStructuralTrussElectricalKind::Voltage || m.kind==RStructuralTrussElectricalKind::ConductorCharge,
                "Invalid active piezo electrical kind.");
        finite(m.initialValue);
    }
    if(!ports.empty()) require(ports.size()==active.size(),"Piezo readout catalog must classify every active electrical port.");
    const auto &control=model.getProblemSetup().getStructuralNativeControlSpec();
    bounded(truss.heldCommands.size()); bounded(control.actions.size());
    std::set<QString> commandedPorts;
    size_t commandCount=0;
    for(const auto &command:truss.heldCommands) {
        commandCount+=command.electrical.size(); bounded(commandCount);
        for(const auto &e:command.electrical) commandedPorts.insert(e.portId);
    }
    for(const auto &action:control.actions) if(!action.portId.isEmpty()) commandedPorts.insert(action.portId);
    for(const auto &p:ports) {
        const auto found=active.find(p.portId);
        require(found!=active.end(),"Piezo readout references an unknown active electrical port.");
        const auto &m=*found->second;
        if(p.role!=RStructuralPiezoPortRole::Sensor)
            require(m.kind==RStructuralTrussElectricalKind::Voltage,"Piezo actuator requires a voltage-driven port.");
        else {
            if(p.readout==RStructuralPiezoReadoutMode::OpenCircuitVoltage)
                require(m.kind==RStructuralTrussElectricalKind::ConductorCharge,"Open-circuit sensor requires fixed total conductor charge.");
            else require(m.kind==RStructuralTrussElectricalKind::Voltage && m.initialValue==0.0,
                         "Short-circuit/ideal-amplifier sensor requires fixed zero voltage.");
            require(commandedPorts.count(p.portId)==0,"Sensor-only piezo port cannot receive held electrical commands or native control actions.");
        }
    }
    if(workpoint.available) {
        bounded(model.getNNodes());
        std::set<uint> nodes;
        for(uint i=0;i<model.getNNodes();++i) {
            const uint physical=model.getNodePhysicalId(i);
            require(physical!=0 && physical!=RConstants::eod && nodes.insert(physical).second,
                    "Invalid model physical node identity for piezo workpoint.");
        }
        require(workpoint.nodes.size()==nodes.size() && workpoint.ports.size()==active.size(),
                "Piezo workpoint must cover all physical nodes and active electrical ports.");
        for(const auto &n:workpoint.nodes) require(nodes.count(n.physicalNodeId)==1,"Piezo workpoint references an unknown physical node.");
        for(const auto &p:workpoint.ports) require(active.count(p.portId)==1,"Piezo workpoint references an unknown active port.");
    }
}

QByteArray RStructuralPiezoReadoutSetup::identityBytes() const
{
    validate();
    if(!hasNonDefaultData()) return {};
    Writer w; w.text("range-fea/piezo-readout/source/v1/binary64");
    w.integer(ports.size());
    for(const auto &p:ports) {
        w.text(p.portId); w.integer(uint(p.role)); w.integer(uint(p.readout));
        w.real(p.Cf); w.integer(quint64(qint64(p.amplifierPolarity))); w.real(p.Qref); w.real(p.VoutBias);
    }
    w.integer(workpoint.available?1:0); w.integer(workpoint.nodes.size());
    for(const auto &n:workpoint.nodes) { w.integer(n.physicalNodeId); for(double v:n.q0) w.real(v); for(double v:n.f0) w.real(v); }
    w.integer(workpoint.ports.size());
    for(const auto &p:workpoint.ports) { w.text(p.portId); w.real(p.V0); w.real(p.Q0); }
    return w.bytes;
}
