#ifndef RML_STRUCTURAL_CONTROL_SETUP_H
#define RML_STRUCTURAL_CONTROL_SETUP_H

#include <QtGlobal>
#include <QString>
#include <vector>

//! Optional per-actuator override; absent IDs use zero delay, every-step
//! zero-order hold, and no noise. Noise is a uniform force half-width in N.
struct RStructuralControlChannelConditioning
{
    QString actuatorId;
    uint delaySteps = 0;
    uint holdSteps = 1;
    double noiseAmplitude = 0.0;
};

//! Portable Stage-6 control-session configuration, independent of GUI objects.
//! Protocol v1 samples one controller command per accepted fixed solver step.
struct RStructuralControlSetup
{
    bool enabled = false;
    uint controlProtocolVersion = 1;
    QString controllerId;
    quint64 deterministicSeed = 0;
    std::vector<RStructuralControlChannelConditioning> channelConditioning;

    void validate() const;
};

#endif // RML_STRUCTURAL_CONTROL_SETUP_H
