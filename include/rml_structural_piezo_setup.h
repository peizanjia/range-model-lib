#ifndef RML_STRUCTURAL_PIEZO_SETUP_H
#define RML_STRUCTURAL_PIEZO_SETUP_H

#include <QString>

#include <vector>

class RModel;

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

//! Model-bound input for a static, terminal 1-D piezoelectric configuration.
//!
//! The mesh fingerprint uses the restart fingerprint's existing v1 encoding:
//! element order and connectivity are exact, while coordinates are normalized
//! to five significant digits. It is an anti-misattachment guard, not an exact
//! geometric identity check. This configuration alone does not enable a
//! solver capability or claim that a physical analysis has been performed.
struct RStructuralPiezoSetup
{
    bool enabled = false;
    QString meshFingerprint;
    std::vector<RStructuralPiezoPort> ports;

    //! Validate port values and any stored fingerprint without requiring a model.
    void validate() const;
    //! Validate port topology and the stored fingerprint binding against a model.
    void validate(const RModel &model) const;
    //! Check port topology, then explicitly bind this setup to the model.
    void bindToModel(const RModel &model);
};

#endif // RML_STRUCTURAL_PIEZO_SETUP_H
