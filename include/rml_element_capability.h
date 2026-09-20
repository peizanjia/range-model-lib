#ifndef RML_ELEMENT_CAPABILITY_H
#define RML_ELEMENT_CAPABILITY_H

#include <QString>

#include "rml_element.h"

//! Evidence describing structural support for an element type.
class RElementCapability
{

    private:

        //! Element type represented by this capability.
        RElementType elementType;
        //! The element type is registered in the model format.
        bool registered;
        //! A structural formulation is implemented.
        bool formulationImplemented;
        //! The formulation has an automated regression test.
        bool automatedTested;
        //! The formulation passed the reference-host validation baseline.
        bool referenceHostValidated;
        //! The element can be submitted to the structural solver.
        bool enabled;
        //! The element is an auxiliary carrier rather than a stiffness element.
        bool auxiliary;
        //! Human-readable explanation of the product decision.
        QString diagnostic;

    public:

        //! Constructor.
        RElementCapability(RElementType elementType = R_ELEMENT_NONE,
                           bool registered = false,
                           bool formulationImplemented = false,
                           bool automatedTested = false,
                           bool referenceHostValidated = false,
                           bool enabled = false,
                           bool auxiliary = false,
                           const QString &diagnostic = QString());

        RElementType getElementType() const;
        bool isRegistered() const;
        bool isFormulationImplemented() const;
        bool isAutomatedTested() const;
        bool isReferenceHostValidated() const;
        bool isEnabled() const;
        bool isAuxiliary() const;
        const QString &getDiagnostic() const;

};

#endif // RML_ELEMENT_CAPABILITY_H
