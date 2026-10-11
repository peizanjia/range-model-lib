#ifndef RML_STRUCTURAL_TRUSS_DYNAMICS_RESTART_DATA_H
#define RML_STRUCTURAL_TRUSS_DYNAMICS_RESTART_DATA_H

#include <QString>
#include <QByteArray>
#include <vector>

//! Complete accepted native mixed TRUSS1 state. Mechanical arrays are full
//! physical XYZ coordinates in orderedPhysicalNodeIds order. Input kinds are
//! 0=none, 1=voltage, 2=conductor charge, 3=cable reference length.
//! Adapter-level source/system matching and command replay remain mandatory.
struct RStructuralTrussDynamicsRestartData
{
    bool available = false;
    uint completedSteps = 0;
    uint runTargetSteps = 0;
    double initialTime = 0.0;
    double time = 0.0;
    double fixedTimeStep = 0.0;
    uint nextCommandIndex = 0;
    std::vector<uint> orderedPhysicalNodeIds;
    std::vector<uint> orderedMemberPhysicalIds;
    std::vector<QString> orderedPortIds;
    //! Independent electrode observations, in orderedPortIds order. These
    //! never occupy shared mechanical-node electrical result coordinates.
    //! The adapter must freshly derive and verify these on restore.
    std::vector<double> portVoltages;
    std::vector<double> portCharges;
    std::vector<double> portEnthalpies;
    QString sourceFingerprint;
    QByteArray systemIdentity;
    std::vector<double> q;
    std::vector<double> v;
    std::vector<double> a;
    std::vector<double> reactions;
    std::vector<double> externalForce;
    std::vector<uint> memberInputKinds;
    std::vector<double> memberInputValues;
    double storedEnergy = 0.0;
    double kineticEnergy = 0.0;
    double initialTotalEnergy = 0.0;
    double cumulativeExternalWork = 0.0;
    double cumulativeSupportWork = 0.0;
    double cumulativeElectricalWork = 0.0;
    double cumulativeRestLengthWork = 0.0;

    void clear();
    void validate() const;
    bool hasNonDefaultData() const;
};

#endif // RML_STRUCTURAL_TRUSS_DYNAMICS_RESTART_DATA_H
