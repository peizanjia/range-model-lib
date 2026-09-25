#include <cmath>
#include <set>

#include <rbl_error.h>

#include "rml_model.h"
#include "rml_structural_cable_setup.h"
#include "rml_structural_dynamics_restart_data.h"

namespace
{
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

void validateValues(const RStructuralCableSetup &setup, bool requireFingerprint)
{
    if (setup.members.size() > 1000000u)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Excessive structural cable member count (%llu).",
                     static_cast<unsigned long long>(setup.members.size()));
    }
    if (setup.maxIterations == 0 || setup.maxLineSearchIterations == 0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural cable iteration limits must be positive.");
    }
    if (!std::isfinite(setup.absoluteForceTolerance) || setup.absoluteForceTolerance <= 0.0 ||
        !std::isfinite(setup.relativeForceTolerance) || setup.relativeForceTolerance <= 0.0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural cable force tolerances must be finite and positive.");
    }

    std::set<uint> elementIds;
    for (const RStructuralCableMember &member : setup.members)
    {
        if (!std::isfinite(member.referenceLength) || member.referenceLength <= 0.0 ||
            !std::isfinite(member.initialTension) || member.initialTension < 0.0)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural cable member length and initial tension are invalid.");
        }
        if (!elementIds.insert(member.elementId).second)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural cable member element IDs must be unique.");
        }
    }

    if (setup.enabled && setup.members.empty())
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Enabled structural cable setup requires at least one member.");
    }
    if ((requireFingerprint && setup.enabled) || !setup.meshFingerprint.isEmpty())
    {
        if (!isSha256Fingerprint(setup.meshFingerprint))
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural cable mesh fingerprint is missing or invalid.");
        }
    }
}

void validateTopology(const RStructuralCableSetup &setup, const RModel &model)
{
    for (const RStructuralCableMember &member : setup.members)
    {
        if (member.elementId >= model.getNElements())
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural cable member references an unknown element (%u).",
                         member.elementId);
        }
        if (model.getElement(member.elementId).getType() != R_ELEMENT_TRUSS1)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural cable member element %u is not a TRUSS1 element.",
                         member.elementId);
        }
        const RElement &element = model.getElement(member.elementId);
        if (element.size() != 2)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural cable member element %u does not have exactly two endpoints.",
                         member.elementId);
        }
        const uint firstNodeId = element.getNodeId(0);
        const uint secondNodeId = element.getNodeId(1);
        if (firstNodeId >= model.getNNodes() || secondNodeId >= model.getNNodes() ||
            firstNodeId == secondNodeId)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural cable member element %u has invalid or repeated endpoint IDs.",
                         member.elementId);
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
                         "Structural cable member element %u has non-finite endpoint coordinates or zero length.",
                         member.elementId);
        }
    }
}
}

void RStructuralCableSetup::validate() const
{
    validateValues(*this,true);
}

void RStructuralCableSetup::validate(const RModel &model) const
{
    this->validate();
    validateTopology(*this,model);

    if (!this->meshFingerprint.isEmpty() &&
        this->meshFingerprint != RStructuralDynamicsRestartData::computeMeshFingerprint(model))
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural cable setup is bound to a different mesh.");
    }
}

void RStructuralCableSetup::bindToModel(const RModel &model)
{
    // Validate the member list and its topology before replacing any prior
    // fingerprint. This intentionally bypasses only the binding requirement.
    validateValues(*this,false);
    validateTopology(*this,model);
    this->meshFingerprint = RStructuralDynamicsRestartData::computeMeshFingerprint(model);
    this->validate(model);
}
