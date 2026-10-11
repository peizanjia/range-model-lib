#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

#include <rbl_utils.h>
#include <rbl_error.h>

#include "rml_structural_truss_dynamics_restart_data.h"

namespace
{
constexpr size_t maximumCount = 1000000u;

void require(bool condition, const char *message)
{
    if (!condition) throw RError(RError::Type::InvalidInput,R_ERROR_REF,message);
}

bool isDigest(const QString &value)
{
    if (value.size() != 64) return false;
    for (QChar c : value)
        if (!((c >= QLatin1Char('0') && c <= QLatin1Char('9')) ||
              (c >= QLatin1Char('a') && c <= QLatin1Char('f')))) return false;
    return true;
}

void validateId(const QString &id)
{
    require(!id.isEmpty() && id.trimmed() == id && id.size() <= 4096 &&
            id.toUtf8().size() <= 4096,"Invalid native truss restart port ID.");
    for (int i = 0; i < id.size(); ++i)
    {
        const ushort c = id.at(i).unicode();
        require(c >= 0x20 && c != 0x7f && c != '"' && c != '\\',
                "Invalid character in native truss restart port ID.");
        if (QChar::isHighSurrogate(c))
        {
            require(i+1 < id.size() && id.at(i+1).isLowSurrogate(),
                    "Unpaired surrogate in native truss restart port ID.");
            ++i;
        }
        else require(!QChar::isLowSurrogate(c),"Unpaired surrogate in native truss restart port ID.");
    }
}

void validatePhysicalIds(const std::vector<uint> &ids)
{
    require(!ids.empty() && ids.size() <= maximumCount,"Invalid native truss restart physical ID count.");
    std::set<uint> seen;
    for (uint id : ids)
        require(id != 0 && id != RConstants::eod && seen.insert(id).second,
                "Native truss restart physical IDs must be nonzero and unique.");
}

void validateVector(const std::vector<double> &values, size_t count)
{
    require(values.size() == count,"Native truss restart physical state dimensions do not agree.");
    for (double value : values)
        require(std::isfinite(value),"Non-finite native truss restart physical state.");
}
}

void RStructuralTrussDynamicsRestartData::clear()
{
    *this = RStructuralTrussDynamicsRestartData();
}

bool RStructuralTrussDynamicsRestartData::hasNonDefaultData() const
{
    return available || completedSteps != 0 || runTargetSteps != 0 ||
           initialTime != 0.0 || time != 0.0 || fixedTimeStep != 0.0 || nextCommandIndex != 0 ||
           !orderedPhysicalNodeIds.empty() || !orderedMemberPhysicalIds.empty() || !orderedPortIds.empty() ||
           !portVoltages.empty() || !portCharges.empty() || !portEnthalpies.empty() ||
           !sourceFingerprint.isEmpty() || !systemIdentity.isEmpty() ||
           !q.empty() || !v.empty() || !a.empty() || !reactions.empty() || !externalForce.empty() ||
           !memberInputKinds.empty() || !memberInputValues.empty() ||
           storedEnergy != 0.0 || kineticEnergy != 0.0 || initialTotalEnergy != 0.0 ||
           cumulativeExternalWork != 0.0 || cumulativeSupportWork != 0.0 ||
           cumulativeElectricalWork != 0.0 || cumulativeRestLengthWork != 0.0;
}

void RStructuralTrussDynamicsRestartData::validate() const
{
    if (!available)
    {
        require(!hasNonDefaultData(),"Unavailable native truss restart must contain only default data.");
        return;
    }
    require(runTargetSteps > 0 && runTargetSteps <= maximumCount && completedSteps <= runTargetSteps &&
            nextCommandIndex <= maximumCount,"Invalid native truss restart step or command index.");
    require(std::isfinite(initialTime) && std::isfinite(time) &&
            std::isfinite(fixedTimeStep) && fixedTimeStep > 0.0,
            "Invalid native truss restart time grid.");
    const double elapsed = double(completedSteps)*fixedTimeStep;
    const double expectedTime = initialTime+elapsed;
    const double targetTime = initialTime+double(runTargetSteps)*fixedTimeStep;
    require(std::isfinite(elapsed) && std::isfinite(expectedTime) && std::isfinite(targetTime),
            "Non-finite native truss restart time product.");
    const double scale = std::max({fixedTimeStep,std::abs(initialTime),std::abs(elapsed),std::abs(expectedTime),std::abs(time)});
    // The driver adds dt to each accepted clock. Allow only the accumulated
    // floating-point addition bound, without an absolute one-second floor.
    const double rounding = double(std::max(1u,completedSteps))*std::numeric_limits<double>::epsilon();
    const double gamma = rounding/(1.0-rounding);
    require(std::abs(time-expectedTime) <= (4.0*gamma)*scale,
            "Native truss restart clock does not match its completed fixed steps.");
    require(isDigest(sourceFingerprint) && systemIdentity.size() == 32,
            "Invalid native truss restart source or system identity.");
    validatePhysicalIds(orderedPhysicalNodeIds);
    validatePhysicalIds(orderedMemberPhysicalIds);
    require(orderedPortIds.size() <= maximumCount && orderedPortIds.size() <= orderedMemberPhysicalIds.size(),
            "Invalid native truss restart port count.");
    std::set<QString> portIds;
    for (const QString &id : orderedPortIds)
    {
        validateId(id);
        require(portIds.insert(id).second,"Native truss restart port IDs must be unique.");
    }
    validateVector(portVoltages,orderedPortIds.size());
    validateVector(portCharges,orderedPortIds.size());
    validateVector(portEnthalpies,orderedPortIds.size());
    const size_t physicalSize = 3u*orderedPhysicalNodeIds.size();
    validateVector(q,physicalSize);
    validateVector(v,physicalSize);
    validateVector(a,physicalSize);
    validateVector(reactions,physicalSize);
    validateVector(externalForce,physicalSize);
    require(memberInputKinds.size() == orderedMemberPhysicalIds.size(),
            "Native truss restart member input dimensions do not agree.");
    validateVector(memberInputValues,orderedMemberPhysicalIds.size());
    size_t electricalCount = 0;
    for (size_t i = 0; i < memberInputKinds.size(); ++i)
    {
        const uint kind = memberInputKinds[i];
        require(kind <= 3,"Invalid native truss restart member input kind.");
        if (kind == 0) require(memberInputValues[i] == 0.0,"Passive restart member must have zero input.");
        if (kind == 3) require(memberInputValues[i] > 0.0,"Cable restart reference length must be positive.");
        if (kind == 1 || kind == 2) ++electricalCount;
    }
    require(electricalCount == orderedPortIds.size(),"Native truss restart electrical inputs and ports do not agree.");
    require(std::isfinite(storedEnergy) && storedEnergy >= 0.0 &&
            std::isfinite(kineticEnergy) && kineticEnergy >= 0.0 &&
            std::isfinite(initialTotalEnergy) && initialTotalEnergy >= 0.0 &&
            std::isfinite(cumulativeExternalWork) && std::isfinite(cumulativeSupportWork) &&
            std::isfinite(cumulativeElectricalWork) && std::isfinite(cumulativeRestLengthWork),
            "Invalid native truss restart energy or work ledger.");
}
