#ifndef RML_STRUCTURAL_DYNAMICS_RESTART_DATA_H
#define RML_STRUCTURAL_DYNAMICS_RESTART_DATA_H

#include <QString>
#include <vector>

#include "rml_structural_interaction.h"

class RModel;

//! Disk-persisted, accepted state of a second-order structural solve.
//!
//! q/v/a/externalForce use the reduced integration coordinates.  The DOF
//! fingerprint describes the ordered physical equation book before MPC
//! elimination, so a restart cannot silently attach to another mesh/order.
struct RStructuralDynamicsRestartData
{
    bool available = false;
    uint completedSteps = 0;
    double time = 0.0;
    double fixedTimeStep = 0.0;
    uint meshNodeCount = 0;
    uint meshElementCount = 0;
    uint physicalDofCount = 0;
    QString meshFingerprint;
    QString dofFingerprint;
    //! Solver-owned fingerprint of scaled M/K and the physical-to-reduced map.
    //! This prevents restoring reduced coordinates against changed operators,
    //! prescribed values or local boundary frames with unchanged DOF counts.
    QString systemFingerprint;
    std::vector<double> q;
    std::vector<double> v;
    std::vector<double> a;
    std::vector<double> externalForce;
    double cumulativeExternalWork = 0.0;
    double cumulativeDampingDissipation = 0.0;
    double initialMechanicalEnergy = 0.0;
    //! Optional accepted support-on-structure telemetry in physical units.
    //! Order and constrained-row operators are bound by supportFingerprint,
    //! independent of the core dynamic systemFingerprint.
    //! Pre-1.3.10 checkpoints have no support ledger and may not resume a
    //! support-response observer without explicitly starting a new run.
    bool supportResponseAvailable = false;
    QString supportFingerprint;
    std::vector<double> supportReactions;
    std::vector<double> supportPowers;
    std::vector<double> cumulativeSupportWork;

    void clear();
    void validate() const;
    bool matches(const RModel &model,
                 const std::vector<RStructuralDofRef> &orderedPhysicalDofs) const;

    //! SHA-256 over explicitly encoded geometry/topology, independent of process hash seeds.
    static QString computeMeshFingerprint(const RModel &model);
    //! SHA-256 over the exact physical DOF equation order.
    static QString computeDofFingerprint(const std::vector<RStructuralDofRef> &orderedPhysicalDofs);
};

#endif // RML_STRUCTURAL_DYNAMICS_RESTART_DATA_H
