#include <algorithm>

#include <QStringList>

#include "rml_capability_registry.h"
#include "rml_problem.h"
#include "rml_model.h"
#include "rml_structural_product_profile.h"

namespace
{

RProblemTypeMask getRegisteredProblemTypeMask()
{
    return R_PROBLEM_ACOUSTICS |
           R_PROBLEM_FLUID_PARTICLE |
           R_PROBLEM_ELECTROSTATICS |
           R_PROBLEM_MAGNETOSTATICS |
           R_PROBLEM_FLUID |
           R_PROBLEM_FLUID_HEAT |
           R_PROBLEM_HEAT |
           R_PROBLEM_RADIATIVE_HEAT |
           R_PROBLEM_STRESS |
           R_PROBLEM_STRESS_MODAL |
           R_PROBLEM_POTENTIAL |
           R_PROBLEM_WAVE |
           R_PROBLEM_MESH;
}

}

RProductProfileValidation::RProductProfileValidation(RProblemTypeMask unsupportedProblemTypeMask,
                                                     const std::vector<RElementType> &unsupportedElementTypes,
                                                     bool unsupportedTimeSolver,
                                                     const QString &diagnostic)
    : unsupportedProblemTypeMask(unsupportedProblemTypeMask)
    , unsupportedElementTypes(unsupportedElementTypes)
    , unsupportedTimeSolver(unsupportedTimeSolver)
    , diagnostic(diagnostic)
{
}

bool RProductProfileValidation::isAccepted() const
{
    return this->unsupportedProblemTypeMask == R_PROBLEM_NONE &&
           this->unsupportedElementTypes.empty() &&
           !this->unsupportedTimeSolver;
}

RProblemTypeMask RProductProfileValidation::getUnsupportedProblemTypeMask() const
{
    return this->unsupportedProblemTypeMask;
}

const std::vector<RElementType> &RProductProfileValidation::getUnsupportedElementTypes() const
{
    return this->unsupportedElementTypes;
}

bool RProductProfileValidation::getUnsupportedTimeSolver() const
{
    return this->unsupportedTimeSolver;
}

const QString &RProductProfileValidation::getDiagnostic() const
{
    return this->diagnostic;
}

QString RStructuralProductProfile::getId()
{
    return "structural";
}

QString RStructuralProductProfile::getName()
{
    return "Structural analysis";
}

RProblemTypeMask RStructuralProductProfile::getEnabledProblemTypeMask()
{
    return R_PROBLEM_STRESS | R_PROBLEM_STRESS_MODAL;
}

std::vector<RProblemType> RStructuralProductProfile::getEnabledProblemTypes()
{
    return RProblem::getTypes(RStructuralProductProfile::getEnabledProblemTypeMask());
}

RProblemCapability RStructuralProductProfile::getProblemCapability(RProblemType problemType)
{
    const RCapability *capability = nullptr;
    if (R_PROBLEM_TYPE_IS_VALID(problemType) && problemType != R_PROBLEM_NONE)
    {
        capability = RCapabilityRegistry::getBuiltIn().find(
            RCapabilityDomain::Analysis,RProblem::getId(problemType));
    }
    if (!capability)
    {
        return RProblemCapability(problemType,false,false,false,false,false,
                                  "Not enabled: invalid or unknown problem type.");
    }
    const bool enabled = capability->isExposed(RStructuralProductProfile::getId());
    const QString diagnostic = enabled
        ? QStringLiteral("Enabled by the structural product profile.")
        : QStringLiteral("Not enabled: this product profile supports structural analysis only.");
    return RProblemCapability(problemType,
                              capability->isRegistered(),
                              capability->isImplemented(),
                              capability->isTested(),
                              capability->isValidated(),
                              enabled,
                              diagnostic);
}

RElementCapability RStructuralProductProfile::getElementCapability(RElementType elementType)
{
    const RCapability *capability = nullptr;
    if (R_ELEMENT_TYPE_IS_VALID(elementType) && elementType != R_ELEMENT_NONE)
    {
        capability = RCapabilityRegistry::getBuiltIn().find(
            RCapabilityDomain::Element,RElement::getId(elementType));
    }
    if (!capability)
    {
        return RElementCapability(elementType,false,false,false,false,false,false,
                                  "Not enabled: invalid or unknown element type.");
    }
    const bool enabled = capability->isExposed(RStructuralProductProfile::getId());
    QString diagnostic = capability->getDiagnostic();
    if (diagnostic.isEmpty())
    {
        diagnostic = enabled
            ? QStringLiteral("Enabled by the structural product profile.")
            : QStringLiteral("Not enabled by the structural product profile.");
    }
    return RElementCapability(elementType,
                              capability->isRegistered(),
                              capability->isImplemented(),
                              capability->isTested(),
                              capability->isValidated(),
                              enabled,
                              capability->getMetadata().value(QStringLiteral("auxiliary")).toBool(),
                              diagnostic);
}

bool RStructuralProductProfile::isProblemTypeEnabled(RProblemType problemType)
{
    return RStructuralProductProfile::getProblemCapability(problemType).isEnabled();
}

bool RStructuralProductProfile::isElementTypeEnabled(RElementType elementType)
{
    return RStructuralProductProfile::getElementCapability(elementType).isEnabled();
}

bool RStructuralProductProfile::isBoundaryConditionEnabled(RBoundaryConditionType type)
{
    const RCapability *capability = RCapabilityRegistry::getBuiltIn().find(
        RCapabilityDomain::BoundaryCondition,RBoundaryCondition::getId(type));
    return capability && capability->isExposed(RStructuralProductProfile::getId());
}

bool RStructuralProductProfile::isInitialConditionEnabled(RInitialConditionType type)
{
    const RCapability *capability = RCapabilityRegistry::getBuiltIn().find(
        RCapabilityDomain::InitialCondition,RInitialCondition::getId(type));
    return capability && capability->isExposed(RStructuralProductProfile::getId());
}

bool RStructuralProductProfile::isEnvironmentConditionEnabled(REnvironmentConditionType type)
{
    const RCapability *capability = RCapabilityRegistry::getBuiltIn().find(
        RCapabilityDomain::EnvironmentCondition,REnvironmentCondition::getId(type));
    return capability && capability->isExposed(RStructuralProductProfile::getId());
}

bool RStructuralProductProfile::isMaterialPropertyEnabled(RMaterialProperty::Type type)
{
    const RCapability *capability = RCapabilityRegistry::getBuiltIn().find(
        RCapabilityDomain::Material,RMaterialProperty::getId(type));
    return capability && capability->isExposed(RStructuralProductProfile::getId());
}

bool RStructuralProductProfile::isResultVariableEnabled(RVariableType type)
{
    const RProblemTypeMask resultProblemTypeMask = RVariable::getResultsProblemTypeMask(type);

    // Preserve custom and legacy derived variables until result provenance is
    // represented explicitly in the model format.
    return resultProblemTypeMask == R_PROBLEM_NONE ||
           (resultProblemTypeMask & RStructuralProductProfile::getEnabledProblemTypeMask()) != 0;
}

bool RStructuralProductProfile::isTimeSolverEnabled()
{
    return false;
}

RProductProfileValidation RStructuralProductProfile::validate(RProblemTypeMask problemTypeMask)
{
    const RProblemTypeMask unsupportedProblemTypeMask =
        problemTypeMask & ~RStructuralProductProfile::getEnabledProblemTypeMask();

    if (unsupportedProblemTypeMask == R_PROBLEM_NONE)
    {
        return RProductProfileValidation();
    }

    QStringList unsupportedProblemNames;
    const RProblemTypeMask registeredUnsupportedProblemTypeMask =
        unsupportedProblemTypeMask & getRegisteredProblemTypeMask();
    if (registeredUnsupportedProblemTypeMask != R_PROBLEM_NONE)
    {
        const std::vector<RProblemType> unsupportedProblemTypes =
            RProblem::getTypes(registeredUnsupportedProblemTypeMask);
        for (RProblemType problemType : unsupportedProblemTypes)
        {
            unsupportedProblemNames.append(RProblem::getName(problemType));
        }
    }

    const RProblemTypeMask unknownProblemTypeMask =
        unsupportedProblemTypeMask & ~getRegisteredProblemTypeMask();
    if (unknownProblemTypeMask != R_PROBLEM_NONE)
    {
        unsupportedProblemNames.append(QString("unknown problem mask 0x%1").arg(unknownProblemTypeMask,0,16));
    }

    const QString diagnostic =
        QString("The '%1' product profile supports only Stress analysis and Modal analysis. "
                "Unsupported problem types: %2.")
            .arg(RStructuralProductProfile::getName(),unsupportedProblemNames.join(", "));

    return RProductProfileValidation(unsupportedProblemTypeMask,std::vector<RElementType>(),false,diagnostic);
}

RProductProfileValidation RStructuralProductProfile::validate(const RModel &model)
{
    const RProductProfileValidation problemValidation =
        RStructuralProductProfile::validate(model.getProblemTaskTree().getProblemTypeMask());

    std::vector<RElementType> unsupportedElementTypes;
    for (uint i=0;i<model.getNElements();i++)
    {
        const RElementType elementType = model.getElement(i).getType();
        if (RStructuralProductProfile::isElementTypeEnabled(elementType))
        {
            continue;
        }
        if (std::find(unsupportedElementTypes.begin(),unsupportedElementTypes.end(),elementType) ==
            unsupportedElementTypes.end())
        {
            unsupportedElementTypes.push_back(elementType);
        }
    }

    QStringList diagnostics;
    if (!problemValidation.getDiagnostic().isEmpty())
    {
        diagnostics.append(problemValidation.getDiagnostic());
    }
    if (!unsupportedElementTypes.empty())
    {
        QStringList elementNames;
        for (RElementType elementType : unsupportedElementTypes)
        {
            if (R_ELEMENT_TYPE_IS_VALID(elementType))
            {
                elementNames.append(RElement::getId(elementType) + " (" + RElement::getName(elementType) + ")");
            }
            else
            {
                elementNames.append(QString("unknown element type %1").arg(int(elementType)));
            }
        }
        diagnostics.append("The structural solver has no enabled formulation for element types: " +
                           elementNames.join(", ") + ".");
    }

    // Structural dynamics reuses RTimeSolver for its time grid after the
    // second-order setup selects the actual integration formulation. Reject
    // only the legacy first-order structural march, not that shared time grid.
    const bool unsupportedTimeSolver =
        model.getTimeSolver().getEnabled() &&
        !model.getProblemSetup().getStructuralDynamicsSetup().getEnabled() &&
        !RStructuralProductProfile::isTimeSolverEnabled();
    if (unsupportedTimeSolver)
    {
        diagnostics.append("Transient structural analysis is not enabled because the current time solver is a legacy first-order relaxation, not a verified structural dynamics formulation.");
    }

    return RProductProfileValidation(problemValidation.getUnsupportedProblemTypeMask(),
                                     unsupportedElementTypes,
                                     unsupportedTimeSolver,
                                     diagnostics.join(" "));
}
