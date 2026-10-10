#include <algorithm>
#include <cmath>
#include <map>
#include <limits>
#include <set>

#include <rbl_error.h>

#include "rml_element_group.h"
#include "rml_model.h"
#include "rml_structural_interaction.h"

namespace
{
bool isMechanicalDof(RDofType dof)
{
    return dof == RDofType::UX || dof == RDofType::UY || dof == RDofType::UZ ||
           dof == RDofType::RX || dof == RDofType::RY || dof == RDofType::RZ;
}

int dofComponent(RDofType dof)
{
    switch (dof)
    {
        case RDofType::UX: case RDofType::RX: return 0;
        case RDofType::UY: case RDofType::RY: return 1;
        case RDofType::UZ: case RDofType::RZ: return 2;
        default: return -1;
    }
}

bool isRotation(RDofType dof)
{
    return dof == RDofType::RX || dof == RDofType::RY || dof == RDofType::RZ;
}

void addTerm(RLinearConstraint &equation, uint nodeId, RDofType dof, double coefficient)
{
    if (std::fabs(coefficient) <= 1.0e-14)
    {
        return;
    }
    RLinearConstraintTerm term;
    term.reference.nodeId = nodeId;
    term.reference.dof = dof;
    term.coefficient = coefficient;
    equation.terms.push_back(term);
}

bool validateSetElements(const RModel &model, const QString &setName,
                         REntityGroupType entityType, const QString &objectId,
                         RStructuralValidationReport &report)
{
    bool valid = true;
    for (uint groupId=0;groupId<model.getNElementGroups();groupId++)
    {
        const RElementGroup *group = model.getElementGroupPtr(groupId);
        if (!group || group->getName() != setName ||
            (entityType != R_ENTITY_GROUP_NONE && entityType != model.getEntityGroupType(groupId,true)))
            continue;
        for (uint i=0;i<group->size();i++)
        {
            if (group->get(i) >= model.getNElements())
            {
                report.add(RStructuralDiagnosticSeverity::Error,"invalid-element",objectId,
                           QString("Structural object '%1' set '%2' contains nonexistent element %3.")
                           .arg(objectId).arg(setName).arg(group->get(i)));
                valid = false;
            }
        }
    }
    return valid;
}

void rejectInvalidReport(const RStructuralValidationReport &report)
{
    if (!report.hasErrors()) return;
    QStringList messages;
    for (const auto &diagnostic : report.getDiagnostics()) messages.push_back(diagnostic.message);
    throw RError(RError::Type::InvalidInput,R_ERROR_REF,"%s",qPrintable(messages.join(" ")));
}

struct TieMatches
{
    std::vector<std::pair<uint,uint>> nodes;
    RStructuralValidationReport report;
};

TieMatches matchTieNodes(const RModel &model, const RStructuralCoupling &coupling)
{
    TieMatches result;
    auto error = [&](const QString &code, const QString &message)
    {
        result.report.add(RStructuralDiagnosticSeverity::Error,code,coupling.id,message);
    };
    const bool mastersValid = validateSetElements(model,coupling.masterSet,coupling.entityType,coupling.id,result.report);
    const bool slavesValid = validateSetElements(model,coupling.slaveSet,coupling.entityType,coupling.id,result.report);
    const auto masters = mastersValid
        ? RStructuralInteractionSetup::resolveNamedSetNodes(model,coupling.masterSet,coupling.entityType) : std::vector<uint>();
    const auto slaves = slavesValid
        ? RStructuralInteractionSetup::resolveNamedSetNodes(model,coupling.slaveSet,coupling.entityType) : std::vector<uint>();
    if (slavesValid && slaves.empty()) error("missing-slave-set","Coupling slave set is empty or does not exist.");
    if (mastersValid && masters.empty()) error("missing-master-set","Tie master set is empty or does not exist.");
    if (!std::isfinite(coupling.tolerance) || coupling.tolerance <= 0.0)
    {
        error("invalid-tolerance",QString("Tie '%1' matching tolerance must be finite and positive.").arg(coupling.id));
        return result;
    }
    if (!mastersValid || !slavesValid) return result;
    for (uint master : masters)
    {
        if (master >= model.getNNodes())
            error("invalid-node",QString("Tie '%1' master node %2 does not exist.").arg(coupling.id).arg(master));
    }
    std::vector<uint> missing;
    std::vector<uint> ambiguous;
    for (uint slave : slaves)
    {
        if (slave >= model.getNNodes())
        {
            error("invalid-node",QString("Tie '%1' slave node %2 does not exist.").arg(coupling.id).arg(slave));
            continue;
        }
        uint nearest = model.getNNodes();
        double best = std::numeric_limits<double>::infinity();
        double second = std::numeric_limits<double>::infinity();
        for (uint master : masters)
        {
            if (master >= model.getNNodes()) continue;
            const RNode &a = model.getNode(slave);
            const RNode &b = model.getNode(master);
            // Avoid overflow/underflow from squaring the requested tolerance.
            const double distance = std::hypot(a.getX()-b.getX(),a.getY()-b.getY(),a.getZ()-b.getZ());
            if (!std::isfinite(distance) || distance > coupling.tolerance) continue;
            if (distance < best)
            {
                second = best;
                best = distance;
                nearest = master;
            }
            else if (distance < second) second = distance;
        }
        if (nearest >= model.getNNodes()) missing.push_back(slave);
        // Relative rounding tolerance compares only candidates already inside
        // the user tolerance; it never increases the matching radius.
        else if (std::isfinite(second) && second-best <=
                 32.0*std::numeric_limits<double>::epsilon()*std::max(best,second))
            ambiguous.push_back(slave);
        else result.nodes.push_back({slave,nearest});
    }
    for (uint slave : missing)
        error("unmatched-tie-slave",QString("Tie '%1' slave node %2 has no master within tolerance; missing slave count: %3.")
              .arg(coupling.id).arg(slave).arg(qulonglong(missing.size())));
    for (uint slave : ambiguous)
        error("ambiguous-tie-match",QString("Tie '%1' slave node %2 has equally near masters within numerical precision; ambiguous slave count: %3; missing slave count: %4.")
              .arg(coupling.id).arg(slave).arg(qulonglong(ambiguous.size())).arg(qulonglong(missing.size())));
    return result;
}

int matrixRank(std::vector<std::vector<double> > matrix, double tolerance = 1.0e-10)
{
    if (matrix.empty())
    {
        return 0;
    }
    const size_t nColumns = matrix[0].size();
    size_t row = 0;
    for (size_t column=0;column<nColumns && row<matrix.size();column++)
    {
        size_t pivot = row;
        for (size_t i=row+1;i<matrix.size();i++)
        {
            if (std::fabs(matrix[i][column]) > std::fabs(matrix[pivot][column]))
            {
                pivot = i;
            }
        }
        if (std::fabs(matrix[pivot][column]) <= tolerance)
        {
            continue;
        }
        std::swap(matrix[pivot],matrix[row]);
        const double divisor = matrix[row][column];
        for (size_t j=column;j<nColumns;j++)
        {
            matrix[row][j] /= divisor;
        }
        for (size_t i=0;i<matrix.size();i++)
        {
            if (i == row)
            {
                continue;
            }
            const double factor = matrix[i][column];
            for (size_t j=column;j<nColumns;j++)
            {
                matrix[i][j] -= factor*matrix[row][j];
            }
        }
        row++;
    }
    return int(row);
}

bool actuatorVariableMatches(const RStructuralActuator &actuator)
{
    switch (actuator.type)
    {
        case RStructuralActuatorType::NodalForce:
            return actuator.variable == R_VARIABLE_FORCE ||
                   actuator.variable == R_VARIABLE_FORCE_X || actuator.variable == R_VARIABLE_FORCE_Y || actuator.variable == R_VARIABLE_FORCE_Z;
        case RStructuralActuatorType::SurfaceForce:
            return actuator.variable == R_VARIABLE_FORCE || actuator.variable == R_VARIABLE_FORCE_UNIT_AREA ||
                   actuator.variable == R_VARIABLE_FORCE_UNIT_AREA_X || actuator.variable == R_VARIABLE_FORCE_UNIT_AREA_Y || actuator.variable == R_VARIABLE_FORCE_UNIT_AREA_Z;
        case RStructuralActuatorType::BaseMotion:
            return actuator.variable == R_VARIABLE_DISPLACEMENT || actuator.variable == R_VARIABLE_VELOCITY || actuator.variable == R_VARIABLE_ACCELERATION;
        case RStructuralActuatorType::HeatSource:
            return actuator.variable == R_VARIABLE_HEAT || actuator.variable == R_VARIABLE_HEAT_RATE_AREA || actuator.variable == R_VARIABLE_HEAT_RATE_VOLUME;
        case RStructuralActuatorType::CustomInput:
            return actuator.variable != R_VARIABLE_NONE;
    }
    return false;
}

}

void RStructuralValidationReport::add(RStructuralDiagnosticSeverity severity,
                                      const QString &code,
                                      const QString &objectId,
                                      const QString &message)
{
    diagnostics.push_back({severity,code,objectId,message});
}

bool RStructuralValidationReport::hasErrors() const
{
    for (const RStructuralDiagnostic &diagnostic : diagnostics)
    {
        if (diagnostic.severity == RStructuralDiagnosticSeverity::Error)
        {
            return true;
        }
    }
    return false;
}

const std::vector<RStructuralDiagnostic> &RStructuralValidationReport::getDiagnostics() const
{
    return diagnostics;
}

void RStructuralPrescribedMotion::validate() const
{
    if (id.trimmed().isEmpty() || id.contains(QLatin1Char('"')) ||
        id.contains(QLatin1Char('\n')) || id.contains(QLatin1Char('\r')))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                     "Prescribed motion requires a nonempty stable ID without quotes or line breaks.");
    }
    if (!isMechanicalDof(target.dof))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                     "Prescribed motion '%s' targets a non-mechanical DOF.",qPrintable(id));
    }
    if (knots.size() < 2)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                     "Prescribed motion '%s' requires at least two time knots.",qPrintable(id));
    }
    double previousTime = 0.0;
    for (std::size_t i=0;i<knots.size();i++)
    {
        const RStructuralMotionKnot &knot = knots[i];
        const double h = i > 0 ? knot.time-previousTime : 1.0;
        if (!std::isfinite(knot.time) || !std::isfinite(knot.displacement) ||
            !std::isfinite(knot.velocity) || !std::isfinite(knot.acceleration) ||
            (i > 0 && (!std::isfinite(h) || h <= 0.0 ||
                       !std::isfinite(h*h) || !std::isfinite(1.0/(h*h)))))
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                         "Prescribed motion '%s' has non-finite values or non-increasing times.",qPrintable(id));
        }
        previousTime = knot.time;
    }
}

RStructuralMotionState RStructuralPrescribedMotion::evaluate(double time) const
{
    validate();
    const double firstTime = knots.front().time;
    const double lastTime = knots.back().time;
    if (!std::isfinite(time))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                     "Prescribed motion '%s' is evaluated outside its time domain.",qPrintable(id));
    }
    // Accept only floating-point rounding at the endpoints, not extrapolation.
    const double startTolerance = 16.0*std::numeric_limits<double>::epsilon()*
        std::max({1.0,std::fabs(time),std::fabs(firstTime)});
    const double endTolerance = 16.0*std::numeric_limits<double>::epsilon()*
        std::max({1.0,std::fabs(time),std::fabs(lastTime)});
    if (time < firstTime-startTolerance || time > lastTime+endTolerance)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                     "Prescribed motion '%s' is evaluated outside its time domain.",qPrintable(id));
    }
    time = std::max(firstTime,std::min(time,lastTime));
    if (time == lastTime)
    {
        const auto &last = knots.back();
        return {last.displacement,last.velocity,last.acceleration};
    }
    const auto upper = std::upper_bound(knots.begin(),knots.end(),time,
        [](double value, const RStructuralMotionKnot &knot) { return value < knot.time; });
    const auto &left = *(upper-1);
    const auto &right = *upper;
    const double h = right.time-left.time;
    const double s = (time-left.time)/h;
    const double c0 = left.displacement;
    const double c1 = h*left.velocity;
    const double c2 = 0.5*h*h*left.acceleration;
    const double dg = right.displacement-c0-c1-c2;
    const double dv = h*right.velocity-c1-2.0*c2;
    const double da = h*h*right.acceleration-2.0*c2;
    const double c3 = 10.0*dg-4.0*dv+0.5*da;
    const double c4 = -15.0*dg+7.0*dv-da;
    const double c5 = 6.0*dg-3.0*dv+0.5*da;
    const RStructuralMotionState state{
        c0+s*(c1+s*(c2+s*(c3+s*(c4+s*c5)))),
        (c1+s*(2.0*c2+s*(3.0*c3+s*(4.0*c4+s*5.0*c5))))/h,
        (2.0*c2+s*(6.0*c3+s*(12.0*c4+s*20.0*c5)))/(h*h)};
    if (!std::isfinite(state.displacement) || !std::isfinite(state.velocity) ||
        !std::isfinite(state.acceleration))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                     "Prescribed motion '%s' overflows during evaluation.",qPrintable(id));
    }
    return state;
}

const std::vector<RLinearConstraint> &RStructuralInteractionSetup::getLinearConstraints() const { return linearConstraints; }
std::vector<RLinearConstraint> &RStructuralInteractionSetup::getLinearConstraints() { return linearConstraints; }
const std::vector<RStructuralCoupling> &RStructuralInteractionSetup::getCouplings() const { return couplings; }
std::vector<RStructuralCoupling> &RStructuralInteractionSetup::getCouplings() { return couplings; }
const std::vector<RStructuralSensor> &RStructuralInteractionSetup::getSensors() const { return sensors; }
std::vector<RStructuralSensor> &RStructuralInteractionSetup::getSensors() { return sensors; }
const std::vector<RStructuralActuator> &RStructuralInteractionSetup::getActuators() const { return actuators; }
std::vector<RStructuralActuator> &RStructuralInteractionSetup::getActuators() { return actuators; }
const std::vector<RStructuralPrescribedMotion> &RStructuralInteractionSetup::getPrescribedMotions() const { return prescribedMotions; }
std::vector<RStructuralPrescribedMotion> &RStructuralInteractionSetup::getPrescribedMotions() { return prescribedMotions; }

std::vector<uint> RStructuralInteractionSetup::resolveNamedSetNodes(const RModel &model,
                                                                    const QString &setName,
                                                                    REntityGroupType entityType)
{
    std::set<uint> nodeIds;
    for (uint groupId=0;groupId<model.getNElementGroups();groupId++)
    {
        const RElementGroup *group = model.getElementGroupPtr(groupId);
        const REntityGroupType groupType = model.getEntityGroupType(groupId,true);
        if (!group || group->getName() != setName ||
            (entityType != R_ENTITY_GROUP_NONE && entityType != groupType))
        {
            continue;
        }
        for (uint i=0;i<group->size();i++)
        {
            if (group->get(i) >= model.getNElements())
                throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                             "Named set '%s' contains nonexistent element %u.",qPrintable(setName),group->get(i));
            const RElement &element = model.getElement(group->get(i));
            for (uint j=0;j<element.size();j++)
            {
                nodeIds.insert(element.getNodeId(j));
            }
        }
    }
    return std::vector<uint>(nodeIds.begin(),nodeIds.end());
}

QString RStructuralInteractionSetup::getSensorUnit(RStructuralSensorQuantity quantity)
{
    switch (quantity)
    {
        case RStructuralSensorQuantity::Displacement: return "m";
        case RStructuralSensorQuantity::Velocity: return "m/s";
        case RStructuralSensorQuantity::Acceleration: return "m/s^2";
        case RStructuralSensorQuantity::Strain: return "1";
        case RStructuralSensorQuantity::Stress: return "Pa";
        case RStructuralSensorQuantity::Temperature: return "K";
        case RStructuralSensorQuantity::Charge: return "C";
    }
    return QString();
}

std::vector<RLinearConstraint> RStructuralInteractionSetup::buildLinearConstraints(const RModel &model) const
{
    std::vector<RLinearConstraint> equations = linearConstraints;
    for (const RStructuralCoupling &coupling : couplings)
    {
        if (coupling.type != RStructuralCouplingType::Tie)
        {
            RStructuralValidationReport report;
            validateSetElements(model,coupling.slaveSet,coupling.entityType,coupling.id,report);
            rejectInvalidReport(report);
        }
        const std::vector<RDofType> dofs = coupling.dofs.empty()
            ? std::vector<RDofType>{RDofType::UX,RDofType::UY,RDofType::UZ}
            : coupling.dofs;
        if (coupling.type == RStructuralCouplingType::RigidBody)
        {
            const std::vector<uint> targetNodes = resolveNamedSetNodes(model,coupling.slaveSet,coupling.entityType);
            if (coupling.referenceNode >= model.getNNodes())
            {
                continue;
            }
            const RNode &reference = model.getNode(coupling.referenceNode);
            for (uint nodeId : targetNodes)
            {
                if (nodeId == coupling.referenceNode)
                {
                    continue;
                }
                const double rx = model.getNode(nodeId).getX()-reference.getX();
                const double ry = model.getNode(nodeId).getY()-reference.getY();
                const double rz = model.getNode(nodeId).getZ()-reference.getZ();
                for (RDofType dof : dofs)
                {
                    if (isRotation(dof))
                    {
                        RLinearConstraint equation;
                        equation.id = coupling.id + QString(":r%1:%2").arg(nodeId).arg(dofComponent(dof));
                        equation.name = coupling.name;
                        addTerm(equation,nodeId,dof,1.0);
                        addTerm(equation,coupling.referenceNode,dof,-1.0);
                        equations.push_back(equation);
                        continue;
                    }
                    const int c = dofComponent(dof);
                    if (c < 0)
                    {
                        continue;
                    }
                    RLinearConstraint equation;
                    equation.id = coupling.id + QString(":u%1:%2").arg(nodeId).arg(c);
                    equation.name = coupling.name;
                    addTerm(equation,nodeId,dof,1.0);
                    addTerm(equation,coupling.referenceNode,dof,-1.0);
                    // u = u_ref + theta_ref x r
                    if (c == 0) { addTerm(equation,coupling.referenceNode,RDofType::RY,-rz); addTerm(equation,coupling.referenceNode,RDofType::RZ, ry); }
                    if (c == 1) { addTerm(equation,coupling.referenceNode,RDofType::RX, rz); addTerm(equation,coupling.referenceNode,RDofType::RZ,-rx); }
                    if (c == 2) { addTerm(equation,coupling.referenceNode,RDofType::RX,-ry); addTerm(equation,coupling.referenceNode,RDofType::RY, rx); }
                    equations.push_back(equation);
                }
            }
        }
        else if (coupling.type == RStructuralCouplingType::Distributed)
        {
            const std::vector<uint> targetNodes = resolveNamedSetNodes(model,coupling.slaveSet,coupling.entityType);
            if (targetNodes.empty())
            {
                continue;
            }
            const double weight = 1.0/double(targetNodes.size());
            for (RDofType dof : dofs)
            {
                RLinearConstraint equation;
                equation.id = coupling.id + QString(":avg:%1").arg(dofComponent(dof));
                equation.name = coupling.name;
                addTerm(equation,coupling.referenceNode,dof,1.0);
                for (uint nodeId : targetNodes)
                {
                    addTerm(equation,nodeId,dof,-weight);
                }
                equations.push_back(equation);
            }
        }
        else
        {
            const TieMatches matches = matchTieNodes(model,coupling);
            rejectInvalidReport(matches.report);
            for (const auto &match : matches.nodes)
            {
                const uint slave = match.first;
                const uint nearest = match.second;
                for (RDofType dof : dofs)
                {
                    RLinearConstraint equation;
                    equation.id = coupling.id + QString(":tie:%1:%2").arg(slave).arg(dofComponent(dof));
                    equation.name = coupling.name;
                    addTerm(equation,slave,dof,1.0);
                    addTerm(equation,nearest,dof,-1.0);
                    equations.push_back(equation);
                }
            }
        }
    }
    return equations;
}

RStructuralValidationReport RStructuralInteractionSetup::validate(const RModel &model) const
{
    RStructuralValidationReport report;
    std::set<QString> ids;
    auto checkId = [&report,&ids](const QString &id, const QString &kind)
    {
        if (id.trimmed().isEmpty())
        {
            report.add(RStructuralDiagnosticSeverity::Error,"missing-id",id,kind + " requires a stable ID.");
        }
        else if (!ids.insert(id).second)
        {
            report.add(RStructuralDiagnosticSeverity::Error,"duplicate-id",id,"Stable IDs must be unique across the structural interaction catalog.");
        }
    };

    for (const RLinearConstraint &constraint : linearConstraints) checkId(constraint.id,"Linear constraint");
    for (const RStructuralCoupling &coupling : couplings) checkId(coupling.id,"Coupling");
    for (const RStructuralSensor &sensor : sensors) checkId(sensor.id,"Sensor");
    for (const RStructuralActuator &actuator : actuators) checkId(actuator.id,"Actuator");
    for (const RStructuralPrescribedMotion &motion : prescribedMotions) checkId(motion.id,"Prescribed motion");

    std::set<std::pair<uint,int>> motionTargets;
    for (const RStructuralPrescribedMotion &motion : prescribedMotions)
    {
        try
        {
            motion.validate();
        }
        catch (const RError &error)
        {
            report.add(RStructuralDiagnosticSeverity::Error,"invalid-motion",motion.id,error.getMessage());
        }
        if (motion.target.nodeId >= model.getNNodes())
        {
            report.add(RStructuralDiagnosticSeverity::Error,"invalid-motion-node",motion.id,
                       "Prescribed motion target node does not exist.");
        }
        if (!motionTargets.insert({motion.target.nodeId,int(motion.target.dof)}).second)
        {
            report.add(RStructuralDiagnosticSeverity::Error,"duplicate-motion-target",motion.id,
                       "Only one prescribed motion may own a nodal DOF.");
        }
    }

    RStructuralInteractionSetup buildable = *this;
    buildable.couplings.clear();
    for (const RStructuralCoupling &coupling : couplings)
    {
        if ((coupling.type == RStructuralCouplingType::RigidBody || coupling.type == RStructuralCouplingType::Distributed) &&
            coupling.referenceNode >= model.getNNodes())
            report.add(RStructuralDiagnosticSeverity::Error,"invalid-reference-node",coupling.id,"Coupling reference node does not exist.");
        if (coupling.type == RStructuralCouplingType::Tie)
        {
            const TieMatches matches = matchTieNodes(model,coupling);
            for (const auto &diagnostic : matches.report.getDiagnostics())
                report.add(diagnostic.severity,diagnostic.code,diagnostic.objectId,diagnostic.message);
            if (!matches.report.hasErrors()) buildable.couplings.push_back(coupling);
        }
        else if (validateSetElements(model,coupling.slaveSet,coupling.entityType,coupling.id,report))
        {
            if (resolveNamedSetNodes(model,coupling.slaveSet,coupling.entityType).empty())
                report.add(RStructuralDiagnosticSeverity::Error,"missing-slave-set",coupling.id,"Coupling slave set is empty or does not exist.");
            buildable.couplings.push_back(coupling);
        }
        for (RDofType dof : coupling.dofs)
            if (!isMechanicalDof(dof))
                report.add(RStructuralDiagnosticSeverity::Error,"invalid-coupling-dof",coupling.id,"Coupling contains a non-mechanical DOF.");
    }

    // Invalid ties and corrupt sets have already supplied typed diagnostics.
    // Exclude those couplings so all remaining validation checks still run.
    const std::vector<RLinearConstraint> equations = buildable.buildLinearConstraints(model);
    std::map<std::pair<uint,int>,double> prescribed;
    std::vector<std::vector<double> > rigidRows;
    for (const RLinearConstraint &constraint : equations)
    {
        if (constraint.terms.empty())
        {
            report.add(RStructuralDiagnosticSeverity::Error,"empty-equation",constraint.id,"Linear constraint has no terms.");
            continue;
        }
        std::map<std::pair<uint,int>,double> combined;
        for (const RLinearConstraintTerm &term : constraint.terms)
        {
            if (term.reference.nodeId >= model.getNNodes())
            {
                report.add(RStructuralDiagnosticSeverity::Error,"invalid-node",constraint.id,
                           QString("Node %1 does not exist.").arg(term.reference.nodeId));
                continue;
            }
            if (!isMechanicalDof(term.reference.dof) || !std::isfinite(term.coefficient))
            {
                report.add(RStructuralDiagnosticSeverity::Error,"invalid-term",constraint.id,"Equation terms require finite coefficients and mechanical DOFs.");
                continue;
            }
            combined[{term.reference.nodeId,int(term.reference.dof)}] += term.coefficient;
        }
        for (auto it=combined.begin();it!=combined.end();)
        {
            if (std::fabs(it->second) <= 1.0e-12) it = combined.erase(it); else ++it;
        }
        if (combined.empty())
        {
            report.add(RStructuralDiagnosticSeverity::Error,"zero-equation",constraint.id,"All equation coefficients cancel.");
            continue;
        }
        if (combined.size() == 1)
        {
            const auto &term = *combined.begin();
            const double value = constraint.rhs/term.second;
            auto found = prescribed.find(term.first);
            if (found != prescribed.end() && std::fabs(found->second-value) > 1.0e-9*std::max({1.0,std::fabs(found->second),std::fabs(value)}))
            {
                report.add(RStructuralDiagnosticSeverity::Error,"contradictory-equations",constraint.id,"The same DOF is prescribed to incompatible values.");
            }
            prescribed[term.first] = value;
        }

        std::vector<double> rigid(6,0.0);
        for (const auto &entry : combined)
        {
            const uint nodeId = entry.first.first;
            const RDofType dof = RDofType(entry.first.second);
            const int c = dofComponent(dof);
            const double a = entry.second;
            if (isRotation(dof))
            {
                rigid[3+c] += a;
            }
            else
            {
                rigid[c] += a;
                const RNode &node = model.getNode(nodeId);
                if (c == 0) { rigid[4] += a*node.getZ(); rigid[5] -= a*node.getY(); }
                if (c == 1) { rigid[3] -= a*node.getZ(); rigid[5] += a*node.getX(); }
                if (c == 2) { rigid[3] += a*node.getY(); rigid[4] -= a*node.getX(); }
            }
        }
        rigidRows.push_back(rigid);
    }

    auto validateTarget = [&model,&report](const RStructuralTarget &target, const QString &id)
    {
        if (target.scope == RStructuralTargetScope::Node)
        {
            if (target.nodeId >= model.getNNodes())
                report.add(RStructuralDiagnosticSeverity::Error,"invalid-target",id,"Target node does not exist.");
        }
        else if (validateSetElements(model,target.setName,target.entityType,id,report) &&
                 RStructuralInteractionSetup::resolveNamedSetNodes(model,target.setName,target.entityType).empty())
        {
            report.add(RStructuralDiagnosticSeverity::Error,"missing-set",id,"Named target set is empty or does not exist.");
        }
    };
    for (const RStructuralSensor &sensor : sensors)
    {
        validateTarget(sensor.target,sensor.id);
        if (sensor.unit.isEmpty() || sensor.unit != getSensorUnit(sensor.quantity))
            report.add(RStructuralDiagnosticSeverity::Error,"unit-mismatch",sensor.id,"Sensor unit must match the selected structural quantity metadata.");
        if (sensor.frame == RStructuralFrame::Custom && sensor.direction.length() <= 1.0e-12)
            report.add(RStructuralDiagnosticSeverity::Error,"missing-direction",sensor.id,"Custom-frame sensor requires a non-zero direction.");
    }
    for (const RStructuralActuator &actuator : actuators)
    {
        validateTarget(actuator.target,actuator.id);
        if (actuator.unit.isEmpty() || actuator.unit != RVariable::getUnits(actuator.variable))
            report.add(RStructuralDiagnosticSeverity::Error,"unit-mismatch",actuator.id,"Actuator unit must match the selected command variable metadata.");
        if (!actuatorVariableMatches(actuator))
            report.add(RStructuralDiagnosticSeverity::Error,"actuator-variable-mismatch",actuator.id,"Actuator type and command variable are incompatible.");
        if (actuator.minimum > actuator.maximum || actuator.rateLimit < 0.0)
            report.add(RStructuralDiagnosticSeverity::Error,"invalid-limit",actuator.id,"Actuator limits are inconsistent.");
        if ((actuator.type == RStructuralActuatorType::NodalForce || actuator.type == RStructuralActuatorType::SurfaceForce || actuator.type == RStructuralActuatorType::BaseMotion) &&
            actuator.frame == RStructuralFrame::Custom && actuator.direction.length() <= 1.0e-12)
            report.add(RStructuralDiagnosticSeverity::Error,"missing-direction",actuator.id,"Custom-frame actuator requires a non-zero direction.");
    }

    if (model.getNNodes() > 0 && !equations.empty())
    {
        const int rank = matrixRank(rigidRows);
        if (rank < 6)
        {
            report.add(RStructuralDiagnosticSeverity::Warning,"rigid-mode-rank",QString(),
                       QString("Linear equations restrain only %1 of 6 global rigid-body modes; legacy boundary conditions are evaluated separately by the solver.").arg(rank));
        }
    }
    return report;
}
