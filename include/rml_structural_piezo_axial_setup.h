#ifndef RML_STRUCTURAL_PIEZO_AXIAL_SETUP_H
#define RML_STRUCTURAL_PIEZO_AXIAL_SETUP_H

#include <QByteArray>
#include <QString>

class RModel;

//! Whole SI experiment declaration, independent of the scalar native truss law.
//! Canonical compact JSON uses sorted object keys, Qt canonical UTF-8 strings
//! and shortest binary64 numbers, with explicit -0 also permitted. The family
//! is axialRod. Controller/excitation/design/materials/members/supports/initial/
//! workpoint/damping are source declarations, not numerical certificates.
//! The composer/solver must certify material-to-scalar equivalence before use.
struct RStructuralPiezoAxialSetup
{
    bool enabled = false;
    QString sourceFingerprint;
    QByteArray recipeConfiguration;

    void validate() const;
    void validate(const RModel &model) const;
    bool hasNonDefaultData() const;
    //! Exact raw canonical declaration identity; empty for the legacy default.
    QByteArray identityBytes() const;
    QString computeSourceFingerprint(const RModel &model) const;
    //! Failure leaves the original declaration and its binding unchanged.
    void bindToModel(const RModel &model);
};

//! Sealed opaque accepted physical and controller state. The model checks size
//! and source linkage only; the solver codec must verify and restore the payload.
struct RStructuralPiezoAxialRestart
{
    bool available = false;
    QString sourceFingerprint;
    QByteArray runtimeState;

    void validate() const;
    bool hasNonDefaultData() const;
};

#endif // RML_STRUCTURAL_PIEZO_AXIAL_SETUP_H
