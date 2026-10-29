#ifndef RML_STRUCTURAL_PIEZO_READOUT_SETUP_H
#define RML_STRUCTURAL_PIEZO_READOUT_SETUP_H

#include <QByteArray>
#include <QString>
#include <array>
#include <vector>

class RModel;

enum class RStructuralPiezoPortRole { Actuator = 0, Sensor = 1, ActuatorSensor = 2 };
enum class RStructuralPiezoReadoutMode
{
    None = 0, OpenCircuitVoltage = 1, ShortCircuitCharge = 2,
    IdealChargeAmplifier = 3, ActuatorCharge = 4
};

enum class RStructuralPiezoReadoutQuantity { None = 0, Voltage = 1, Charge = 2 };
RStructuralPiezoReadoutQuantity rStructuralPiezoReadoutQuantity(RStructuralPiezoReadoutMode mode);
QString rStructuralPiezoReadoutUnit(RStructuralPiezoReadoutMode mode);

struct RStructuralPiezoReadoutPort
{
    QString portId;
    RStructuralPiezoPortRole role = RStructuralPiezoPortRole::Actuator;
    RStructuralPiezoReadoutMode readout = RStructuralPiezoReadoutMode::None;
    double Cf = 0.0; //!< Feedback capacitance [F], ideal amplifier only.
    int amplifierPolarity = -1; //!< Exactly +1 or -1; amplifier only.
    double Qref = 0.0; //!< Reference transferred charge [C], amplifier only.
    double VoutBias = 0.0; //!< Output = bias + polarity*(Q-Qref)/Cf [V].
};

struct RStructuralPiezoNodeWorkpoint
{
    uint physicalNodeId = 0;
    std::array<double,3> q0 = {{0.0,0.0,0.0}}; //!< Absolute displacement [m].
    std::array<double,3> f0 = {{0.0,0.0,0.0}}; //!< Applied force [N].
};

struct RStructuralPiezoPortWorkpoint
{
    QString portId;
    double V0 = 0.0; //!< Positive-minus-negative electrode potential [V].
    double Q0 = 0.0; //!< Positive electrode free charge [C].
};

//! Explicit source data, never an accepted-result or equilibrium certificate.
//! When available, every physical model node and active electrical port is listed.
struct RStructuralPiezoWorkpoint
{
    bool available = false;
    std::vector<RStructuralPiezoNodeWorkpoint> nodes;
    std::vector<RStructuralPiezoPortWorkpoint> ports;
};

//! Independent readout catalog referencing native truss activeMembers by portId.
//! An empty catalog preserves legacy electrical semantics. A nonempty catalog
//! classifies every active port. Open sensors hold the existing initialValue as
//! fixed total charge; short/amplifier sensors have fixed zero voltage. None is
//! an actuator action record, not an independent deformation measurement.
//! ActuatorCharge retains the Ce*V direct term in Q = Ce*V-G^T*q.
struct RStructuralPiezoReadoutSetup
{
    std::vector<RStructuralPiezoReadoutPort> ports;
    RStructuralPiezoWorkpoint workpoint;

    void validate() const;
    //! Shape, finite values, identities and electrical-role compatibility only.
    //! Does not call truss.validate(), compute a fingerprint or read results.
    void validate(const RModel &model) const;
    bool hasNonDefaultData() const;
    //! Canonical little-endian binary64 identity; empty for the legacy default.
    //! IEEE signed zero is preserved. Catalog order is part of source identity.
    QByteArray identityBytes() const;
};

#endif // RML_STRUCTURAL_PIEZO_READOUT_SETUP_H
