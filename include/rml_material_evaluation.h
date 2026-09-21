#ifndef RML_MATERIAL_EVALUATION_H
#define RML_MATERIAL_EVALUATION_H

#include <QMap>
#include <QString>
#include <QVector>

#include "rml_variable.h"

//! Committed or trial history variables at one material point.
class RMaterialPointState
{
    protected:
        QMap<QString,double> variables;

    public:
        bool contains(const QString &id) const;
        double get(const QString &id, double defaultValue = 0.0) const;
        void set(const QString &id, double value);
        void remove(const QString &id);
        void clear();
        const QMap<QString,double> &getVariables() const;
};

//! Inputs used while evaluating a material property or constitutive behavior.
//! Fields are keyed by the stable RVariableType contract so temperature,
//! electric potential and future coupled fields do not require solver-specific
//! material APIs.
class RMaterialEvaluationContext
{
    protected:
        double time;
        double timeStep;
        QMap<RVariableType,double> fields;
        QVector<double> strain;
        QVector<double> strainIncrement;
        RMaterialPointState committedState;

    public:
        RMaterialEvaluationContext();

        double getTime() const;
        void setTime(double value);
        double getTimeStep() const;
        void setTimeStep(double value);

        bool hasField(RVariableType type) const;
        double getField(RVariableType type) const;
        void setField(RVariableType type, double value);

        const QVector<double> &getStrain() const;
        void setStrain(const QVector<double> &value);
        const QVector<double> &getStrainIncrement() const;
        void setStrainIncrement(const QVector<double> &value);

        const RMaterialPointState &getCommittedState() const;
        void setCommittedState(const RMaterialPointState &value);
};

//! Result of one side-effect-free material evaluation.  A nonlinear behavior
//! writes history only to trialState; the caller decides when a converged step
//! is committed or rolled back.
class RMaterialEvaluationResult
{
    protected:
        double value;
        QString behaviorId;
        RMaterialPointState trialState;

    public:
        RMaterialEvaluationResult(double value = 0.0,
                                  const QString &behaviorId = QString(),
                                  const RMaterialPointState &trialState = RMaterialPointState());

        double getValue() const;
        const QString &getBehaviorId() const;
        const RMaterialPointState &getTrialState() const;
};

#endif /* RML_MATERIAL_EVALUATION_H */
