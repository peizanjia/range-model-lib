#include <cmath>
#include <set>

#include <QByteArray>

#include <rbl_error.h>

#include "rml_model.h"
#include "rml_structural_dynamics_restart_data.h"
#include "rml_structural_piezo_setup.h"

namespace
{
constexpr std::size_t maximumPiezoPortCount = 1000000u;
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

void validateValues(const RStructuralPiezoSetup &setup, bool requireFingerprint)
{
    if (setup.ports.size() > maximumPiezoPortCount)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Excessive structural piezo port count (%llu).",
                     static_cast<unsigned long long>(setup.ports.size()));
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
    }

    if (setup.enabled && setup.ports.empty())
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Enabled structural piezo setup requires at least one port.");
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
    for (const RStructuralPiezoPort &port : setup.ports)
    {
        if (port.elementId >= model.getNElements())
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo port references an unknown element (%u).",
                         port.elementId);
        }
        const RElement &element = model.getElement(port.elementId);
        if (element.getType() != R_ELEMENT_TRUSS1)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo port element %u is not a TRUSS1 element.",
                         port.elementId);
        }
        if (element.size() != 2)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo port element %u does not have exactly two endpoints.",
                         port.elementId);
        }

        const uint firstNodeId = element.getNodeId(0);
        const uint secondNodeId = element.getNodeId(1);
        if (firstNodeId >= model.getNNodes() || secondNodeId >= model.getNNodes() ||
            firstNodeId == secondNodeId)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural piezo port element %u has invalid or repeated endpoint IDs.",
                         port.elementId);
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
                         "Structural piezo port element %u has non-finite endpoint coordinates or zero length.",
                         port.elementId);
        }
    }
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

    if (!this->meshFingerprint.isEmpty() &&
        this->meshFingerprint != RStructuralDynamicsRestartData::computeMeshFingerprint(model))
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural piezo setup is bound to a different mesh.");
    }
}

void RStructuralPiezoSetup::bindToModel(const RModel &model)
{
    // Validate port values and topology before replacing any previous binding.
    validateValues(*this,false);
    validateTopology(*this,model);
    this->meshFingerprint = RStructuralDynamicsRestartData::computeMeshFingerprint(model);
    this->validate(model);
}
