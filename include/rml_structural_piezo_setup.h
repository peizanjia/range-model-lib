#ifndef RML_STRUCTURAL_PIEZO_SETUP_H
#define RML_STRUCTURAL_PIEZO_SETUP_H

#include <QString>

#include <vector>

class RModel;

//! Electrical formulation stored by a structural piezo configuration.
enum class RStructuralPiezoElectricalMode : int
{
    //! Version 1.3.14 terminal-to-terminal constant voltage formulation.
    LegacyTerminalVoltage = 0,
    //! Nodal electric potential formulation with nodal conductor charge data.
    NodalPotential = 1
};

//! Type of an electrical nodal boundary condition.
enum class RStructuralPiezoBoundaryType : int
{
    Potential = 0,
    ConductorCharge = 1
};

//! Prescribed electrical potential (V) or conductor charge (C) at one node.
struct RStructuralPiezoNodeBoundary
{
    uint nodeId = 0;
    RStructuralPiezoBoundaryType type = RStructuralPiezoBoundaryType::Potential;
    double value = 0.0;
};

//! A constant-voltage terminal port associated with one native TRUSS1 element.
struct RStructuralPiezoPort
{
    QString id;
    uint elementId = 0;
    //! Signed piezoelectric coupling e in C/m^2.
    double coupling = 0.0;
    //! Absolute constant-strain permittivity epsS in F/m.
    double permittivity = 0.0;
    //! Prescribed voltage difference phi(node 1) - phi(node 0), in V.
    double voltage = 0.0;
};

//! Model-bound input for a static 1-D piezoelectric configuration.
//!
//! LegacyTerminalVoltage retains the restart fingerprint's five-significant-
//! digit coordinate binding. NodalPotential uses an exact geometry and native
//! connectivity fingerprint that also binds port direction and coefficients.
struct RStructuralPiezoSetup
{
    bool enabled = false;
    QString meshFingerprint;
    std::vector<RStructuralPiezoPort> ports;
    //! Explicit mechanical-only TRUSS1 elements; LegacyTerminalVoltage only.
    //! A nonempty list requires complete, disjoint active/passive classification.
    std::vector<uint> passiveElementIds;
    //! Defaults to the v1.3.14-compatible terminal voltage formulation.
    RStructuralPiezoElectricalMode electricalMode = RStructuralPiezoElectricalMode::LegacyTerminalVoltage;
    //! Used only by NodalPotential; each node may have at most one condition.
    std::vector<RStructuralPiezoNodeBoundary> nodalBoundaryConditions;

    //! Validate port values and any stored fingerprint without requiring a model.
    void validate() const;
    //! Validate port topology and the stored fingerprint binding against a model.
    void validate(const RModel &model) const;
    //! Check port topology, then explicitly bind this setup to the model.
    void bindToModel(const RModel &model);
};

#endif // RML_STRUCTURAL_PIEZO_SETUP_H
