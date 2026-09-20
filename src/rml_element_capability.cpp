#include "rml_element_capability.h"

RElementCapability::RElementCapability(RElementType elementType,
                                       bool registered,
                                       bool formulationImplemented,
                                       bool automatedTested,
                                       bool referenceHostValidated,
                                       bool enabled,
                                       bool auxiliary,
                                       const QString &diagnostic)
    : elementType(elementType)
    , registered(registered)
    , formulationImplemented(formulationImplemented)
    , automatedTested(automatedTested)
    , referenceHostValidated(referenceHostValidated)
    , enabled(enabled)
    , auxiliary(auxiliary)
    , diagnostic(diagnostic)
{
}

RElementType RElementCapability::getElementType() const
{
    return this->elementType;
}

bool RElementCapability::isRegistered() const
{
    return this->registered;
}

bool RElementCapability::isFormulationImplemented() const
{
    return this->formulationImplemented;
}

bool RElementCapability::isAutomatedTested() const
{
    return this->automatedTested;
}

bool RElementCapability::isReferenceHostValidated() const
{
    return this->referenceHostValidated;
}

bool RElementCapability::isEnabled() const
{
    return this->enabled;
}

bool RElementCapability::isAuxiliary() const
{
    return this->auxiliary;
}

const QString &RElementCapability::getDiagnostic() const
{
    return this->diagnostic;
}
