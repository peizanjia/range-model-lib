#include <rbl_error.h>

#include "rml_material_evaluation.h"

bool RMaterialPointState::contains(const QString &id) const
{
    return this->variables.contains(id);
}

double RMaterialPointState::get(const QString &id, double defaultValue) const
{
    return this->variables.value(id,defaultValue);
}

void RMaterialPointState::set(const QString &id, double value)
{
    this->variables.insert(id,value);
}

void RMaterialPointState::remove(const QString &id)
{
    this->variables.remove(id);
}

void RMaterialPointState::clear()
{
    this->variables.clear();
}

const QMap<QString,double> &RMaterialPointState::getVariables() const
{
    return this->variables;
}

RMaterialEvaluationContext::RMaterialEvaluationContext()
    : time(0.0),
      timeStep(0.0)
{
}

double RMaterialEvaluationContext::getTime() const
{
    return this->time;
}

void RMaterialEvaluationContext::setTime(double value)
{
    this->time = value;
}

double RMaterialEvaluationContext::getTimeStep() const
{
    return this->timeStep;
}

void RMaterialEvaluationContext::setTimeStep(double value)
{
    this->timeStep = value;
}

bool RMaterialEvaluationContext::hasField(RVariableType type) const
{
    return this->fields.contains(type);
}

double RMaterialEvaluationContext::getField(RVariableType type) const
{
    if (!this->fields.contains(type))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                     "Material evaluation field '%s' is not available.",
                     RVariable::getId(type).toUtf8().constData());
    }
    return this->fields.value(type);
}

void RMaterialEvaluationContext::setField(RVariableType type, double value)
{
    this->fields.insert(type,value);
}

const QVector<double> &RMaterialEvaluationContext::getStrain() const
{
    return this->strain;
}

void RMaterialEvaluationContext::setStrain(const QVector<double> &value)
{
    this->strain = value;
}

const QVector<double> &RMaterialEvaluationContext::getStrainIncrement() const
{
    return this->strainIncrement;
}

void RMaterialEvaluationContext::setStrainIncrement(const QVector<double> &value)
{
    this->strainIncrement = value;
}

const RMaterialPointState &RMaterialEvaluationContext::getCommittedState() const
{
    return this->committedState;
}

void RMaterialEvaluationContext::setCommittedState(const RMaterialPointState &value)
{
    this->committedState = value;
}

RMaterialEvaluationResult::RMaterialEvaluationResult(double value,
                                                     const QString &behaviorId,
                                                     const RMaterialPointState &trialState)
    : value(value),
      behaviorId(behaviorId),
      trialState(trialState)
{
}

double RMaterialEvaluationResult::getValue() const
{
    return this->value;
}

const QString &RMaterialEvaluationResult::getBehaviorId() const
{
    return this->behaviorId;
}

const RMaterialPointState &RMaterialEvaluationResult::getTrialState() const
{
    return this->trialState;
}
