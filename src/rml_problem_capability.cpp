#include "rml_problem_capability.h"

RProblemCapability::RProblemCapability(RProblemType problemType,
                                       bool registered,
                                       bool formulationImplemented,
                                       bool automatedTested,
                                       bool referenceHostValidated,
                                       bool enabled,
                                       const QString &diagnostic)
    : problemType(problemType)
    , registered(registered)
    , formulationImplemented(formulationImplemented)
    , automatedTested(automatedTested)
    , referenceHostValidated(referenceHostValidated)
    , enabled(enabled)
    , diagnostic(diagnostic)
{
}

RProblemType RProblemCapability::getProblemType() const
{
    return this->problemType;
}

bool RProblemCapability::isRegistered() const
{
    return this->registered;
}

bool RProblemCapability::isFormulationImplemented() const
{
    return this->formulationImplemented;
}

bool RProblemCapability::isAutomatedTested() const
{
    return this->automatedTested;
}

bool RProblemCapability::isReferenceHostValidated() const
{
    return this->referenceHostValidated;
}

bool RProblemCapability::isEnabled() const
{
    return this->enabled;
}

const QString &RProblemCapability::getDiagnostic() const
{
    return this->diagnostic;
}
