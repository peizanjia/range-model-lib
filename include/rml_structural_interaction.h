#ifndef RML_STRUCTURAL_INTERACTION_H
#define RML_STRUCTURAL_INTERACTION_H

#include <QString>
#include <QStringList>

#include <limits>
#include <vector>

#include <rbl_r3vector.h>

#include "rml_dof.h"
#include "rml_entity_group.h"
#include "rml_variable.h"

class RFileIO;
class RModel;

enum class RStructuralFrame
{
    Global = 0,
    Local,
    Normal,
    Custom
};

enum class RStructuralTargetScope
{
    Node = 0,
    NamedSet
};

//! Stable target used by sensors and actuators.
struct RStructuralTarget
{
    RStructuralTargetScope scope = RStructuralTargetScope::Node;
    uint nodeId = 0;
    QString setName;
    REntityGroupType entityType = R_ENTITY_GROUP_NONE;
};

struct RStructuralDofRef
{
    uint nodeId = 0;
    RDofType dof = RDofType::UX;
};

struct RLinearConstraintTerm
{
    RStructuralDofRef reference;
    double coefficient = 0.0;
};

//! A linear equation sum(a_i*q_i) = rhs in global coordinates.
struct RLinearConstraint
{
    QString id;
    QString name;
    std::vector<RLinearConstraintTerm> terms;
    double rhs = 0.0;
};

enum class RStructuralCouplingType
{
    RigidBody = 0,
    Distributed,
    Tie
};

//! High-level coupling which is expanded to linear equations before solving.
struct RStructuralCoupling
{
    QString id;
    QString name;
    RStructuralCouplingType type = RStructuralCouplingType::RigidBody;
    uint referenceNode = 0;
    QString masterSet;
    QString slaveSet;
    REntityGroupType entityType = R_ENTITY_GROUP_NONE;
    std::vector<RDofType> dofs;
    double tolerance = 1.0e-8;
};

enum class RStructuralSensorAggregation
{
    Point = 0,
    Average,
    Minimum,
    Maximum,
    Sum
};

enum class RStructuralSensorQuantity
{
    Displacement = 0,
    Velocity,
    Acceleration,
    Strain,
    Stress,
    Temperature,
    Charge
};

struct RStructuralSensor
{
    QString id;
    QString name;
    RStructuralTarget target;
    RStructuralSensorQuantity quantity = RStructuralSensorQuantity::Displacement;
    RStructuralSensorAggregation aggregation = RStructuralSensorAggregation::Point;
    RStructuralFrame frame = RStructuralFrame::Global;
    RR3Vector direction = RR3Vector(0.0,0.0,0.0);
    QString unit;
};

enum class RStructuralActuatorType
{
    NodalForce = 0,
    SurfaceForce,
    BaseMotion,
    HeatSource,
    CustomInput
};

struct RStructuralActuator
{
    QString id;
    QString name;
    RStructuralActuatorType type = RStructuralActuatorType::NodalForce;
    RStructuralTarget target;
    RVariableType variable = R_VARIABLE_NONE;
    RStructuralFrame frame = RStructuralFrame::Global;
    RR3Vector direction = RR3Vector(0.0,0.0,0.0);
    QString unit;
    double minimum = -std::numeric_limits<double>::infinity();
    double maximum = std::numeric_limits<double>::infinity();
    double rateLimit = std::numeric_limits<double>::infinity();
};

enum class RStructuralDiagnosticSeverity
{
    Information = 0,
    Warning,
    Error
};

struct RStructuralDiagnostic
{
    RStructuralDiagnosticSeverity severity = RStructuralDiagnosticSeverity::Information;
    QString code;
    QString objectId;
    QString message;
};

class RStructuralValidationReport
{
    private:
        std::vector<RStructuralDiagnostic> diagnostics;

    public:
        void add(RStructuralDiagnosticSeverity severity,
                 const QString &code,
                 const QString &objectId,
                 const QString &message);
        bool hasErrors() const;
        const std::vector<RStructuralDiagnostic> &getDiagnostics() const;
};

//! Stage-4 model contract. Dynamic execution of base motion and control input
//! is intentionally deferred to the second-order dynamics solver.
class RStructuralInteractionSetup
{
    private:
        std::vector<RLinearConstraint> linearConstraints;
        std::vector<RStructuralCoupling> couplings;
        std::vector<RStructuralSensor> sensors;
        std::vector<RStructuralActuator> actuators;

    public:
        const std::vector<RLinearConstraint> &getLinearConstraints() const;
        std::vector<RLinearConstraint> &getLinearConstraints();
        const std::vector<RStructuralCoupling> &getCouplings() const;
        std::vector<RStructuralCoupling> &getCouplings();
        const std::vector<RStructuralSensor> &getSensors() const;
        std::vector<RStructuralSensor> &getSensors();
        const std::vector<RStructuralActuator> &getActuators() const;
        std::vector<RStructuralActuator> &getActuators();

        //! Expand rigid, distributed and tie couplings into solver equations.
        std::vector<RLinearConstraint> buildLinearConstraints(const RModel &model) const;

        //! Validate identifiers, references, units, directions, contradictions
        //! and the rank of absolute restraints against the six rigid modes.
        RStructuralValidationReport validate(const RModel &model) const;

        static std::vector<uint> resolveNamedSetNodes(const RModel &model,
                                                       const QString &setName,
                                                       REntityGroupType entityType = R_ENTITY_GROUP_NONE);
        static QString getSensorUnit(RStructuralSensorQuantity quantity);

        friend class RFileIO;
};

#endif // RML_STRUCTURAL_INTERACTION_H
