#include <cmath>
#include <set>

#include <rbl_error.h>

#include "rml_structural_thermal_control_setup.h"

namespace
{
constexpr std::size_t maxThermalChannels = 4096u;
constexpr std::size_t maxSparseObservationNodes = 1000000u;
constexpr qsizetype maxStableIdLength = 128;

bool validStableId(const QString &id)
{
    return !id.isEmpty() && id.size() <= maxStableIdLength && id == id.trimmed();
}
}

void RStructuralThermalControlSetup::validate() const
{
    if (!this->enabled)
    {
        if (this->thermalSubstepSeconds != 0.0 ||
            this->thermalSubstepsPerMechanicalMacrostep != 1 ||
            this->mechanicalMacrostepsPerControllerSample != 1 ||
            !this->channels.empty() || !this->sparseObservationNodeIds.empty())
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Disabled thermal control setup must not retain configuration data.");
        }
        return;
    }

    if (!std::isfinite(this->thermalSubstepSeconds) || this->thermalSubstepSeconds <= 0.0 ||
        this->thermalSubstepsPerMechanicalMacrostep == 0 ||
        this->mechanicalMacrostepsPerControllerSample == 0 ||
        this->channels.empty() || this->channels.size() > maxThermalChannels ||
        this->sparseObservationNodeIds.size() > maxSparseObservationNodes)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Thermal control interval, rates or configuration counts are invalid.");
    }

    const double mechanicalPeriod = this->thermalSubstepSeconds *
                                    double(this->thermalSubstepsPerMechanicalMacrostep);
    if (!std::isfinite(mechanicalPeriod) ||
        !std::isfinite(mechanicalPeriod *
                       double(this->mechanicalMacrostepsPerControllerSample)))
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Thermal and controller intervals must remain finite.");
    }

    std::set<QString> actuatorIds;
    std::set<QString> sensorIds;
    for (const auto &channel : this->channels)
    {
        if (!validStableId(channel.actuatorId) || !validStableId(channel.heaterNodeId) ||
            !validStableId(channel.temperatureSensorId) ||
            !validStableId(channel.temperatureSensorNodeId) ||
            !actuatorIds.insert(channel.actuatorId).second ||
            !sensorIds.insert(channel.temperatureSensorId).second ||
            !std::isfinite(channel.turnOnBelowKelvin) || channel.turnOnBelowKelvin <= 0.0 ||
            !std::isfinite(channel.turnOffAtOrAboveKelvin) ||
            channel.turnOffAtOrAboveKelvin < channel.turnOnBelowKelvin ||
            !std::isfinite(channel.onPowerWatts) || channel.onPowerWatts < 0.0 ||
            !std::isfinite(channel.minimumPowerWatts) || channel.minimumPowerWatts < 0.0 ||
            !std::isfinite(channel.maximumPowerWatts) ||
            channel.maximumPowerWatts < channel.minimumPowerWatts ||
            channel.onPowerWatts < channel.minimumPowerWatts ||
            channel.onPowerWatts > channel.maximumPowerWatts ||
            !std::isfinite(channel.maximumPowerSlewWattsPerSecond) ||
            channel.maximumPowerSlewWattsPerSecond < 0.0)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Thermal control channel IDs, thresholds or power bounds are invalid.");
        }
    }

    std::set<QString> sparseNodeIds;
    for (const QString &nodeId : this->sparseObservationNodeIds)
    {
        if (!validStableId(nodeId) || !sparseNodeIds.insert(nodeId).second)
        {
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Sparse thermal observation node IDs must be unique stable IDs.");
        }
    }
}
