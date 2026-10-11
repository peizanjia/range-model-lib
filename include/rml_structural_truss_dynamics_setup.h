#ifndef RML_STRUCTURAL_TRUSS_DYNAMICS_SETUP_H
#define RML_STRUCTURAL_TRUSS_DYNAMICS_SETUP_H

#include <QString>
#include <array>
#include <vector>

class RModel;

enum class RStructuralTrussElectricalKind { Voltage = 0, ConductorCharge = 1 };
enum class RStructuralTrussMassSource { MaterialDensity = 0, FixedMemberMass = 1 };

struct RStructuralTrussPassiveMember
{
    uint physicalElementId = 0;
    double referenceLength = 0.0;
};

struct RStructuralTrussActiveMember
{
    uint physicalElementId = 0;
    QString portId;
    double referenceLength = 0.0;
    double axialStiffness = 0.0;
    double coupling = 0.0;
    double capacitance = 0.0;
    RStructuralTrussElectricalKind kind = RStructuralTrussElectricalKind::Voltage;
    double initialValue = 0.0;
    RStructuralTrussMassSource massSource = RStructuralTrussMassSource::MaterialDensity;
    double fixedMass = 0.0;
};

struct RStructuralTrussCableMember
{
    uint physicalElementId = 0;
    double initialReferenceLength = 0.0;
    double initialTension = 0.0;
    RStructuralTrussMassSource massSource = RStructuralTrussMassSource::MaterialDensity;
    double fixedMass = 0.0;
};

struct RStructuralTrussElectricalCommand
{
    QString portId;
    double value = 0.0;
};

struct RStructuralTrussCableCommand
{
    uint physicalElementId = 0;
    double referenceLength = 0.0;
};

struct RStructuralTrussNodalForceCommand
{
    uint physicalNodeId = 0;
    std::array<double,3> force = {{0.0,0.0,0.0}};
};

//! Commands apply at the start of the one-based FE step. Addressed values
//! replace absolute applied inputs; omitted targets retain their prior value.
struct RStructuralTrussHeldCommand
{
    uint stepIndex = 0;
    std::vector<RStructuralTrussElectricalCommand> electrical;
    std::vector<RStructuralTrussCableCommand> cables;
    std::vector<RStructuralTrussNodalForceCommand> nodalForces;
};

//! Native mixed TRUSS1 configuration. The existing structural dynamics setup
//! owns dt, run length and output frequency. All member/node IDs are stable
//! physical IDs, never positions. Density mass uses the original mesh chord
//! at 293.15 K; controlled rest lengths do not change frozen mass.
//! TimeSolver.inputStartTime is the first FE end time; initial time is that
//! value minus the authoritative structural dynamics dt.
struct RStructuralTrussDynamicsSetup
{
    bool enabled = false;
    QString sourceFingerprint;
    std::vector<RStructuralTrussPassiveMember> passiveMembers;
    std::vector<RStructuralTrussActiveMember> activeMembers;
    std::vector<RStructuralTrussCableMember> cableMembers;
    std::vector<RStructuralTrussHeldCommand> heldCommands;
    uint maxIterations = 50;
    uint maxLineSearchIterations = 24;
    //! Each force tolerance is in [0,1e-8]; at least one must be positive.
    double absoluteForceTolerance = 1.0e-9;
    double relativeForceTolerance = 1.0e-9;
    //! Strictly between zero and one.
    double linearRelativeTolerance = 1.0e-10;

    void validate() const;
    void validate(const RModel &model) const;
    //! Transactional binding: failure leaves this object unchanged.
    void bindToModel(const RModel &model);
    bool hasNonDefaultData() const;
    //! Exact binary64 source identity, excluding result/restart/runtime clocks
    //! and this setup's own fingerprint. Does not serialize RModel recursively.
    QString computeSourceFingerprint(const RModel &model) const;
};

#endif // RML_STRUCTURAL_TRUSS_DYNAMICS_SETUP_H
