#include <algorithm>
#include <cmath>
#include <map>
#include <set>

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

double distanceSquared(const RNode &a, const RNode &b)
{
    const double dx = a.getX()-b.getX();
    const double dy = a.getY()-b.getY();
    const double dz = a.getZ()-b.getZ();
    return dx*dx+dy*dy+dz*dz;
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

const std::vector<RLinearConstraint> &RStructuralInteractionSetup::getLinearConstraints() const { return linearConstraints; }
std::vector<RLinearConstraint> &RStructuralInteractionSetup::getLinearConstraints() { return linearConstraints; }
const std::vector<RStructuralCoupling> &RStructuralInteractionSetup::getCouplings() const { return couplings; }
std::vector<RStructuralCoupling> &RStructuralInteractionSetup::getCouplings() { return couplings; }
const std::vector<RStructuralSensor> &RStructuralInteractionSetup::getSensors() const { return sensors; }
std::vector<RStructuralSensor> &RStructuralInteractionSetup::getSensors() { return sensors; }
const std::vector<RStructuralActuator> &RStructuralInteractionSetup::getActuators() const { return actuators; }
std::vector<RStructuralActuator> &RStructuralInteractionSetup::getActuators() { return actuators; }

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
            const std::vector<uint> masters = resolveNamedSetNodes(model,coupling.masterSet,coupling.entityType);
            const std::vector<uint> slaves = resolveNamedSetNodes(model,coupling.slaveSet,coupling.entityType);
            const double tolerance2 = coupling.tolerance*coupling.tolerance;
            for (uint slave : slaves)
            {
                uint nearest = model.getNNodes();
                double best = std::numeric_limits<double>::infinity();
                for (uint master : masters)
                {
                    const double d2 = distanceSquared(model.getNode(slave),model.getNode(master));
                    if (d2 < best)
                    {
                        best = d2;
                        nearest = master;
                    }
                }
                if (nearest >= model.getNNodes() || best > tolerance2)
                {
                    continue;
                }
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

    for (const RStructuralCoupling &coupling : couplings)
    {
        if ((coupling.type == RStructuralCouplingType::RigidBody || coupling.type == RStructuralCouplingType::Distributed) &&
            coupling.referenceNode >= model.getNNodes())
            report.add(RStructuralDiagnosticSeverity::Error,"invalid-reference-node",coupling.id,"Coupling reference node does not exist.");
        if (resolveNamedSetNodes(model,coupling.slaveSet,coupling.entityType).empty())
            report.add(RStructuralDiagnosticSeverity::Error,"missing-slave-set",coupling.id,"Coupling slave set is empty or does not exist.");
        if (coupling.type == RStructuralCouplingType::Tie && resolveNamedSetNodes(model,coupling.masterSet,coupling.entityType).empty())
            report.add(RStructuralDiagnosticSeverity::Error,"missing-master-set",coupling.id,"Tie master set is empty or does not exist.");
        if (coupling.type == RStructuralCouplingType::Tie && coupling.tolerance <= 0.0)
            report.add(RStructuralDiagnosticSeverity::Error,"invalid-tolerance",coupling.id,"Tie matching tolerance must be positive.");
        for (RDofType dof : coupling.dofs)
            if (!isMechanicalDof(dof))
                report.add(RStructuralDiagnosticSeverity::Error,"invalid-coupling-dof",coupling.id,"Coupling contains a non-mechanical DOF.");
    }

    const std::vector<RLinearConstraint> equations = buildLinearConstraints(model);
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
        else if (RStructuralInteractionSetup::resolveNamedSetNodes(model,target.setName,target.entityType).empty())
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
