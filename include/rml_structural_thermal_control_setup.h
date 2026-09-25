#ifndef RML_STRUCTURAL_THERMAL_CONTROL_SETUP_H
#define RML_STRUCTURAL_THERMAL_CONTROL_SETUP_H

#include <QtGlobal>
#include <QString>

#include <vector>

//! Heater-to-node and temperature-sensor bindings for a thermal controller.
struct RStructuralThermalControlChannelSetup
{
    QString actuatorId;
    QString heaterNodeId;
    QString temperatureSensorId;
    QString temperatureSensorNodeId;
    double turnOnBelowKelvin = 0.0;
    double turnOffAtOrAboveKelvin = 0.0;
    double onPowerWatts = 0.0;
    double minimumPowerWatts = 0.0;
    double maximumPowerWatts = 0.0;
    //! A finite value is always stored; the flag explicitly disables its use.
    double maximumPowerSlewWattsPerSecond = 0.0;
    bool maximumPowerSlewUnlimited = true;
};

//! Model-bound multirate thermal-control configuration. Dynamic plant and
//! controller state belongs in the accepted restart checkpoint.
struct RStructuralThermalControlSetup
{
    bool enabled = false;
    double thermalSubstepSeconds = 0.0;
    uint thermalSubstepsPerMechanicalMacrostep = 1;
    uint mechanicalMacrostepsPerControllerSample = 1;
    std::vector<RStructuralThermalControlChannelSetup> channels;
    //! Optional stable physical node IDs included in sparse observations.
    std::vector<QString> sparseObservationNodeIds;

    void validate() const;
};

#endif // RML_STRUCTURAL_THERMAL_CONTROL_SETUP_H
