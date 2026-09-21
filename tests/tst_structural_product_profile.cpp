#include <QtTest>
#include <QJsonArray>

#include "rml_capability_registry.h"
#include "rml_boundary_condition.h"
#include "rml_environment_condition.h"
#include "rml_problem.h"
#include "rml_structural_product_profile.h"

class TestStructuralProductProfile : public QObject
{
    Q_OBJECT

private slots:

    void exposesOnlyStructuralAnalyses();
    void separatesCapabilityEvidence();
    void reportsElementCapabilityEvidence();
    void acceptsStructuralAndGeometryOnlyModels();
    void rejectsUnsupportedAndMixedModels();
    void keepsTemperatureAsStructuralInput();
    void filtersStructuralProductFeatures();
    void exposesFrameBoundaryContracts();
    void registryCoversProductDomains();
    void registryRoundTripsJson();
    void registryRejectsInvalidJson();
};

void TestStructuralProductProfile::exposesOnlyStructuralAnalyses()
{
    QCOMPARE(RStructuralProductProfile::getId(), QString("structural"));
    QCOMPARE(RStructuralProductProfile::getEnabledProblemTypeMask(),
             R_PROBLEM_STRESS | R_PROBLEM_STRESS_MODAL);

    const std::vector<RProblemType> problemTypes = RStructuralProductProfile::getEnabledProblemTypes();
    QCOMPARE(problemTypes.size(), std::size_t(2));
    QCOMPARE(problemTypes[0], R_PROBLEM_STRESS);
    QCOMPARE(problemTypes[1], R_PROBLEM_STRESS_MODAL);
}

void TestStructuralProductProfile::reportsElementCapabilityEvidence()
{
    const RElementCapability tetra =
        RStructuralProductProfile::getElementCapability(R_ELEMENT_TETRA1);
    QVERIFY(tetra.isRegistered());
    QVERIFY(tetra.isFormulationImplemented());
    QVERIFY(tetra.isAutomatedTested());
    QVERIFY(tetra.isReferenceHostValidated());
    QVERIFY(tetra.isEnabled());
    QVERIFY(!tetra.isAuxiliary());

    for (RElementType type : {R_ELEMENT_TRI1,R_ELEMENT_QUAD1,
                              R_ELEMENT_WEDGE1,R_ELEMENT_HEXA1})
    {
        const RElementCapability solid =
            RStructuralProductProfile::getElementCapability(type);
        QVERIFY(solid.isRegistered());
        QVERIFY(solid.isFormulationImplemented());
        QVERIFY(solid.isAutomatedTested());
        QVERIFY(solid.isReferenceHostValidated());
        QVERIFY(solid.isEnabled());
        QVERIFY(!solid.isAuxiliary());
    }

    const RElementCapability point =
        RStructuralProductProfile::getElementCapability(R_ELEMENT_POINT);
    QVERIFY(point.isEnabled());
    QVERIFY(point.isAuxiliary());

    const RElementCapability beam =
        RStructuralProductProfile::getElementCapability(R_ELEMENT_BEAM1);
    QVERIFY(beam.isRegistered());
    QVERIFY(!beam.isFormulationImplemented());
    QVERIFY(!beam.isAutomatedTested());
    QVERIFY(!beam.isReferenceHostValidated());
    QVERIFY(!beam.isEnabled());
    QVERIFY(beam.getDiagnostic().contains("file compatibility"));
}

void TestStructuralProductProfile::separatesCapabilityEvidence()
{
    const RProblemCapability stress =
        RStructuralProductProfile::getProblemCapability(R_PROBLEM_STRESS);
    QVERIFY(stress.isRegistered());
    QVERIFY(stress.isFormulationImplemented());
    QVERIFY(stress.isAutomatedTested());
    QVERIFY(stress.isReferenceHostValidated());
    QVERIFY(stress.isEnabled());

    const RProblemCapability heat =
        RStructuralProductProfile::getProblemCapability(R_PROBLEM_HEAT);
    QVERIFY(heat.isRegistered());
    QVERIFY(heat.isFormulationImplemented());
    QVERIFY(!heat.isAutomatedTested());
    QVERIFY(!heat.isReferenceHostValidated());
    QVERIFY(!heat.isEnabled());
    QVERIFY(!heat.getDiagnostic().isEmpty());

    QVERIFY(!RStructuralProductProfile::isProblemTypeEnabled(
        RProblemType(R_PROBLEM_STRESS | R_PROBLEM_HEAT)));
}

void TestStructuralProductProfile::acceptsStructuralAndGeometryOnlyModels()
{
    QVERIFY(RStructuralProductProfile::validate(R_PROBLEM_NONE).isAccepted());
    QVERIFY(RStructuralProductProfile::validate(R_PROBLEM_STRESS).isAccepted());
    QVERIFY(RStructuralProductProfile::validate(R_PROBLEM_STRESS_MODAL).isAccepted());
    QVERIFY(RStructuralProductProfile::validate(R_PROBLEM_STRESS | R_PROBLEM_STRESS_MODAL).isAccepted());
}

void TestStructuralProductProfile::rejectsUnsupportedAndMixedModels()
{
    const RProductProfileValidation heat =
        RStructuralProductProfile::validate(R_PROBLEM_HEAT);
    QVERIFY(!heat.isAccepted());
    QCOMPARE(heat.getUnsupportedProblemTypeMask(), RProblemTypeMask(R_PROBLEM_HEAT));
    QVERIFY(heat.getDiagnostic().contains(RProblem::getName(R_PROBLEM_HEAT)));

    const RProductProfileValidation mixed =
        RStructuralProductProfile::validate(R_PROBLEM_STRESS | R_PROBLEM_FLUID);
    QVERIFY(!mixed.isAccepted());
    QCOMPARE(mixed.getUnsupportedProblemTypeMask(), RProblemTypeMask(R_PROBLEM_FLUID));
    QVERIFY(mixed.getDiagnostic().contains(RProblem::getName(R_PROBLEM_FLUID)));
    QVERIFY(!mixed.getDiagnostic().contains("Unsupported problem types: Stress analysis"));

    const RProductProfileValidation unknown =
        RStructuralProductProfile::validate(R_PROBLEM_STRESS | (1 << 20));
    QVERIFY(!unknown.isAccepted());
    QVERIFY(unknown.getDiagnostic().contains("0x100000"));
}

void TestStructuralProductProfile::keepsTemperatureAsStructuralInput()
{
    const RProblemTypeMask temperatureProblems =
        REnvironmentCondition::getProblemTypeMask(R_ENVIRONMENT_CONDITION_TEMPERATURE);

    QVERIFY(temperatureProblems & R_PROBLEM_STRESS);
    QVERIFY(temperatureProblems & R_PROBLEM_STRESS_MODAL);
}

void TestStructuralProductProfile::filtersStructuralProductFeatures()
{
    QVERIFY(RStructuralProductProfile::isEnvironmentConditionEnabled(
        R_ENVIRONMENT_CONDITION_TEMPERATURE));
    QVERIFY(RStructuralProductProfile::isEnvironmentConditionEnabled(
        R_ENVIRONMENT_CONDITION_G_ACCELERATION));
    QVERIFY(!RStructuralProductProfile::isInitialConditionEnabled(
        R_INITIAL_CONDITION_TEMPERATURE));

    QVERIFY(RStructuralProductProfile::isMaterialPropertyEnabled(
        RMaterialProperty::ModulusOfElasiticity));
    QVERIFY(RStructuralProductProfile::isMaterialPropertyEnabled(
        RMaterialProperty::ThermalExpansionCoefficient));
    QVERIFY(!RStructuralProductProfile::isMaterialPropertyEnabled(
        RMaterialProperty::ThermalConductivity));

    QVERIFY(RStructuralProductProfile::isResultVariableEnabled(R_VARIABLE_DISPLACEMENT));
    QVERIFY(RStructuralProductProfile::isResultVariableEnabled(R_VARIABLE_STRESS));
    QVERIFY(!RStructuralProductProfile::isResultVariableEnabled(R_VARIABLE_ELECTRIC_FIELD));
    QVERIFY(!RStructuralProductProfile::isTimeSolverEnabled());
}

void TestStructuralProductProfile::exposesFrameBoundaryContracts()
{
    QCOMPARE(RBoundaryCondition::getId(R_BOUNDARY_CONDITION_ROTATION),QString("bc-rotation"));
    QCOMPARE(RBoundaryCondition::getTypeFromId("bc-moment"),R_BOUNDARY_CONDITION_MOMENT);
    QVERIFY(RBoundaryCondition::getExplicit(R_BOUNDARY_CONDITION_ROTATION));
    QVERIFY(!RBoundaryCondition::getExplicit(R_BOUNDARY_CONDITION_MOMENT));
    QVERIFY(RBoundaryCondition::getApplyOnPoint(R_BOUNDARY_CONDITION_ROTATION));
    QVERIFY(RBoundaryCondition::getApplyOnLine(R_BOUNDARY_CONDITION_ROTATION));
    QVERIFY(RBoundaryCondition::getApplyOnPoint(R_BOUNDARY_CONDITION_MOMENT));
    QVERIFY(!RBoundaryCondition::getApplyOnLine(R_BOUNDARY_CONDITION_MOMENT));

    const std::vector<RVariableType> rotation =
        RBoundaryCondition::getDefaultComponents(R_BOUNDARY_CONDITION_ROTATION);
    QCOMPARE(rotation.size(),std::size_t(3));
    QCOMPARE(rotation[0],R_VARIABLE_ROTATION_X);
    QCOMPARE(rotation[1],R_VARIABLE_ROTATION_Y);
    QCOMPARE(rotation[2],R_VARIABLE_ROTATION_Z);

    const std::vector<RVariableType> moment =
        RBoundaryCondition::getDefaultComponents(R_BOUNDARY_CONDITION_MOMENT);
    QCOMPARE(moment.size(),std::size_t(3));
    QCOMPARE(moment[0],R_VARIABLE_MOMENT_X);
    QCOMPARE(moment[1],R_VARIABLE_MOMENT_Y);
    QCOMPARE(moment[2],R_VARIABLE_MOMENT_Z);
    QCOMPARE(RVariable::getUnits(R_VARIABLE_ROTATION),QString("rad"));
    QCOMPARE(RVariable::getUnits(R_VARIABLE_MOMENT),QString("N*m"));

    QVERIFY(RStructuralProductProfile::isBoundaryConditionEnabled(
        R_BOUNDARY_CONDITION_ROTATION));
    QVERIFY(RStructuralProductProfile::isBoundaryConditionEnabled(
        R_BOUNDARY_CONDITION_MOMENT));
    QVERIFY(RStructuralProductProfile::isResultVariableEnabled(R_VARIABLE_ROTATION));
    QVERIFY(RStructuralProductProfile::isResultVariableEnabled(R_VARIABLE_MOMENT));
}

void TestStructuralProductProfile::registryCoversProductDomains()
{
    const RCapabilityRegistry &registry = RCapabilityRegistry::getBuiltIn();
    for (RCapabilityDomain domain : {RCapabilityDomain::Analysis,
                                     RCapabilityDomain::Element,
                                     RCapabilityDomain::Material,
                                     RCapabilityDomain::BoundaryCondition,
                                     RCapabilityDomain::InitialCondition,
                                     RCapabilityDomain::EnvironmentCondition,
                                     RCapabilityDomain::Mesher,
                                     RCapabilityDomain::Solver})
    {
        QVERIFY2(!registry.list(domain).isEmpty(),
                 qPrintable(RCapability::domainToId(domain)));
    }

    const RCapability *heat = registry.find(RCapabilityDomain::Analysis,
                                             RProblem::getId(R_PROBLEM_HEAT));
    QVERIFY(heat);
    QVERIFY(heat->isRegistered());
    QVERIFY(heat->isImplemented());
    QVERIFY(heat->isExposed("full"));
    QVERIFY(!heat->isExposed(RStructuralProductProfile::getId()));

    const RCapability *stress = registry.find(RCapabilityDomain::Analysis,
                                               RProblem::getId(R_PROBLEM_STRESS));
    QVERIFY(stress);
    QVERIFY(stress->isTested());
    QVERIFY(stress->isValidated());
    QVERIFY(stress->isExposed("full"));
    QVERIFY(stress->isExposed(RStructuralProductProfile::getId()));
}

void TestStructuralProductProfile::registryRoundTripsJson()
{
    const RCapabilityRegistry &source = RCapabilityRegistry::getBuiltIn();
    RCapabilityRegistry restored;
    QString error;
    QVERIFY2(RCapabilityRegistry::fromJson(source.toJson(),restored,&error),qPrintable(error));
    QCOMPARE(restored.getAll().size(),source.getAll().size());

    const RCapability *tetgen = restored.find(RCapabilityDomain::Mesher,"tetgen.volume");
    QVERIFY(tetgen);
    QCOMPARE(tetgen->getName(),QString("TetGen volume mesher"));
    QVERIFY(tetgen->isImplemented());
    QVERIFY(tetgen->isExposed(RStructuralProductProfile::getId()));
}

void TestStructuralProductProfile::registryRejectsInvalidJson()
{
    RCapabilityRegistry registry;
    QString error;
    QJsonObject invalidVersion{{"schemaVersion",RCapabilityRegistry::SchemaVersion + 1},
                               {"capabilities",QJsonArray()}};
    QVERIFY(!RCapabilityRegistry::fromJson(invalidVersion,registry,&error));
    QVERIFY(!error.isEmpty());

    QJsonObject invalidCapability{{"schemaVersion",RCapabilityRegistry::SchemaVersion},
                                  {"capabilities",QJsonArray{QJsonObject{{"domain","unknown"},
                                                                         {"id","bad"}}}}};
    error.clear();
    QVERIFY(!RCapabilityRegistry::fromJson(invalidCapability,registry,&error));
    QVERIFY(error.contains("domain"));
}

QTEST_APPLESS_MAIN(TestStructuralProductProfile)

#include "tst_structural_product_profile.moc"
