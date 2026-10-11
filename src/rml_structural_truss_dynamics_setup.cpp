#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <utility>

#include <QCryptographicHash>

#include <rbl_error.h>

#include "rml_model.h"
#include "rml_structural_truss_dynamics_setup.h"

namespace
{
constexpr size_t maximumCount = 1000000u;

void require(bool condition, const char *message)
{
    if (!condition) throw RError(RError::Type::InvalidInput,R_ERROR_REF,message);
}

void finite(double value)
{
    require(std::isfinite(value),"Native truss source contains a non-finite value.");
}

void positive(double value)
{
    require(std::isfinite(value) && value > 0.0,"Native truss parameter must be finite and positive.");
}

double positiveProduct(double first, double second, double third = 1.0)
{
    // Mass is rho*A*originalChord. An intermediate rho*A overflow/underflow
    // must not reject a representable final frozen mass.
    positive(first); positive(second); positive(third);
    int firstExponent = 0, secondExponent = 0, thirdExponent = 0;
    const double mantissa = std::frexp(first,&firstExponent)*std::frexp(second,&secondExponent)*std::frexp(third,&thirdExponent);
    const double result = std::ldexp(mantissa,firstExponent+secondExponent+thirdExponent);
    positive(result);
    return result;
}

void physicalId(uint value)
{
    require(value != 0 && value != RConstants::eod,"Native truss physical ID is zero or reserved.");
}

void count(size_t value)
{
    require(value <= maximumCount,"Native truss source exceeds its bounded entry count.");
}

bool isDigest(const QString &value)
{
    if (value.size() != 64) return false;
    for (QChar c : value)
        if (!((c >= QLatin1Char('0') && c <= QLatin1Char('9')) ||
              (c >= QLatin1Char('a') && c <= QLatin1Char('f')))) return false;
    return true;
}

void validateText(const QString &value, const char *field = "Native truss source text")
{
    const auto check = [field](bool condition, const char *reason)
    {
        if (!condition) throw RError(RError::Type::InvalidInput,R_ERROR_REF,"%s: %s",field,reason);
    };
    // Source metadata must survive both existing string encodings exactly.
    // ASCII quoted strings drop leading whitespace, have no quote escape,
    // and text-mode newline conversion can change CR/LF. Binary uses strlen
    // and therefore truncates at NUL. Empty metadata and trailing/interior
    // whitespace (other than CR/LF) remain allowed; IDs impose stricter rules.
    check(value.size() <= 4096,"exceeds the 4096-byte UTF-8 bound.");
    check(value.isEmpty() || !value.front().isSpace(),"leading whitespace does not survive ASCII serialization.");
    for (int i = 0; i < value.size(); ++i)
    {
        const QChar character = value.at(i);
        check(!character.isNull(),"NUL does not survive binary serialization.");
        check(character != QLatin1Char('"'),"quotes do not survive ASCII serialization.");
        check(character != QLatin1Char('\r') && character != QLatin1Char('\n'),
              "CR/LF does not have a stable text-mode serialization.");
        if (value.at(i).isHighSurrogate())
        {
            check(i+1 < value.size() && value.at(i+1).isLowSurrogate(),"contains an unpaired surrogate.");
            ++i;
        }
        else check(!value.at(i).isLowSurrogate(),"contains an unpaired surrogate.");
    }
    check(value.toUtf8().size() <= 4096,"exceeds the 4096-byte UTF-8 bound.");
}

void validateId(const QString &value)
{
    validateText(value);
    require(!value.isEmpty() && value.trimmed() == value,"Native truss ID must be nonempty and unpadded.");
    for (QChar c : value)
        require(c.unicode() >= 0x20 && c.unicode() != 0x7f && c != QLatin1Char('"') && c != QLatin1Char('\\'),
                "Native truss ID contains a forbidden character.");
}

void validateMass(RStructuralTrussMassSource source, double fixedMass)
{
    require(source == RStructuralTrussMassSource::MaterialDensity ||
            source == RStructuralTrussMassSource::FixedMemberMass,"Invalid native truss mass source.");
    if (source == RStructuralTrussMassSource::FixedMemberMass) positive(fixedMass);
    else require(fixedMass == 0.0,"Density-derived native truss mass cannot also have a fixed mass.");
}

void validateValues(const RStructuralTrussDynamicsSetup &setup, bool requireFingerprint)
{
    count(setup.passiveMembers.size());
    count(setup.activeMembers.size());
    count(setup.cableMembers.size());
    count(setup.heldCommands.size());
    require(setup.maxIterations > 0 && setup.maxIterations <= maximumCount &&
            setup.maxLineSearchIterations > 0 && setup.maxLineSearchIterations <= maximumCount,
            "Invalid native truss iteration limits.");
    require(std::isfinite(setup.absoluteForceTolerance) && setup.absoluteForceTolerance >= 0.0 && setup.absoluteForceTolerance <= 1.0e-8 &&
            std::isfinite(setup.relativeForceTolerance) && setup.relativeForceTolerance >= 0.0 && setup.relativeForceTolerance <= 1.0e-8 &&
            (setup.absoluteForceTolerance > 0.0 || setup.relativeForceTolerance > 0.0),
            "Native truss force tolerances must agree with the accepted-state certificate bounds.");
    positive(setup.linearRelativeTolerance);
    require(setup.linearRelativeTolerance < 1.0,"Native truss linear relative tolerance must be below one.");
    if (!setup.sourceFingerprint.isEmpty() || (setup.enabled && requireFingerprint))
        require(isDigest(setup.sourceFingerprint),"Native truss source fingerprint must be a lowercase SHA-256 digest.");
    std::set<uint> members;
    std::set<QString> ports;
    std::set<uint> cableIds;
    const auto addMember = [&members](uint id)
    {
        physicalId(id);
        require(members.insert(id).second,"Native truss member classification must be unique and disjoint.");
    };
    for (const auto &member : setup.passiveMembers)
    {
        addMember(member.physicalElementId);
        positive(member.referenceLength);
    }
    for (const auto &member : setup.activeMembers)
    {
        addMember(member.physicalElementId);
        validateId(member.portId);
        require(ports.insert(member.portId).second,"Native truss port IDs must be unique.");
        positive(member.referenceLength);
        positive(member.axialStiffness);
        finite(member.coupling);
        positive(member.capacitance);
        finite(member.initialValue);
        require(member.kind == RStructuralTrussElectricalKind::Voltage ||
                member.kind == RStructuralTrussElectricalKind::ConductorCharge,"Invalid native truss electrical kind.");
        validateMass(member.massSource,member.fixedMass);
    }
    for (const auto &member : setup.cableMembers)
    {
        addMember(member.physicalElementId);
        cableIds.insert(member.physicalElementId);
        positive(member.initialReferenceLength);
        require(std::isfinite(member.initialTension) && member.initialTension >= 0.0,
                "Native truss cable initial tension must be finite and nonnegative.");
        validateMass(member.massSource,member.fixedMass);
    }
    if (setup.enabled) require(!members.empty(),"Enabled native truss setup requires members.");
    uint previousStep = 0;
    size_t targets = 0;
    for (const auto &command : setup.heldCommands)
    {
        require(command.stepIndex > previousStep && command.stepIndex <= maximumCount,
                "Native truss command steps must be one-based and strictly increasing.");
        previousStep = command.stepIndex;
        count(command.electrical.size()); count(command.cables.size()); count(command.nodalForces.size());
        targets += command.electrical.size()+command.cables.size()+command.nodalForces.size();
        count(targets);
        std::set<QString> rowPorts;
        std::set<uint> rowCables, rowNodes;
        for (const auto &input : command.electrical)
        {
            validateId(input.portId);
            finite(input.value);
            require(ports.count(input.portId) == 1 && rowPorts.insert(input.portId).second,
                    "Native truss electrical command target is unknown or duplicated.");
        }
        for (const auto &input : command.cables)
        {
            physicalId(input.physicalElementId);
            positive(input.referenceLength);
            require(cableIds.count(input.physicalElementId) == 1 && rowCables.insert(input.physicalElementId).second,
                    "Native truss cable command target is unknown or duplicated.");
        }
        for (const auto &input : command.nodalForces)
        {
            physicalId(input.physicalNodeId);
            require(rowNodes.insert(input.physicalNodeId).second,"Native truss nodal force target is duplicated.");
            for (double value : input.force) finite(value);
        }
    }
}

double materialValue(const RMaterial &material, RMaterialProperty::Type type)
{
    const uint position = material.findPosition(type);
    require(position < material.size(),"Native truss line material lacks a required property.");
    const auto &property = material.get(position);
    require(property.getKeyType() == R_VARIABLE_TEMPERATURE && !property.empty(),
            "Native truss material property must have a nonempty temperature table.");
    count(property.size());
    for (uint i = 0; i < property.size(); ++i) { finite(property.getKey(i)); finite(property.getValue(i)); }
    const double value = property.get(293.15);
    positive(value);
    return value;
}

void validateTopology(const RStructuralTrussDynamicsSetup &setup, const RModel &model)
{
    count(model.getNNodes()); count(model.getNElements()); count(model.getNLines()); count(model.getNPoints());
    std::set<uint> nodes;
    std::map<uint,uint> elements;
    for (uint i = 0; i < model.getNNodes(); ++i)
    {
        physicalId(model.getNodePhysicalId(i));
        require(nodes.insert(model.getNodePhysicalId(i)).second,"Duplicate native truss model physical node ID.");
        const auto &node = model.getNode(i);
        finite(node.getX()); finite(node.getY()); finite(node.getZ());
    }
    std::map<uint,uint> owner;
    for (uint i = 0; i < model.getNLines(); ++i)
    {
        const auto &line = model.getLine(i);
        count(line.size());
        for (uint j = 0; j < line.size(); ++j)
        {
            const uint element = line.get(j);
            require(element < model.getNElements(),"Native truss line contains an unknown element position.");
            require(owner.emplace(element,i).second,"Native truss element must have a unique line owner.");
        }
    }
    uint trussCount = 0;
    for (uint i = 0; i < model.getNElements(); ++i)
    {
        physicalId(model.getElementPhysicalId(i));
        require(elements.emplace(model.getElementPhysicalId(i),i).second,"Duplicate native truss model physical element ID.");
        const auto &element = model.getElement(i);
        count(element.size());
        for (uint j = 0; j < element.size(); ++j)
            require(element.getNodeId(j) < model.getNNodes(),"Native truss element has an unknown endpoint position.");
        if (element.getType() == R_ELEMENT_TRUSS1) ++trussCount;
        if (setup.enabled)
            require(element.getType() == R_ELEMENT_TRUSS1 || element.getType() == R_ELEMENT_POINT,
                    "Native mixed dynamics supports only TRUSS1 and POINT elements.");
    }
    const auto memberProperties = [&model,&owner,&elements](uint id, bool elastic, RStructuralTrussMassSource massSource)
    {
        const auto found = elements.find(id);
        require(found != elements.end(),"Native truss member references an unknown physical element ID.");
        const uint position = found->second;
        const auto &element = model.getElement(position);
        require(element.getType() == R_ELEMENT_TRUSS1 && element.size() == 2,
                "Native truss member must reference a two-endpoint TRUSS1.");
        const uint first = element.getNodeId(0), second = element.getNodeId(1);
        require(first < model.getNNodes() && second < model.getNNodes() && first != second,
                "Native truss member endpoints are invalid or repeated.");
        const auto &a = model.getNode(first); const auto &b = model.getNode(second);
        const double chord = std::hypot(std::hypot(b.getX()-a.getX(),b.getY()-a.getY()),b.getZ()-a.getZ());
        positive(chord);
        const auto entry = owner.find(position);
        require(entry != owner.end(),"Native truss member has no line owner.");
        const auto &line = model.getLine(entry->second);
        positive(line.getCrossArea());
        if (elastic) positiveProduct(materialValue(line.getMaterial(),RMaterialProperty::ModulusOfElasiticity),line.getCrossArea());
        if (massSource == RStructuralTrussMassSource::MaterialDensity)
            positiveProduct(materialValue(line.getMaterial(),RMaterialProperty::Density),line.getCrossArea(),chord);
    };
    for (const auto &member : setup.passiveMembers)
        memberProperties(member.physicalElementId,true,RStructuralTrussMassSource::MaterialDensity);
    for (const auto &member : setup.activeMembers)
        memberProperties(member.physicalElementId,false,member.massSource);
    for (const auto &member : setup.cableMembers)
        memberProperties(member.physicalElementId,true,member.massSource);
    if (setup.enabled)
    {
        require(trussCount == setup.passiveMembers.size()+setup.activeMembers.size()+setup.cableMembers.size(),
                "Enabled native truss classification must cover every TRUSS1 element.");
        const auto &dynamics = model.getProblemSetup().getStructuralDynamicsSetup();
        require(dynamics.getEnabled() && dynamics.getMethod() == RStructuralTimeIntegrationMethod::Newmark &&
                dynamics.getNewmarkBeta() == 0.25 && dynamics.getNewmarkGamma() == 0.5 &&
                dynamics.getRayleighAlpha() == 0.0 && dynamics.getRayleighBeta() == 0.0,
                "Native truss dynamics requires enabled undamped Newmark (beta=.25, gamma=.5).");
        for (double ratio : dynamics.getModalDampingRatios())
            require(ratio == 0.0,"Native truss dynamics requires zero modal damping.");
        positive(dynamics.getTimeStep());
        require(dynamics.getNSteps() > 0 && dynamics.getNSteps() <= maximumCount &&
                dynamics.getOutputFrequency() > 0 && dynamics.getOutputFrequency() <= maximumCount,
                "Invalid native truss authoritative dynamics time grid.");
        const double initialTime = model.getTimeSolver().getInputStartTime()-dynamics.getTimeStep();
        finite(initialTime);
        finite(double(dynamics.getNSteps())*dynamics.getTimeStep());
        finite(initialTime+double(dynamics.getNSteps())*dynamics.getTimeStep());
        for (const auto &command : setup.heldCommands)
            require(command.stepIndex <= dynamics.getNSteps(),"Native truss command step exceeds the run target.");
    }
    for (const auto &command : setup.heldCommands)
        for (const auto &input : command.nodalForces)
            require(nodes.count(input.physicalNodeId) == 1,
                    "Native truss nodal force references an unknown physical node ID.");
}

//! Incremental, endian-defined encoding avoids allocating a second full model.
class SourceWriter
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

void writeTable(SourceWriter &writer, const RValueTable &table)
{
    writer.text(table.getKeyName(),"Native truss table key name"); writer.text(table.getKeyUnits(),"Native truss table key units");
    writer.text(table.getValueName(),"Native truss table value name"); writer.text(table.getValueUnits(),"Native truss table value units");
    writer.entries(table.size());
    for (uint i = 0; i < table.size(); ++i) { writer.real(table.getKey(i)); writer.real(table.getValue(i)); }
}

void writeCondition(SourceWriter &writer, const RCondition &condition)
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

void writeGroup(SourceWriter &writer, const RElementGroup &group, const RModel &model)
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

void writeMixedSetup(SourceWriter &writer, const RStructuralTrussDynamicsSetup &setup)
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

void writeDof(SourceWriter &writer, const RStructuralDofRef &dof)
{
    writer.integer(dof.nodeId); writer.integer(uint(dof.dof));
}

void writeTarget(SourceWriter &writer, const RStructuralTarget &target)
{
    writer.integer(uint(target.scope)); writer.integer(target.nodeId);
    writer.text(target.setName); writer.integer(target.entityType);
}

void writeInteraction(SourceWriter &writer, const RStructuralInteractionSetup &interaction)
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

void writeTask(SourceWriter &writer, const RProblemTaskItem &task, uint depth, size_t &total)
{
    require(depth <= 256,"Native truss problem tree exceeds its depth bound.");
    count(++total);
    writer.integer(task.getProblemType()); writer.integer(task.getNIterations()); writer.entries(task.getNChildren());
    for (uint i = 0; i < task.getNChildren(); ++i) writeTask(writer,task.getChild(i),depth+1,total);
}

void writeOtherStructuralSetup(SourceWriter &writer, const RProblemSetup &problem)
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
}

void RStructuralTrussDynamicsSetup::validate() const
{
    validateValues(*this,true);
}

void RStructuralTrussDynamicsSetup::validate(const RModel &model) const
{
    validate();
    validateTopology(*this,model);
    if (!sourceFingerprint.isEmpty())
        require(sourceFingerprint == computeSourceFingerprint(model),"Native truss setup is bound to a changed source model.");
}

void RStructuralTrussDynamicsSetup::bindToModel(const RModel &model)
{
    RStructuralTrussDynamicsSetup candidate = *this;
    validateValues(candidate,false);
    validateTopology(candidate,model);
    candidate.sourceFingerprint = candidate.computeSourceFingerprint(model);
    candidate.validate();
    *this = std::move(candidate);
}

bool RStructuralTrussDynamicsSetup::hasNonDefaultData() const
{
    return enabled || !sourceFingerprint.isEmpty() || !passiveMembers.empty() || !activeMembers.empty() ||
           !cableMembers.empty() || !heldCommands.empty() || maxIterations != 50 || maxLineSearchIterations != 24 ||
           absoluteForceTolerance != 1.0e-9 || relativeForceTolerance != 1.0e-9 || linearRelativeTolerance != 1.0e-10;
}

QString RStructuralTrussDynamicsSetup::computeSourceFingerprint(const RModel &model) const
{
    validateValues(*this,false);
    validateTopology(*this,model);
    SourceWriter writer;
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
    writeMixedSetup(writer,*this);
    writeOtherStructuralSetup(writer,model.getProblemSetup());
    writer.real(model.getTimeSolver().getInputStartTime());
    size_t taskCount = 0;
    writeTask(writer,model.getProblemTaskTree(),0,taskCount);
    return writer.digest();
}
