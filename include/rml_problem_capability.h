#ifndef RML_PROBLEM_CAPABILITY_H
#define RML_PROBLEM_CAPABILITY_H

#include <QString>

#include "rml_problem_type.h"

//! Evidence describing how far an analysis capability has been verified.
class RProblemCapability
{

    private:

        //! Problem type represented by this capability.
        RProblemType problemType;
        //! The problem type is registered in the model format.
        bool registered;
        //! The structural product contains an implemented formulation.
        bool formulationImplemented;
        //! The formulation has an automated regression test.
        bool automatedTested;
        //! The formulation passed the reference-host validation baseline.
        bool referenceHostValidated;
        //! The capability is exposed by the structural product profile.
        bool enabled;
        //! Human-readable explanation of the product decision.
        QString diagnostic;

    public:

        //! Constructor.
        RProblemCapability(RProblemType problemType = R_PROBLEM_NONE,
                           bool registered = false,
                           bool formulationImplemented = false,
                           bool automatedTested = false,
                           bool referenceHostValidated = false,
                           bool enabled = false,
                           const QString &diagnostic = QString());

        //! Return the problem type.
        RProblemType getProblemType() const;

        //! Return whether the type is registered in the model format.
        bool isRegistered() const;

        //! Return whether the formulation is implemented for this product.
        bool isFormulationImplemented() const;

        //! Return whether an automated regression test exists.
        bool isAutomatedTested() const;

        //! Return whether the reference-host validation baseline passed.
        bool isReferenceHostValidated() const;

        //! Return whether the capability is exposed by the product profile.
        bool isEnabled() const;

        //! Return the human-readable product decision.
        const QString &getDiagnostic() const;

};

#endif // RML_PROBLEM_CAPABILITY_H
