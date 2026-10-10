#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <vector>

#include <QByteArray>
#include <QCryptographicHash>

#include <rbl_error.h>

#include "rml_model.h"
#include "rml_structural_dynamics_restart_data.h"
#include "rml_structural_piezo_setup.h"

namespace
{
constexpr std::size_t maximumPiezoPortCount = 1000000u;
constexpr std::size_t maximumPiezoBoundaryCount = 1000000u;
constexpr qsizetype maximumPiezoPortIdBytes = 4096;

bool isSha256Fingerprint(const QString &value)
{
    if (value.size() != 64) return false;
    for (const QChar character : value)
    {
        if (!((character >= QLatin1Char('0') && character <= QLatin1Char('9')) ||
              (character >= QLatin1Char('a') && character <= QLatin1Char('f'))))
        {
            return false;
        }
    }
    return true;
}

void appendUInt(QByteArray &bytes, uint value)
{
    for (unsigned int byte = 0; byte < 4; ++byte)
    {
        bytes.append(char((value >> (8 * byte)) & 0xffu));
    }
}

void appendExactDouble(QByteArray &bytes, double value)
{
    static_assert(sizeof(double) == sizeof(quint64),
                  "Exact piezo fingerprint requires IEEE-754 binary64 doubles.");
    static_assert(std::numeric_limits<double>::is_iec559,
                  "Exact piezo fingerprint requires IEEE-754 binary64 doubles.");

    quint64 bits = 0;
    std::memcpy(&bits,&value,sizeof(bits));
    for (unsigned int byte = 0; byte < 8; ++byte)
    {
        bytes.append(char((bits >> (8 * byte)) & 0xffu));
    }
}

void appendUtf8(QByteArray &bytes, const QString &value)
{
    const QByteArray encoded = value.toUtf8();
    appendUInt(bytes,uint(encoded.size()));
    bytes.append(encoded);
}

QString digest(const QByteArray &bytes)
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
}

void validateValues(const RStructuralPiezoSetup &setup, bool requireFingerprint)
{
    if (setup.ports.size() > maximumPiezoPortCount)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Excessive structural piezo port count (%llu).",
                     static_cast<unsigned long long>(setup.ports.size()));
    }
    if (setup.passiveElementIds.size() > maximumPiezoPortCount)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Excessive structural piezo passive element count (%llu).",
                     static_cast<unsigned long long>(setup.passiveElementIds.size()));
    }
    if (setup.electricalMode == RStructuralPiezoElectricalMode::NodalPotential &&
        !setup.passiveElementIds.empty())
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Nodal-potential piezo setup cannot contain passive mechanical elements.");
    }
    if (setup.nodalBoundaryConditions.size() > maximumPiezoBoundaryCount)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Excessive structural piezo nodal boundary count (%llu).",
                     static_cast<unsigned long long>(setup.nodalBoundaryConditions.size()));
    }

    if (setup.electricalMode != RStructuralPiezoElectricalMode::LegacyTerminalVoltage &&
        setup.electricalMode != RStructuralPiezoElectricalMode::NodalPotential)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Unsupported structural piezo electrical mode (%d).",
                     int(setup.electricalMode));
    }

    std::set<QString> portIds;
    std::set<uint> elementIds;
    for (const RStructuralPiezoPort &port : setup.ports)
    {
        const QByteArray encodedId = port.id.toUtf8();
        if (port.id.trimmed().isEmpty() || port.id != port.id.trimmed() ||
            encodedId.size() > maximumPiezoPortIdBytes ||
            port.id.contains(QLatin1Char('"')) || port.id.contains(QLatin1Char('\n')) ||
            port.id.contains(QLatin1Char('\r')) || port.id.contains(QChar(0)))
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo port ID is empty or cannot be represented in the model file.");
        }
        if (!portIds.insert(port.id).second)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo port IDs must be unique.");
        }
        if (!elementIds.insert(port.elementId).second)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo port element IDs must be unique.");
        }
        if (!std::isfinite(port.coupling) || !std::isfinite(port.permittivity) ||
            port.permittivity <= 0.0 || !std::isfinite(port.voltage))
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo coupling and voltage must be finite, and permittivity must be finite and positive.");
        }
        if (setup.electricalMode == RStructuralPiezoElectricalMode::NodalPotential &&
            port.voltage != 0.0)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Nodal-potential piezo setup cannot also specify legacy terminal voltage.");
        }
    }

    for (const uint elementId : setup.passiveElementIds)
    {
        if (!elementIds.insert(elementId).second)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo active and passive element IDs must be unique and disjoint.");
        }
    }

    std::set<uint> boundaryNodeIds;
    bool hasPotentialBoundary = false;
    for (const RStructuralPiezoNodeBoundary &boundary : setup.nodalBoundaryConditions)
    {
        if (!boundaryNodeIds.insert(boundary.nodeId).second)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo nodal boundary conditions must use unique node IDs.");
        }
        if (!std::isfinite(boundary.value))
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo nodal boundary value must be finite.");
        }
        if (boundary.type == RStructuralPiezoBoundaryType::Potential)
        {
            hasPotentialBoundary = true;
        }
        else if (boundary.type != RStructuralPiezoBoundaryType::ConductorCharge)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Unsupported structural piezo nodal boundary type (%d).",
                         int(boundary.type));
        }
    }

    if (setup.electricalMode == RStructuralPiezoElectricalMode::LegacyTerminalVoltage &&
        !setup.nodalBoundaryConditions.empty())
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Legacy terminal-voltage piezo setup cannot contain nodal electrical boundary conditions.");
    }
    if (setup.enabled && setup.ports.empty())
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Enabled structural piezo setup requires at least one port.");
    }
    if (setup.enabled && setup.electricalMode == RStructuralPiezoElectricalMode::NodalPotential &&
        !hasPotentialBoundary)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Enabled nodal-potential piezo setup requires a potential gauge boundary.");
    }
    if ((requireFingerprint && setup.enabled) || !setup.meshFingerprint.isEmpty())
    {
        if (!isSha256Fingerprint(setup.meshFingerprint))
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo mesh fingerprint is missing or invalid.");
        }
    }
}

void validateTopology(const RStructuralPiezoSetup &setup, const RModel &model)
{
    std::map<uint,std::set<uint>> electricalGraph;
    std::vector<uint> classifiedElementIds;
    classifiedElementIds.reserve(setup.ports.size()+setup.passiveElementIds.size());
    for (const RStructuralPiezoPort &port : setup.ports) classifiedElementIds.push_back(port.elementId);
    classifiedElementIds.insert(classifiedElementIds.end(),setup.passiveElementIds.begin(),setup.passiveElementIds.end());
    for (std::size_t index = 0; index < classifiedElementIds.size(); ++index)
    {
        const uint elementId = classifiedElementIds[index];
        if (elementId >= model.getNElements())
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo classification references an unknown element (%u).",
                         elementId);
        }
        const RElement &element = model.getElement(elementId);
        if (element.getType() != R_ELEMENT_TRUSS1)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo classified element %u is not a TRUSS1 element.",
                         elementId);
        }
        if (element.size() != 2)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo classified element %u does not have exactly two endpoints.",
                         elementId);
        }

        const uint firstNodeId = element.getNodeId(0);
        const uint secondNodeId = element.getNodeId(1);
        if (firstNodeId >= model.getNNodes() || secondNodeId >= model.getNNodes() ||
            firstNodeId == secondNodeId)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo classified element %u has invalid or repeated endpoint IDs.",
                         elementId);
        }

        const RNode &firstNode = model.getNode(firstNodeId);
        const RNode &secondNode = model.getNode(secondNodeId);
        const double dx = secondNode.getX() - firstNode.getX();
        const double dy = secondNode.getY() - firstNode.getY();
        const double dz = secondNode.getZ() - firstNode.getZ();
        const double currentLength = std::hypot(std::hypot(dx,dy),dz);
        if (!std::isfinite(firstNode.getX()) || !std::isfinite(firstNode.getY()) ||
            !std::isfinite(firstNode.getZ()) || !std::isfinite(secondNode.getX()) ||
            !std::isfinite(secondNode.getY()) || !std::isfinite(secondNode.getZ()) ||
            !std::isfinite(currentLength) || currentLength <= 0.0)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo classified element %u has non-finite endpoint coordinates or zero length.",
                         elementId);
        }
        if (index < setup.ports.size())
        {
            electricalGraph[firstNodeId].insert(secondNodeId);
            electricalGraph[secondNodeId].insert(firstNodeId);
        }
    }

    if (!setup.passiveElementIds.empty())
    {
        const std::set<uint> classified(classifiedElementIds.begin(),classifiedElementIds.end());
        for (uint elementId = 0; elementId < model.getNElements(); ++elementId)
        {
            if (model.getElement(elementId).getType() == R_ELEMENT_TRUSS1 &&
                classified.find(elementId) == classified.end())
            {
                throw RError(RError::Type::Application,R_ERROR_REF,
                             "Structural piezo TRUSS1 element %u lacks an active or passive classification.",elementId);
            }
        }
    }

    if (setup.electricalMode != RStructuralPiezoElectricalMode::NodalPotential)
    {
        return;
    }

    for (const RStructuralPiezoNodeBoundary &boundary : setup.nodalBoundaryConditions)
    {
        if (electricalGraph.find(boundary.nodeId) == electricalGraph.end())
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo nodal boundary references a node outside the piezo network (%u).",
                         boundary.nodeId);
        }
    }

    std::map<uint,RStructuralPiezoBoundaryType> boundaryTypes;
    for (const RStructuralPiezoNodeBoundary &boundary : setup.nodalBoundaryConditions)
    {
        boundaryTypes.emplace(boundary.nodeId,boundary.type);
    }

    std::set<uint> visited;
    for (const auto &entry : electricalGraph)
    {
        if (visited.find(entry.first) != visited.end()) continue;
        std::vector<uint> pending{entry.first};
        visited.insert(entry.first);
        bool hasPotentialGauge = false;
        while (!pending.empty())
        {
            const uint nodeId = pending.back();
            pending.pop_back();
            const auto boundary = boundaryTypes.find(nodeId);
            if (boundary != boundaryTypes.end() &&
                boundary->second == RStructuralPiezoBoundaryType::Potential)
            {
                hasPotentialGauge = true;
            }
            for (const uint connectedNodeId : electricalGraph.at(nodeId))
            {
                if (visited.insert(connectedNodeId).second)
                {
                    pending.push_back(connectedNodeId);
                }
            }
        }
        if (!hasPotentialGauge)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Every connected piezo electrical component requires a prescribed potential gauge.");
        }
    }
}

QString computeLegacyFingerprint(const RModel &model, const RStructuralPiezoSetup &setup)
{
    const QString mesh = RStructuralDynamicsRestartData::computeMeshFingerprint(model);
    if (setup.passiveElementIds.empty()) return mesh;
    QByteArray bytes("range-fea/piezo/legacy/mixed-classification/v1");
    appendUtf8(bytes,mesh);
    std::vector<uint> active;
    for (const RStructuralPiezoPort &port : setup.ports) active.push_back(port.elementId);
    std::sort(active.begin(),active.end());
    appendUInt(bytes,uint(active.size()));
    for (const uint elementId : active) appendUInt(bytes,elementId);
    std::vector<uint> passive = setup.passiveElementIds;
    std::sort(passive.begin(),passive.end());
    appendUInt(bytes,uint(passive.size()));
    for (const uint elementId : passive) appendUInt(bytes,elementId);
    return digest(bytes);
}

QString computeExactNodalFingerprint(const RModel &model,
                                     const std::vector<RStructuralPiezoPort> &ports)
{
    QByteArray bytes("range-fea/piezo/nodal-potential/exact/v1");
    appendUInt(bytes,model.getNNodes());
    for (uint nodeId = 0; nodeId < model.getNNodes(); ++nodeId)
    {
        const RNode &node = model.getNode(nodeId);
        if (!std::isfinite(node.getX()) || !std::isfinite(node.getY()) || !std::isfinite(node.getZ()))
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Nodal-potential piezo fingerprint requires finite model coordinates.");
        }
        appendExactDouble(bytes,node.getX());
        appendExactDouble(bytes,node.getY());
        appendExactDouble(bytes,node.getZ());
    }
    appendUInt(bytes,model.getNElements());
    for (uint elementId = 0; elementId < model.getNElements(); ++elementId)
    {
        const RElement &element = model.getElement(elementId);
        appendUInt(bytes,uint(element.getType()));
        appendUInt(bytes,element.size());
        for (uint localNode = 0; localNode < element.size(); ++localNode)
        {
            appendUInt(bytes,element.getNodeId(localNode));
        }
    }

    std::vector<const RStructuralPiezoPort*> orderedPorts;
    orderedPorts.reserve(ports.size());
    for (const RStructuralPiezoPort &port : ports) orderedPorts.push_back(&port);
    std::sort(orderedPorts.begin(),orderedPorts.end(),[](const auto *first, const auto *second)
    {
        return first->elementId < second->elementId;
    });
    appendUInt(bytes,uint(orderedPorts.size()));
    for (const RStructuralPiezoPort *port : orderedPorts)
    {
        appendUtf8(bytes,port->id);
        appendUInt(bytes,port->elementId);
        const RElement &element = model.getElement(port->elementId);
        // Retain native endpoint order because the constitutive coupling is signed.
        appendUInt(bytes,element.getNodeId(0));
        appendUInt(bytes,element.getNodeId(1));
        appendExactDouble(bytes,port->coupling);
        appendExactDouble(bytes,port->permittivity);
        appendExactDouble(bytes,port->voltage);
    }
    return digest(bytes);
}
}

void RStructuralPiezoSetup::validate() const
{
    validateValues(*this,true);
}

void RStructuralPiezoSetup::validate(const RModel &model) const
{
    this->validate();
    validateTopology(*this,model);

    if (!this->meshFingerprint.isEmpty())
    {
        const QString expectedFingerprint = this->electricalMode == RStructuralPiezoElectricalMode::NodalPotential
            ? computeExactNodalFingerprint(model,this->ports)
            : computeLegacyFingerprint(model,*this);
        if (this->meshFingerprint != expectedFingerprint)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo setup is bound to a different mesh or port configuration.");
        }
    }
}

void RStructuralPiezoSetup::bindToModel(const RModel &model)
{
    // Validate values and topology before replacing any previous binding.
    validateValues(*this,false);
    validateTopology(*this,model);
    this->meshFingerprint = this->electricalMode == RStructuralPiezoElectricalMode::NodalPotential
        ? computeExactNodalFingerprint(model,this->ports)
        : computeLegacyFingerprint(model,*this);
    this->validate(model);
}
