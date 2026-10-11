#ifndef RML_STRUCTURAL_NATIVE_CONTROL_H
#define RML_STRUCTURAL_NATIVE_CONTROL_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>
#include <limits>
#include <vector>

class RModel;

enum class RStructuralNativeControlSource : quint8 { FrozenTable = 0, LocalV3 = 1 };
enum class RStructuralNativeMeasurementQuantity : quint8
{
    NodeDisplacement = 0, NodeVelocity = 1, NodeAcceleration = 2,
    MemberAxialForce = 3, PortVoltage = 4, PortCharge = 5, SupportReaction = 6
};
//! Explicit mapping to supported typed actions. Values 2 and 3 are unsupported.
enum class RStructuralNativeActionQuantity : quint8
{
    Force = 0, CableReferenceLength = 1, Voltage = 4, Charge = 5
};

QString rStructuralNativeMeasurementUnit(RStructuralNativeMeasurementQuantity quantity);
QString rStructuralNativeActionUnit(RStructuralNativeActionQuantity quantity);

struct RStructuralNativeControlSensor
{
    QString sensorId;
    RStructuralNativeMeasurementQuantity quantity = RStructuralNativeMeasurementQuantity::NodeDisplacement;
    uint physicalNodeId = 0;
    uint physicalMemberId = 0;
    QString portId;
    uint axis = 0; //!< Global XYZ for node quantities.
};

struct RStructuralNativeControlAction
{
    QString actionId;
    RStructuralNativeActionQuantity quantity = RStructuralNativeActionQuantity::Force;
    uint physicalNodeId = 0;
    uint physicalMemberId = 0;
    QString portId;
    uint axis = 0;
    double minimum = -std::numeric_limits<double>::infinity();
    double maximum = std::numeric_limits<double>::infinity();
    double rateLimit = std::numeric_limits<double>::infinity(); //!< SI quantity / s.
    double initialCommand = 0.0;
    //! Declarations are persisted; the first N=1 implementation rejects nondefaults.
    uint delayTicks = 0;
    uint holdTicks = 1;
    double noiseAmplitude = 0.0;
};

struct RStructuralNativeControlPILoop
{
    QString actionId;
    QString sensorId;
    double referenceValue = 0.0; //!< Sensor SI units.
    double proportionalGain = 0.0; //!< ActionUnit / SensorUnit.
    double integralGain = 0.0; //!< ActionUnit / (SensorUnit*s).
    double bias = 0.0; //!< Action SI units.
    double initialIntegral = 0.0; //!< SensorUnit*s.
};

struct RStructuralNativeControlSpec
{
    uint schemaVersion = 1;
    RStructuralNativeControlSource source = RStructuralNativeControlSource::FrozenTable;
    QString providerId;
    QString providerRevision;
    QString implementationSha256;
    QByteArray dependencyEnvironmentManifest;
    QString dependencyEnvironmentSha256;
    uint controlEveryFESteps = 1;
    uint samplePhase = 0;
    quint64 deterministicSeed = 0;
    std::vector<RStructuralNativeControlSensor> sensors;
    std::vector<RStructuralNativeControlAction> actions;
    std::vector<RStructuralNativeControlPILoop> piLoops;

    void validate() const;
    void validate(const RModel &model) const;
    bool hasNonDefaultData() const;
    //! Canonical little-endian binary64 spec identity, excluding dynamic state.
    QByteArray identityBytes() const;
};

struct RStructuralNativeControlSample
{
    QString sensorId;
    RStructuralNativeMeasurementQuantity quantity = RStructuralNativeMeasurementQuantity::NodeDisplacement;
    double value = 0.0;
};

struct RStructuralNativeControlFrame
{
    quint64 completedFE = 0;
    quint64 controlTick = 0; //!< Zero-based tick being proposed.
    uint fePhase = 0;
    uint samplePhase = 0;
    double sampleTime = 0.0;
    double nextFETime = 0.0;
    double nextControlTime = 0.0;
    double physicalDt = 0.0;
    std::vector<RStructuralNativeControlSample> samples;
};

enum RStructuralNativeControlLimitFlag : quint32
{
    R_NATIVE_CONTROL_NOT_LIMITED = 0,
    R_NATIVE_CONTROL_AMPLITUDE_LIMITED = 1,
    R_NATIVE_CONTROL_RATE_LIMITED = 2
};

struct RStructuralNativeControlActionRecord
{
    QString actionId;
    RStructuralNativeActionQuantity quantity = RStructuralNativeActionQuantity::Force;
    double previousApplied = 0.0;
    double requested = 0.0;
    double conditioned = 0.0;
    double applied = 0.0;
    quint32 limitFlags = R_NATIVE_CONTROL_NOT_LIMITED;
};

struct RStructuralNativeControlState
{
    bool available = false;
    uint schemaVersion = 1;
    QString sourceFingerprint;
    QString providerRevision;
    quint64 completedFE = 0;
    quint64 completedControlTicks = 0;
    uint fePhase = 0;
    uint samplePhase = 0;
    //! Both envelopes certify the latest pure old-I output / next-I proposal.
    QByteArray lastControllerInputState;
    QByteArray controllerState;
    //! Left accepted frame consumed by the latest accepted control tick.
    RStructuralNativeControlFrame lastFrame;
    std::vector<RStructuralNativeControlActionRecord> lastActions;
    std::vector<double> heldValues;
    std::vector<double> lastAppliedValues;

    void clear();
    void validate() const;
    void validate(const RStructuralNativeControlSpec &spec) const;
    bool hasNonDefaultData() const;
    QByteArray fingerprintBytes() const;
};

#endif // RML_STRUCTURAL_NATIVE_CONTROL_H
