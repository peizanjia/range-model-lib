#ifndef RML_STRUCTURAL_CABLE_SETUP_H
#define RML_STRUCTURAL_CABLE_SETUP_H

#include <QString>

#include <vector>

class RModel;

//! Per-element reference data for an explicitly selected structural cable.
struct RStructuralCableMember
{
    uint elementId = 0;
    double referenceLength = 0.0;
    double initialTension = 0.0;
};

//! Model-bound configuration for structural cable form finding.
//!
//! The mesh fingerprint uses the restart fingerprint's existing v1 encoding:
//! element order and connectivity are exact, while coordinates are normalized
//! to five significant digits. It is an anti-misattachment guard, not an exact
//! geometric identity check. This is configuration data only; it does not
//! enable a solver capability or make a physical-analysis claim by itself.
struct RStructuralCableSetup
{
    bool enabled = false;
    QString meshFingerprint;
    std::vector<RStructuralCableMember> members;
    uint maxIterations = 50;
    uint maxLineSearchIterations = 24;
    double absoluteForceTolerance = 1.0e-8;
    double relativeForceTolerance = 1.0e-8;

    //! Validate values that do not depend on a model.
    void validate() const;
    //! Validate member topology and the stored fingerprint binding against a model.
    void validate(const RModel &model) const;
    //! Check member topology, then explicitly bind this setup to the model.
    void bindToModel(const RModel &model);
};

#endif // RML_STRUCTURAL_CABLE_SETUP_H
