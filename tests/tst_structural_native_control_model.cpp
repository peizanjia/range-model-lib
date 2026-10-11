#include <QtTest>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QFile>
#include <cmath>
#include <cstring>
#include <set>
#include <limits>
#include <utility>
#include "rml_model.h"
#include "rml_file_io.h"

namespace {
void require(bool ok,const char *message) { if(!ok) throw RError(RError::Type::InvalidInput,R_ERROR_REF,message); }
void count(size_t n) { require(n<=1000000,"count"); }
void validateText(const QString &s,const char * = "text") { require(s.toUtf8().size()<=4096,"text"); }
void validateId(const QString &s) { require(!s.isEmpty(),"id"); }
void finite(double v) { require(std::isfinite(v),"finite"); }
void positive(double v) { require(std::isfinite(v)&&v>0,"positive"); }
void physicalId(uint v) { require(v!=0&&v!=RConstants::eod,"id"); }
class LegacySourceWriter
{
    QCryptographicHash hash{QCryptographicHash::Sha256};
public:
    void integer(quint64 value)
    {
        char bytes[8];
        for (uint i = 0; i < 8; ++i) bytes[i] = char((value >> (8*i)) & 0xffu);
        hash.addData(QByteArray(bytes,8));
    }
    void real(double value, bool allowInfinity = false)
    {
        require(allowInfinity ? !std::isnan(value) : std::isfinite(value),"Non-finite native truss fingerprint input.");
        static_assert(sizeof(double) == sizeof(quint64) && std::numeric_limits<double>::is_iec559,
                      "Native truss source identity requires IEEE binary64.");
        quint64 bits = 0;
        std::memcpy(&bits,&value,sizeof(bits));
        integer(bits);
    }
    void text(const QString &value, const char *field = "Native truss source text")
    {
        validateText(value,field);
        const QByteArray bytes = value.toUtf8();
        integer(quint64(bytes.size()));
        hash.addData(bytes);
    }
    void entries(size_t size) { count(size); integer(quint64(size)); }
    void vector(const RR3Vector &value)
    {
        require(value.size() == 3,"Invalid native truss source direction dimensions.");
        for (uint i = 0; i < 3; ++i) real(value[i]);
    }
    QString digest() { return QString::fromLatin1(hash.result().toHex()); }
};

void writeTable(LegacySourceWriter &writer, const RValueTable &table)
{
    writer.text(table.getKeyName(),"Native truss table key name"); writer.text(table.getKeyUnits(),"Native truss table key units");
    writer.text(table.getValueName(),"Native truss table value name"); writer.text(table.getValueUnits(),"Native truss table value units");
    writer.entries(table.size());
    for (uint i = 0; i < table.size(); ++i) { writer.real(table.getKey(i)); writer.real(table.getValue(i)); }
}

void writeCondition(LegacySourceWriter &writer, const RCondition &condition)
{
    writer.text(condition.getName(),"Native truss condition name");
    writer.integer(condition.getApplyOnPoint()); writer.integer(condition.getApplyOnLine());
    writer.integer(condition.getApplyOnSurface()); writer.integer(condition.getApplyOnVolume());
    writer.integer(condition.getProblemTypeMask());
    writer.entries(condition.size());
    for (uint i = 0; i < condition.size(); ++i)
    {
        const auto &component = condition.getComponent(i);
        writer.integer(component.getType()); writer.integer(component.getKeyType()); writer.integer(component.getEnabled());
        writeTable(writer,component);
    }
}

void writeGroup(LegacySourceWriter &writer, const RElementGroup &group, const RModel &model)
{
    physicalId(group.getPhysicalId());
    writer.integer(group.getPhysicalId()); writer.text(group.getName(),"Native truss element group name");
    writer.entries(group.size());
    for (uint i = 0; i < group.size(); ++i)
    {
        require(group.get(i) < model.getNElements(),"Native truss group references an unknown element position.");
        writer.integer(group.get(i)); writer.integer(model.getElementPhysicalId(group.get(i)));
    }
    const auto &material = group.getMaterial();
    writer.text(material.getID().toString(QUuid::WithoutBraces));
    writer.text(material.getName(),"Native truss material name"); writer.integer(material.getState());
    writer.entries(material.size());
    for (uint i = 0; i < material.size(); ++i)
    {
        const auto &property = material.get(i);
        writer.integer(property.getType()); writer.integer(property.getKeyType()); writeTable(writer,property);
    }
    writer.entries(group.getNBoundaryConditions());
    for (uint i = 0; i < group.getNBoundaryConditions(); ++i)
    {
        const auto &condition = group.getBoundaryCondition(i);
        writer.integer(condition.getType()); writer.integer(condition.getOptional()); writer.integer(condition.getExplicit());
        writer.integer(condition.getHasLocalDirection()); writer.integer(condition.getExplicitLocalDirection());
        writer.vector(condition.getLocalDirection()); writeCondition(writer,condition);
    }
    writer.entries(group.getNInitialConditions());
    for (uint i = 0; i < group.getNInitialConditions(); ++i)
    {
        const auto &condition = group.getInitialCondition(i);
        writer.integer(condition.getType()); writeCondition(writer,condition);
    }
    writer.entries(group.getNEnvironmentConditions());
    for (uint i = 0; i < group.getNEnvironmentConditions(); ++i)
    {
        const auto &condition = group.getEnvironmentCondition(i);
        writer.integer(condition.getType()); writeCondition(writer,condition);
    }
}

void writeMixedSetup(LegacySourceWriter &writer, const RStructuralTrussDynamicsSetup &setup)
{
    writer.integer(setup.enabled);
    writer.integer(setup.maxIterations); writer.integer(setup.maxLineSearchIterations);
    writer.real(setup.absoluteForceTolerance); writer.real(setup.relativeForceTolerance); writer.real(setup.linearRelativeTolerance);
    writer.entries(setup.passiveMembers.size());
    for (const auto &member : setup.passiveMembers)
    { writer.integer(member.physicalElementId); writer.real(member.referenceLength); }
    writer.entries(setup.activeMembers.size());
    for (const auto &member : setup.activeMembers)
    {
        writer.integer(member.physicalElementId); writer.text(member.portId); writer.real(member.referenceLength);
        writer.real(member.axialStiffness); writer.real(member.coupling); writer.real(member.capacitance);
        writer.integer(uint(member.kind)); writer.real(member.initialValue);
        writer.integer(uint(member.massSource)); writer.real(member.fixedMass);
    }
    writer.entries(setup.cableMembers.size());
    for (const auto &member : setup.cableMembers)
    {
        writer.integer(member.physicalElementId); writer.real(member.initialReferenceLength); writer.real(member.initialTension);
        writer.integer(uint(member.massSource)); writer.real(member.fixedMass);
    }
    writer.entries(setup.heldCommands.size());
    for (const auto &command : setup.heldCommands)
    {
        writer.integer(command.stepIndex); writer.entries(command.electrical.size());
        for (const auto &input : command.electrical) { writer.text(input.portId); writer.real(input.value); }
        writer.entries(command.cables.size());
        for (const auto &input : command.cables) { writer.integer(input.physicalElementId); writer.real(input.referenceLength); }
        writer.entries(command.nodalForces.size());
        for (const auto &input : command.nodalForces)
        { writer.integer(input.physicalNodeId); for (double value : input.force) writer.real(value); }
    }
}

void writeDof(LegacySourceWriter &writer, const RStructuralDofRef &dof)
{
    writer.integer(dof.nodeId); writer.integer(uint(dof.dof));
}

void writeTarget(LegacySourceWriter &writer, const RStructuralTarget &target)
{
    writer.integer(uint(target.scope)); writer.integer(target.nodeId);
    writer.text(target.setName); writer.integer(target.entityType);
}

void writeInteraction(LegacySourceWriter &writer, const RStructuralInteractionSetup &interaction)
{
    writer.entries(interaction.getLinearConstraints().size());
    for (const auto &constraint : interaction.getLinearConstraints())
    {
        validateId(constraint.id);
        writer.text(constraint.id); writer.text(constraint.name); writer.real(constraint.rhs);
        writer.entries(constraint.terms.size());
        for (const auto &term : constraint.terms) { writeDof(writer,term.reference); writer.real(term.coefficient); }
    }
    writer.entries(interaction.getCouplings().size());
    for (const auto &coupling : interaction.getCouplings())
    {
        validateId(coupling.id);
        writer.text(coupling.id); writer.text(coupling.name); writer.integer(uint(coupling.type));
        writer.integer(coupling.referenceNode); writer.text(coupling.masterSet); writer.text(coupling.slaveSet);
        writer.integer(coupling.entityType); writer.real(coupling.tolerance);
        writer.entries(coupling.dofs.size());
        for (const auto dof : coupling.dofs) writer.integer(uint(dof));
    }
    writer.entries(interaction.getPrescribedMotions().size());
    for (const auto &motion : interaction.getPrescribedMotions())
    {
        validateId(motion.id);
        writer.text(motion.id); writeDof(writer,motion.target); writer.entries(motion.knots.size());
        for (const auto &knot : motion.knots)
        { writer.real(knot.time); writer.real(knot.displacement); writer.real(knot.velocity); writer.real(knot.acceleration); }
    }
    writer.entries(interaction.getSensors().size());
    for (const auto &sensor : interaction.getSensors())
    {
        validateId(sensor.id);
        writer.text(sensor.id); writer.text(sensor.name); writeTarget(writer,sensor.target);
        writer.integer(uint(sensor.quantity)); writer.integer(uint(sensor.aggregation)); writer.integer(uint(sensor.frame));
        writer.vector(sensor.direction); writer.text(sensor.unit);
    }
    writer.entries(interaction.getActuators().size());
    for (const auto &actuator : interaction.getActuators())
    {
        validateId(actuator.id);
        writer.text(actuator.id); writer.text(actuator.name); writer.integer(uint(actuator.type)); writeTarget(writer,actuator.target);
        writer.integer(actuator.variable); writer.integer(uint(actuator.frame)); writer.vector(actuator.direction); writer.text(actuator.unit);
        // The existing actuator API deliberately uses infinite limits by default.
        writer.real(actuator.minimum,true); writer.real(actuator.maximum,true); writer.real(actuator.rateLimit,true);
    }
}

void writeTask(LegacySourceWriter &writer, const RProblemTaskItem &task, uint depth, size_t &total)
{
    require(depth <= 256,"Native truss problem tree exceeds its depth bound.");
    count(++total);
    writer.integer(task.getProblemType()); writer.integer(task.getNIterations()); writer.entries(task.getNChildren());
    for (uint i = 0; i < task.getNChildren(); ++i) writeTask(writer,task.getChild(i),depth+1,total);
}

void writeOtherStructuralSetup(LegacySourceWriter &writer, const RProblemSetup &problem)
{
    writer.integer(uint(problem.getMassMatrixType()));
    const auto &dynamics = problem.getStructuralDynamicsSetup();
    writer.integer(dynamics.getEnabled()); writer.integer(uint(dynamics.getMethod()));
    writer.real(dynamics.getTimeStep()); writer.integer(dynamics.getNSteps()); writer.integer(dynamics.getOutputFrequency());
    writer.real(dynamics.getRayleighAlpha()); writer.real(dynamics.getRayleighBeta()); writer.real(dynamics.getRhoInfinity());
    writer.real(dynamics.getNewmarkGamma()); writer.real(dynamics.getNewmarkBeta());
    writer.entries(dynamics.getModalDampingRatios().size());
    for (double ratio : dynamics.getModalDampingRatios()) writer.real(ratio);
    const auto &control = problem.getStructuralControlSetup();
    writer.integer(control.enabled); writer.integer(control.controlProtocolVersion); writer.text(control.controllerId);
    if (!control.controllerId.isEmpty()) validateId(control.controllerId);
    writer.integer(control.deterministicSeed); writer.entries(control.channelConditioning.size());
    for (const auto &channel : control.channelConditioning)
    {
        validateId(channel.actuatorId);
        writer.text(channel.actuatorId); writer.integer(channel.delaySteps); writer.integer(channel.holdSteps); writer.real(channel.noiseAmplitude);
    }
    const auto &cable = problem.getStructuralCableSetup();
    writer.integer(cable.enabled); writer.text(cable.meshFingerprint);
    writer.integer(cable.maxIterations); writer.integer(cable.maxLineSearchIterations);
    writer.real(cable.absoluteForceTolerance); writer.real(cable.relativeForceTolerance); writer.entries(cable.members.size());
    for (const auto &member : cable.members)
    { writer.integer(member.elementId); writer.real(member.referenceLength); writer.real(member.initialTension); }
    const auto &piezo = problem.getStructuralPiezoSetup();
    writer.integer(piezo.enabled); writer.text(piezo.meshFingerprint); writer.integer(uint(piezo.electricalMode));
    writer.entries(piezo.ports.size());
    for (const auto &port : piezo.ports)
    {
        validateId(port.id);
        writer.text(port.id); writer.integer(port.elementId); writer.real(port.coupling); writer.real(port.permittivity); writer.real(port.voltage);
    }
    writer.entries(piezo.passiveElementIds.size());
    for (uint id : piezo.passiveElementIds) writer.integer(id);
    writer.entries(piezo.nodalBoundaryConditions.size());
    for (const auto &boundary : piezo.nodalBoundaryConditions)
    { writer.integer(boundary.nodeId); writer.integer(uint(boundary.type)); writer.real(boundary.value); }
    const auto &thermal = problem.getStructuralThermalControlSetup();
    writer.integer(thermal.enabled); writer.real(thermal.thermalSubstepSeconds);
    writer.integer(thermal.thermalSubstepsPerMechanicalMacrostep); writer.integer(thermal.mechanicalMacrostepsPerControllerSample);
    writer.entries(thermal.channels.size());
    for (const auto &channel : thermal.channels)
    {
        validateId(channel.actuatorId); validateId(channel.heaterNodeId);
        validateId(channel.temperatureSensorId); validateId(channel.temperatureSensorNodeId);
        writer.text(channel.actuatorId); writer.text(channel.heaterNodeId);
        writer.text(channel.temperatureSensorId); writer.text(channel.temperatureSensorNodeId);
        writer.real(channel.turnOnBelowKelvin); writer.real(channel.turnOffAtOrAboveKelvin);
        writer.real(channel.onPowerWatts); writer.real(channel.minimumPowerWatts); writer.real(channel.maximumPowerWatts);
        writer.real(channel.maximumPowerSlewWattsPerSecond); writer.integer(channel.maximumPowerSlewUnlimited);
    }
    writer.entries(thermal.sparseObservationNodeIds.size());
    for (const auto &id : thermal.sparseObservationNodeIds) { validateId(id); writer.text(id); }
    const auto &parallel = problem.getParallelExecutionConfig();
    writer.integer(uint(parallel.getMode())); writer.integer(parallel.getRequestedThreads()); writer.integer(parallel.getDeterministicReduction());
    writeInteraction(writer,problem.getStructuralInteractionSetup());
}
QString legacySource(const RModel &model,const RStructuralTrussDynamicsSetup &setup) {
    LegacySourceWriter writer;
    writer.text(QStringLiteral("range-fea/native-mixed-truss/source/v1/binary64"));
    writer.real(293.15);
    writer.entries(model.getNNodes());
    for (uint i = 0; i < model.getNNodes(); ++i)
    {
        writer.integer(model.getNodePhysicalId(i));
        const auto &node = model.getNode(i);
        writer.real(node.getX()); writer.real(node.getY()); writer.real(node.getZ());
    }
    writer.entries(model.getNElements());
    for (uint i = 0; i < model.getNElements(); ++i)
    {
        const auto &element = model.getElement(i);
        writer.integer(model.getElementPhysicalId(i)); writer.integer(element.getType()); writer.entries(element.size());
        for (uint j = 0; j < element.size(); ++j)
        { writer.integer(element.getNodeId(j)); writer.integer(model.getNodePhysicalId(element.getNodeId(j))); }
    }
    std::set<uint> groupIds;
    const auto group = [&writer,&model,&groupIds](const RElementGroup &value)
    {
        require(groupIds.insert(value.getPhysicalId()).second,"Duplicate native truss model physical group ID.");
        writeGroup(writer,value,model);
    };
    writer.entries(model.getNPoints());
    for (uint i = 0; i < model.getNPoints(); ++i)
    { const auto &point = model.getPoint(i); group(point); writer.real(point.getVolume()); }
    writer.entries(model.getNLines());
    for (uint i = 0; i < model.getNLines(); ++i)
    {
        const auto &line = model.getLine(i); group(line);
        writer.real(line.getCrossArea()); writer.real(line.getSecondMomentY()); writer.real(line.getSecondMomentZ());
        writer.real(line.getTorsionalConstant()); writer.real(line.getSectionModulusY()); writer.real(line.getSectionModulusZ());
        writer.real(line.getTorsionalSectionModulus()); writer.vector(line.getLocalYDirection());
    }
    writer.entries(model.getNSurfaces());
    for (uint i = 0; i < model.getNSurfaces(); ++i)
    { const auto &surface = model.getSurface(i); group(surface); writer.real(surface.getThickness()); writer.integer(uint(surface.getStructuralFormulation())); }
    writer.entries(model.getNVolumes());
    for (uint i = 0; i < model.getNVolumes(); ++i) group(model.getVolume(i));
    writeMixedSetup(writer,setup);
    writeOtherStructuralSetup(writer,model.getProblemSetup());
    writer.real(model.getTimeSolver().getInputStartTime());
    size_t taskCount = 0;
    writeTask(writer,model.getProblemTaskTree(),0,taskCount);
    return writer.digest();
}
}

namespace {
RModel scene()
{
    RModel m; const uint ids[]={93,17,51,44,22};
    for(uint i=0;i<5;++i) { m.addNode(RNode(double(i),0,0)); m.setNodePhysicalId(i,ids[i]); }
    const uint elements[]={701,211,503,307};
    for(uint i=0;i<4;++i) { RElement e(R_ELEMENT_TRUSS1); e.setNodeId(0,i); e.setNodeId(1,i+1); m.addElement(e,true,0); m.setElementPhysicalId(i,elements[i]); }
    RMaterial material(RMaterial::Solid);
    for(auto entry:{std::pair{RMaterialProperty::Density,100.0},std::pair{RMaterialProperty::ModulusOfElasiticity,1000.0}}) {
        RMaterialProperty p(entry.first); p.add(293.15,entry.second); material.add(p);
    }
    m.getLine(0).setMaterial(material); m.getLine(0).setCrossArea(0.1);
    auto &d=m.getProblemSetup().getStructuralDynamicsSetup(); d.setEnabled(true); d.setTimeStep(0.125); d.setNSteps(10);
    m.getTimeSolver().setInputStartTime(0.125);
    auto &t=m.getProblemSetup().getStructuralTrussDynamicsSetup(); t.enabled=true; t.passiveMembers={{701,1.0}};
    RStructuralTrussActiveMember v; v.physicalElementId=211; v.portId="V port"; v.referenceLength=1; v.axialStiffness=100; v.coupling=0.2; v.capacitance=0.1; v.initialValue=-1.2345678901234567;
    auto q=v; q.physicalElementId=503; q.portId="Q port"; q.kind=RStructuralTrussElectricalKind::ConductorCharge; q.initialValue=2.3456789012345678e-6;
    t.activeMembers={v,q}; t.cableMembers={{307,0.9876543210987654,1.0}};
    return m;
}
RStructuralNativeControlSpec spec()
{
    RStructuralNativeControlSpec s; s.source=RStructuralNativeControlSource::LocalV3; s.providerId="builtin-test-pi"; s.providerRevision="v1";
    s.implementationSha256=QString(64,'a'); s.dependencyEnvironmentManifest=QByteArray("environment\0exact\n",18);
    s.dependencyEnvironmentSha256=QString::fromLatin1(QCryptographicHash::hash(s.dependencyEnvironmentManifest,QCryptographicHash::Sha256).toHex());
    for(uint i=0;i<7;++i) {
        RStructuralNativeControlSensor v; v.sensorId="sensor "+QString::number(i); v.quantity=static_cast<RStructuralNativeMeasurementQuantity>(i);
        if(i==3) v.physicalMemberId=701; else if(i==4||i==5) v.portId=i==4?"V port":"Q port"; else { v.physicalNodeId=17; v.axis=2; }
        s.sensors.push_back(v);
    }
    RStructuralNativeControlAction force; force.actionId="force"; force.physicalNodeId=22; force.axis=1; force.minimum=-2; force.maximum=2; force.rateLimit=0.25;
    RStructuralNativeControlAction cable; cable.actionId="L0"; cable.quantity=RStructuralNativeActionQuantity::CableReferenceLength; cable.physicalMemberId=307; cable.initialCommand=0.9876543210987654;
    RStructuralNativeControlAction v; v.actionId="voltage"; v.quantity=RStructuralNativeActionQuantity::Voltage; v.portId="V port"; v.initialCommand=-1.2345678901234567;
    RStructuralNativeControlAction q; q.actionId="charge"; q.quantity=RStructuralNativeActionQuantity::Charge; q.portId="Q port"; q.initialCommand=2.3456789012345678e-6;
    s.actions={force,cable,v,q};
    for(size_t i=0;i<s.actions.size();++i) { RStructuralNativeControlPILoop p; p.actionId=s.actions[i].actionId; p.sensorId=s.sensors[i].sensorId; p.referenceValue=1.2345678901234567; p.proportionalGain=-2.3456789012345678; p.integralGain=3.4567890123456789; p.bias=-0.0; p.initialIntegral=-4.567890123456789; s.piLoops.push_back(p); }
    return s;
}
RStructuralNativeControlState state(const RStructuralNativeControlSpec &s,const QString &fp,bool initial=false)
{
    RStructuralNativeControlState r; r.available=true; r.sourceFingerprint=fp; r.providerRevision=s.providerRevision; r.controllerState=QByteArray("after\0I",7);
    for(const auto &a:s.actions) { r.heldValues.push_back(a.initialCommand); r.lastAppliedValues.push_back(a.initialCommand); }
    if(initial) return r;
    r.completedFE=2; r.completedControlTicks=2; r.lastControllerInputState=QByteArray("before\0I",8);
    auto &f=r.lastFrame; f.completedFE=1; f.controlTick=1; f.sampleTime=0.125; f.nextFETime=0.25; f.nextControlTime=0.25; f.physicalDt=0.125;
    for(size_t i=0;i<s.sensors.size();++i) f.samples.push_back({s.sensors[i].sensorId,s.sensors[i].quantity,i==0?-0.0:1.2345678901234567});
    for(const auto &a:s.actions) r.lastActions.push_back({a.actionId,a.quantity,a.initialCommand,a.initialCommand,a.initialCommand,a.initialCommand,0});
    auto &a=r.lastActions[0]; a.requested=3; a.conditioned=3; a.applied=0.03125; a.limitFlags=3; r.heldValues[0]=a.applied; r.lastAppliedValues[0]=a.applied;
    return r;
}
RModel localScene(bool initial=false)
{
    auto m=scene(); auto s=spec(); m.getProblemSetup().setStructuralNativeControlSpec(s);
    auto &t=m.getProblemSetup().getStructuralTrussDynamicsSetup(); t.bindToModel(m);
    m.getProblemSetup().setStructuralNativeControlState(state(s,t.sourceFingerprint,initial)); return m;
}
QByteArray readAll(const QString &path) { QFile f(path); if(!f.open(QIODevice::ReadOnly)) return {}; return f.readAll(); }
// The format-20 outer setup field list is frozen here independently of the
// new outer writer. Each old typed record retains its existing encoding.
void writeLegacySetup(RSaveFile &f,const RProblemSetup &p,bool binary)
{
    auto io=[&](const auto &v) { if(binary) RFileIO::writeBinary(f,v); else RFileIO::writeAscii(f,v); };
    io(p.getRestart()); io(p.getRadiationSetup()); io(p.getModalSetup()); io(p.getMeshSetup()); io(p.getAcousticSetup()); io(int(p.getMassMatrixType()));
    io(p.getStructuralInteractionSetup()); io(p.getStructuralDynamicsSetup()); io(p.getParallelExecutionConfig()); io(p.getStructuralDynamicsRestartData());
    io(p.getStructuralControlSetup()); io(p.getStructuralCableSetup()); io(p.getStructuralPiezoSetup()); io(p.getStructuralThermalControlSetup());
    io(p.getStructuralTrussDynamicsSetup()); io(p.getStructuralTrussDynamicsRestartData());
}
}
class TestStructuralNativeControlModel:public QObject {
    Q_OBJECT
private slots:
    void persistence_data() { QTest::addColumn<bool>("binary"); QTest::newRow("RAM")<<false; QTest::newRow("RBM")<<true; }
    void persistence();
    void legacy_data() { persistence_data(); } void legacy();
    void invalidSpec_data(); void invalidSpec();
    void invalidState_data(); void invalidState();
    void sourceIdentity();
    void malformed_data() { persistence_data(); } void malformed();
    void hiddenDowngrade_data() { persistence_data(); } void hiddenDowngrade();
    void rawMalformed_data(); void rawMalformed();
    void fingerprintCompleteness();
};
void TestStructuralNativeControlModel::persistence()
{
    QFETCH(bool,binary); QTemporaryDir dir; auto m=localScene(); const auto &p=m.getProblemSetup();
    const auto path=dir.filePath("native."+RModel::getDefaultFileExtension(binary)); m.write(path,false);
    RModel restored; restored.read(path);
    QCOMPARE(restored.getProblemSetup().getStructuralNativeControlSpec().identityBytes(),p.getStructuralNativeControlSpec().identityBytes());
    QCOMPARE(restored.getProblemSetup().getStructuralNativeControlState().fingerprintBytes(),p.getStructuralNativeControlState().fingerprintBytes());
    restored.getProblemSetup().getStructuralNativeControlSpec().validate(restored);
    restored.getProblemSetup().getStructuralTrussDynamicsSetup().validate(restored);
    for(bool initial:{false,true}) {
        const auto r=state(p.getStructuralNativeControlSpec(),p.getStructuralTrussDynamicsSetup().sourceFingerprint,initial);
        const auto typed=dir.filePath("typed");
        { RSaveFile out(typed,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,21)); QVERIFY(out.open(QIODevice::WriteOnly));
          if(binary) { RFileIO::writeBinary(out,p.getStructuralNativeControlSpec()); RFileIO::writeBinary(out,r); }
          else { RFileIO::writeAscii(out,p.getStructuralNativeControlSpec(),false); RFileIO::writeAscii(out,r,false); } QVERIFY(out.commit()); }
        RFile in(typed,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,21)); QVERIFY(in.open(QIODevice::ReadOnly));
        RStructuralNativeControlSpec ss; RStructuralNativeControlState rr;
        if(binary) { RFileIO::readBinary(in,ss); RFileIO::readBinary(in,rr); } else { RFileIO::readAscii(in,ss); RFileIO::readAscii(in,rr); }
        QCOMPARE(ss.identityBytes(),p.getStructuralNativeControlSpec().identityBytes()); QCOMPARE(rr.fingerprintBytes(),r.fingerprintBytes()); rr.validate(ss);
    }
    RProblemSetup copy=p; QCOMPARE(copy.getStructuralNativeControlState().fingerprintBytes(),p.getStructuralNativeControlState().fingerprintBytes());
    RProblemSetup assigned; assigned=p; QCOMPARE(assigned.getStructuralNativeControlSpec().identityBytes(),p.getStructuralNativeControlSpec().identityBytes());
}
void TestStructuralNativeControlModel::legacy()
{
    QFETCH(bool,binary); QTemporaryDir dir; auto m=scene(); auto &t=m.getProblemSetup().getStructuralTrussDynamicsSetup(); t.bindToModel(m);
    QCOMPARE(t.sourceFingerprint,legacySource(m,t));
    const auto oldPath=dir.filePath("legacy"); const auto goldenPath=dir.filePath("golden");
    for(int pass=0;pass<2;++pass) { RSaveFile out(pass?goldenPath:oldPath,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,20)); QVERIFY(out.open(QIODevice::WriteOnly));
        if(pass) writeLegacySetup(out,m.getProblemSetup(),binary);
        else if(binary) RFileIO::writeBinary(out,m.getProblemSetup()); else RFileIO::writeAscii(out,m.getProblemSetup()); QVERIFY(out.commit()); }
    QCOMPARE(readAll(oldPath),readAll(goldenPath));
    auto dirty=localScene().getProblemSetup();
    { RFile in(oldPath,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,20)); QVERIFY(in.open(QIODevice::ReadOnly));
      if(binary) RFileIO::readBinary(in,dirty); else RFileIO::readAscii(in,dirty); }
    QVERIFY(!dirty.getStructuralNativeControlSpec().hasNonDefaultData()); QVERIFY(!dirty.getStructuralNativeControlState().hasNonDefaultData());
    RStructuralNativeControlSpec ss=spec(); RStructuralNativeControlState rr=state(ss,QString(64,'a'));
    RFile unopened("unused",binary?RFile::BINARY:RFile::ASCII); unopened.setVersion(RVersion(1,3,20));
    if(binary) { RFileIO::readBinary(unopened,ss); RFileIO::readBinary(unopened,rr); } else { RFileIO::readAscii(unopened,ss); RFileIO::readAscii(unopened,rr); }
    QVERIFY(!ss.hasNonDefaultData()); QVERIFY(!rr.hasNonDefaultData());
}
void TestStructuralNativeControlModel::invalidSpec_data()
{
    QTest::addColumn<int>("mutation"); const char *names[]={"schema","source","provider-empty","bad-code-digest","manifest-hash","multirate","sample-phase","seed","sensor-duplicate","sensor-enum","irrelevant-target","node-missing","member-missing","port-missing","action-alias","action-kind","delay","hold","noise","nan-gain","loop-duplicate","loop-missing","wrong-VQ-owner","wrong-L0-owner","initial-V","initial-force","invalid-limit","nan-rate","unsafe-id","oversized-catalog","frozen-hidden"};
    for(int i=0;i<31;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralNativeControlModel::invalidSpec()
{
    QFETCH(int,mutation); auto m=scene(); auto s=spec(); s.validate(m);
    switch(mutation) {
    case 0:s.schemaVersion=2;break;case 1:s.source=static_cast<RStructuralNativeControlSource>(2);break;case 2:s.providerId.clear();break;
    case 3:s.implementationSha256="abc";break;case 4:s.dependencyEnvironmentManifest.append('x');break;case 5:s.controlEveryFESteps=2;break;
    case 6:s.samplePhase=1;break;case 7:s.deterministicSeed=1;break;case 8:s.sensors[1].sensorId=s.sensors[0].sensorId;break;
    case 9:s.sensors[0].quantity=static_cast<RStructuralNativeMeasurementQuantity>(99);break;case 10:s.sensors[0].physicalMemberId=701;break;
    case 11:s.sensors[0].physicalNodeId=999;break;case 12:s.sensors[3].physicalMemberId=999;break;case 13:s.sensors[4].portId="missing";break;
    case 14:s.actions[3].portId=s.actions[2].portId;break;case 15:s.actions[0].quantity=static_cast<RStructuralNativeActionQuantity>(2);break;
    case 16:s.actions[0].delayTicks=1;break;case 17:s.actions[0].holdTicks=2;break;case 18:s.actions[0].noiseAmplitude=0.1;break;
    case 19:s.piLoops[0].proportionalGain=std::numeric_limits<double>::quiet_NaN();break;case 20:s.piLoops[1].actionId=s.piLoops[0].actionId;break;
    case 21:s.piLoops.pop_back();break;case 22:s.actions[2].quantity=RStructuralNativeActionQuantity::Charge;break;
    case 23:s.actions[1].physicalMemberId=701;break;case 24:s.actions[2].initialCommand=0;break;case 25:s.actions[0].initialCommand=1;break;
    case 26:s.actions[0].minimum=3;break;case 27:s.actions[0].rateLimit=std::numeric_limits<double>::quiet_NaN();break;
    case 28:s.sensors[0].sensorId="unsafe\"id";break;case 29:s.sensors.resize(4097);break;case 30:s.source=RStructuralNativeControlSource::FrozenTable;break;
    }
    QVERIFY_EXCEPTION_THROWN(s.validate(m),RError);
}
void TestStructuralNativeControlModel::invalidState_data()
{
    QTest::addColumn<int>("mutation");const char *names[]={"hidden-unavailable","schema","source","revision","counter","phase","missing-envelope","oversized-envelope","bad-frame-counter","sample-quantity","sample-id","nonfinite-sample","action-id","unknown-flags","wrong-flags","wrong-conditioning","held-mismatch","missing-held","nonfinite-record","zero-step-previous","frozen-state","initial-held"};
    for(int i=0;i<22;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralNativeControlModel::invalidState()
{
    QFETCH(int,mutation); auto s=spec(); auto r=state(s,QString(64,'a')); r.validate(s);
    switch(mutation) {
    case 0:r.available=false;break;case 1:r.schemaVersion=2;break;case 2:r.sourceFingerprint="bad";break;case 3:r.providerRevision="other";break;
    case 4:r.completedControlTicks++;break;case 5:r.fePhase=1;break;case 6:r.controllerState.clear();break;case 7:r.controllerState=QByteArray(16*1024*1024+1,'x');break;
    case 8:r.lastFrame.completedFE=2;break;case 9:r.lastFrame.samples[0].quantity=RStructuralNativeMeasurementQuantity::NodeVelocity;break;
    case 10:r.lastFrame.samples[0].sensorId="missing";break;case 11:r.lastFrame.samples[0].value=std::numeric_limits<double>::infinity();break;
    case 12:r.lastActions[0].actionId="missing";break;case 13:r.lastActions[0].limitFlags=8;break;case 14:r.lastActions[0].limitFlags=0;break;
    case 15:r.lastActions[0].conditioned=0.02;break;case 16:r.heldValues[0]=1;break;case 17:r.heldValues.pop_back();break;
    case 18:r.lastActions[0].previousApplied=std::numeric_limits<double>::infinity();break;case 19:r.completedFE=0;r.completedControlTicks=0;break;
    case 20:s=RStructuralNativeControlSpec();break;case 21:r=state(s,QString(64,'a'),true);r.heldValues[0]=1;r.lastAppliedValues[0]=1;break;
    }
    QVERIFY_EXCEPTION_THROWN(r.validate(s),RError);r.clear();r.validate();QVERIFY(!r.hasNonDefaultData());
}
void TestStructuralNativeControlModel::sourceIdentity()
{
    auto m=scene(); auto &t=m.getProblemSetup().getStructuralTrussDynamicsSetup(); const auto frozen=t.computeSourceFingerprint(m);
    QCOMPARE(frozen,legacySource(m,t)); m.getProblemSetup().setStructuralNativeControlSpec(spec());
    const auto local=t.computeSourceFingerprint(m); QVERIFY(local!=frozen); t.bindToModel(m);
    auto independentOrder=spec(); std::swap(independentOrder.piLoops[0],independentOrder.piLoops[1]); independentOrder.validate(m);
    QVERIFY(independentOrder.identityBytes()!=spec().identityBytes());
    m.getProblemSetup().setStructuralNativeControlState(state(spec(),local)); QCOMPARE(t.computeSourceFingerprint(m),local);
    m.getProblemSetup().getStructuralNativeControlState().controllerState.append('x'); QCOMPARE(t.computeSourceFingerprint(m),local);
    m.getProblemSetup().getStructuralNativeControlSpec().piLoops[0].initialIntegral+=1; QVERIFY(t.computeSourceFingerprint(m)!=local);
}
void TestStructuralNativeControlModel::malformed()
{
    QFETCH(bool,binary); QTemporaryDir dir; auto m=localScene(); const auto path=dir.filePath("broken."+RModel::getDefaultFileExtension(binary)); m.write(path,false);
    { QFile f(path); QVERIFY(f.open(QIODevice::ReadWrite)); QVERIFY(f.resize(f.size()/2)); }
    RModel target=m; target.setName("retained"); const auto before=target.getProblemSetup().getStructuralNativeControlState().fingerprintBytes();
    QVERIFY_EXCEPTION_THROWN(target.read(path),RError); QCOMPARE(target.getName(),QString("retained")); QCOMPARE(target.getProblemSetup().getStructuralNativeControlState().fingerprintBytes(),before);
    const auto outer=dir.filePath("outer");
    { RSaveFile f(outer,binary?RSaveFile::BINARY:RSaveFile::ASCII); f.setVersion(RVersion(1,3,21)); QVERIFY(f.open(QIODevice::WriteOnly));
      if(binary) RFileIO::writeBinary(f,m.getProblemSetup()); else RFileIO::writeAscii(f,m.getProblemSetup()); QVERIFY(f.commit()); }
    { QFile f(outer); QVERIFY(f.open(QIODevice::ReadWrite)); QVERIFY(f.resize(f.size()-3)); }
    auto kept=m.getProblemSetup(); RFile f(outer,binary?RFile::BINARY:RFile::ASCII); f.setVersion(RVersion(1,3,21)); QVERIFY(f.open(QIODevice::ReadOnly));
    if(binary) { QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(f,kept),RError); } else { QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(f,kept),RError); }
    QCOMPARE(kept.getStructuralNativeControlState().fingerprintBytes(),before);
}
void TestStructuralNativeControlModel::hiddenDowngrade()
{
    QFETCH(bool,binary); QTemporaryDir dir;
    for(int mutation=0;mutation<7;++mutation) {
        RProblemSetup p;
        switch(mutation) { case 0:p.getStructuralNativeControlSpec().providerId="hidden";break;case 1:p.getStructuralNativeControlSpec().schemaVersion=2;break;
        case 2:p.getStructuralNativeControlSpec().controlEveryFESteps=2;break;case 3:p.getStructuralNativeControlState().completedFE=1;break;
        case 4:p.getStructuralNativeControlState().controllerState="hidden";break;case 5:p.getStructuralNativeControlState().lastFrame.sampleTime=-0.0;break;
        case 6:p=localScene().getProblemSetup();break; }
        RSaveFile out(dir.filePath("downgrade"),binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,20)); QVERIFY(out.open(QIODevice::WriteOnly));
        if(binary) { QVERIFY_EXCEPTION_THROWN(RFileIO::writeBinary(out,p),RError); } else { QVERIFY_EXCEPTION_THROWN(RFileIO::writeAscii(out,p),RError); }
        QCOMPARE(out.pos(),qint64(0));
    }
}
void TestStructuralNativeControlModel::rawMalformed_data()
{
    QTest::addColumn<int>("mutation");const char *names[]={"bool","schema","enum-alias","count","blob-count","truncation","nonfinite","quoted-hex"};
    for(int i=0;i<8;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralNativeControlModel::rawMalformed()
{
    QFETCH(int,mutation); QTemporaryDir dir; const auto path=dir.filePath("raw");
    if(mutation==7) { QFile f(path); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("1 1 \"provider\" \"revision\" \"digest\" \"0g\""); }
    else {
        RSaveFile f(path,RSaveFile::BINARY); f.setVersion(RVersion(1,3,21)); QVERIFY(f.open(QIODevice::WriteOnly));
        if(mutation==0) f.write(QByteArray(1,char(2)));
        else if(mutation==1) { RFileIO::writeBinary(f,2u); RFileIO::writeBinary(f,0u); }
        else if(mutation==2) { RFileIO::writeBinary(f,1u); RFileIO::writeBinary(f,256u); }
        else if(mutation==3 || mutation==4) {
            RFileIO::writeBinary(f,1u); RFileIO::writeBinary(f,1u);
            for(const auto &v:{QString("provider"),QString("revision"),QString(64,'a')}) RFileIO::writeBinary(f,v);
            RFileIO::writeBinary(f,mutation==4?16777217u:0u);
            if(mutation==3) { RFileIO::writeBinary(f,QString(64,'b')); RFileIO::writeBinary(f,1u); RFileIO::writeBinary(f,0u); const quint64 seed=0; f.write(reinterpret_cast<const char*>(&seed),8); RFileIO::writeBinary(f,4097u); }
        } else {
            auto r=state(spec(),QString(64,'a')); RFileIO::writeBinary(f,r);
        } QVERIFY(f.commit());
        if(mutation==5) { QFile ff(path); QVERIFY(ff.open(QIODevice::ReadWrite)); QVERIFY(ff.resize(ff.size()-1)); }
        if(mutation==6) { QFile ff(path); QVERIFY(ff.open(QIODevice::ReadWrite));
            const qint64 offset=1+4+4+64+4+2+8+8+4+4+4+8+4+7+8+8+4+4;
            QVERIFY(ff.seek(offset)); const double inf=std::numeric_limits<double>::infinity(); QCOMPARE(ff.write(reinterpret_cast<const char*>(&inf),8),qint64(8)); }
    }
    RFile in(path,mutation==7?RFile::ASCII:RFile::BINARY); in.setVersion(RVersion(1,3,21)); QVERIFY(in.open(QIODevice::ReadOnly));
    if(mutation==0 || mutation==5 || mutation==6) { auto r=state(spec(),QString(64,'a')); const auto before=r.fingerprintBytes(); QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,r),RError); QCOMPARE(r.fingerprintBytes(),before); }
    else { auto s=spec(); const auto before=s.identityBytes(); if(mutation==7) { QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(in,s),RError); } else { QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,s),RError); } QCOMPARE(s.identityBytes(),before); }
}
void TestStructuralNativeControlModel::fingerprintCompleteness()
{
    auto s=spec();const auto bytes=s.identityBytes();auto changed=s;changed.piLoops[0].bias=0.0;QVERIFY(changed.identityBytes()!=bytes);
    auto r=state(s,QString(64,'a'));const auto before=r.fingerprintBytes();
    for(int field=0;field<16;++field) {auto c=r;switch(field) {
        case 0:c.available=false;break;case 1:c.schemaVersion++;break;case 2:c.sourceFingerprint[0]='b';break;case 3:c.providerRevision.append('x');break;
        case 4:c.completedFE++;break;case 5:c.completedControlTicks++;break;case 6:c.fePhase++;break;case 7:c.samplePhase++;break;
        case 8:c.lastControllerInputState.append('x');break;case 9:c.controllerState.append('x');break;case 10:c.lastFrame.nextControlTime+=0.1;break;
        case 11:c.lastFrame.samples[0].value=0.0;break;case 12:c.lastActions[0].requested+=1;break;case 13:c.lastActions[0].limitFlags=0;break;
        case 14:c.heldValues[0]+=1;break;case 15:c.lastAppliedValues[0]+=1;break;}
        QVERIFY(c.fingerprintBytes()!=before);
    }
}
QTEST_APPLESS_MAIN(TestStructuralNativeControlModel)
#include "tst_structural_native_control_model.moc"
