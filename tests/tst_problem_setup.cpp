#include <QtTest>
#include <QTemporaryDir>

#include "rml_model.h"
#include "rml_file_io.h"

class TestProblemSetup : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndCopy();
    void rejectsInvalidMassMatrixType();
    void modelRoundTrip_data();
    void modelRoundTrip();
    void structuralCatalogAndCouplings();
    void structuralValidation();
    void dynamicsAndParallelValidation();
    void readsLegacyProblemSetup_data();
    void readsLegacyProblemSetup();
    void angularKinematicsDescriptors();
};

void TestProblemSetup::defaultsAndCopy()
{
    RProblemSetup setup;
    QCOMPARE(setup.getMassMatrixType(),RMassMatrixType::Consistent);

    setup.setMassMatrixType(RMassMatrixType::Lumped);
    const RProblemSetup copy(setup);
    QCOMPARE(copy.getMassMatrixType(),RMassMatrixType::Lumped);
    QVERIFY(!copy.getStructuralDynamicsSetup().getEnabled());
    QCOMPARE(copy.getParallelExecutionConfig().getMode(),RParallelExecutionMode::Auto);
}

void TestProblemSetup::rejectsInvalidMassMatrixType()
{
    RProblemSetup setup;
    QVERIFY_EXCEPTION_THROWN(
        setup.setMassMatrixType(static_cast<RMassMatrixType>(99)),
        RError);
    QCOMPARE(setup.getMassMatrixType(),RMassMatrixType::Consistent);
}

void TestProblemSetup::modelRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::addColumn<int>("massMatrixType");
    QTest::newRow("ascii-consistent") << false << int(RMassMatrixType::Consistent);
    QTest::newRow("ascii-lumped") << false << int(RMassMatrixType::Lumped);
    QTest::newRow("binary-consistent") << true << int(RMassMatrixType::Consistent);
    QTest::newRow("binary-lumped") << true << int(RMassMatrixType::Lumped);
}

void TestProblemSetup::modelRoundTrip()
{
    QFETCH(bool,binary);
    QFETCH(int,massMatrixType);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RModel source;
    source.getProblemSetup().setMassMatrixType(
        static_cast<RMassMatrixType>(massMatrixType));
    RLinearConstraint constraint;
    constraint.id = "mpc-1";
    constraint.name = "Equal axial displacement";
    constraint.rhs = 0.25;
    constraint.terms = {{{0,RDofType::UX},1.0},{{1,RDofType::UX},-1.0}};
    source.getProblemSetup().getStructuralInteractionSetup().getLinearConstraints().push_back(constraint);
    RStructuralSensor sensor;
    sensor.id = "sensor-u";
    sensor.name = "Displacement sensor";
    sensor.target.nodeId = 1;
    sensor.quantity = RStructuralSensorQuantity::Displacement;
    sensor.unit = "m";
    source.getProblemSetup().getStructuralInteractionSetup().getSensors().push_back(sensor);
    RStructuralDynamicsSetup &dynamics = source.getProblemSetup().getStructuralDynamicsSetup();
    dynamics.setEnabled(true);
    dynamics.setMethod(RStructuralTimeIntegrationMethod::GeneralizedAlpha);
    dynamics.setTimeStep(0.0025);
    dynamics.setNSteps(400);
    dynamics.setOutputFrequency(8);
    dynamics.setRayleighAlpha(0.12);
    dynamics.setRayleighBeta(0.0003);
    dynamics.setRhoInfinity(0.65);
    dynamics.setModalDampingRatios({0.02,0.03});
    RParallelExecutionConfig &parallel = source.getProblemSetup().getParallelExecutionConfig();
    parallel.setMode(RParallelExecutionMode::FixedThreads);
    parallel.setRequestedThreads(4);
    parallel.setDeterministicReduction(false);

    const QString extension = RModel::getDefaultFileExtension(binary);
    const QString fileName = directory.filePath("problem-setup." + extension);
    RModel restored;
    try
    {
        QCOMPARE(source.write(fileName,false),fileName);
        restored.read(fileName);
    }
    catch (const RError &error)
    {
        QFAIL(qPrintable(QString("Model round trip failed for %1: %2")
                         .arg(fileName,error.getMessage())));
    }

    QCOMPARE(restored.getProblemSetup().getMassMatrixType(),
             static_cast<RMassMatrixType>(massMatrixType));
    const RStructuralInteractionSetup &restoredSetup = restored.getProblemSetup().getStructuralInteractionSetup();
    QCOMPARE(restoredSetup.getLinearConstraints().size(),size_t(1));
    QCOMPARE(restoredSetup.getLinearConstraints()[0].id,QString("mpc-1"));
    QCOMPARE(restoredSetup.getLinearConstraints()[0].terms.size(),size_t(2));
    QCOMPARE(restoredSetup.getSensors().size(),size_t(1));
    QCOMPARE(restoredSetup.getSensors()[0].quantity,RStructuralSensorQuantity::Displacement);
    const RStructuralDynamicsSetup &restoredDynamics = restored.getProblemSetup().getStructuralDynamicsSetup();
    QVERIFY(restoredDynamics.getEnabled());
    QCOMPARE(restoredDynamics.getMethod(),RStructuralTimeIntegrationMethod::GeneralizedAlpha);
    QCOMPARE(restoredDynamics.getTimeStep(),0.0025);
    QCOMPARE(restoredDynamics.getNSteps(),uint(400));
    QCOMPARE(restoredDynamics.getOutputFrequency(),uint(8));
    QCOMPARE(restoredDynamics.getRayleighAlpha(),0.12);
    QCOMPARE(restoredDynamics.getRayleighBeta(),0.0003);
    QCOMPARE(restoredDynamics.getRhoInfinity(),0.65);
    QCOMPARE(restoredDynamics.getModalDampingRatios(),std::vector<double>({0.02,0.03}));
    const RParallelExecutionConfig &restoredParallel = restored.getProblemSetup().getParallelExecutionConfig();
    QCOMPARE(restoredParallel.getMode(),RParallelExecutionMode::FixedThreads);
    QCOMPARE(restoredParallel.getRequestedThreads(),uint(4));
    QVERIFY(!restoredParallel.getDeterministicReduction());
}

void TestProblemSetup::dynamicsAndParallelValidation()
{
    RStructuralDynamicsSetup dynamics;
    QVERIFY(!dynamics.getEnabled());
    QCOMPARE(dynamics.getMethod(),RStructuralTimeIntegrationMethod::Newmark);
    QVERIFY_EXCEPTION_THROWN(dynamics.setTimeStep(0.0),RError);
    QVERIFY_EXCEPTION_THROWN(dynamics.setNSteps(0),RError);
    QVERIFY_EXCEPTION_THROWN(dynamics.setOutputFrequency(0),RError);
    QVERIFY_EXCEPTION_THROWN(dynamics.setRayleighAlpha(-1.0),RError);
    QVERIFY_EXCEPTION_THROWN(dynamics.setRhoInfinity(1.1),RError);
    QVERIFY_EXCEPTION_THROWN(dynamics.setModalDampingRatios({0.02,1.0}),RError);
    QVERIFY_EXCEPTION_THROWN(dynamics.setMethod(static_cast<RStructuralTimeIntegrationMethod>(99)),RError);
    dynamics.setMethod(RStructuralTimeIntegrationMethod::CentralDifference);
    dynamics.validate();

    RParallelExecutionConfig parallel;
    QCOMPARE(parallel.getMode(),RParallelExecutionMode::Auto);
    QVERIFY(parallel.getDeterministicReduction());
    parallel.setMode(RParallelExecutionMode::FixedThreads);
    QVERIFY_EXCEPTION_THROWN(parallel.validate(),RError);
    parallel.setRequestedThreads(3);
    parallel.validate();
    parallel.setMode(RParallelExecutionMode::SerialDeterministic);
    QVERIFY_EXCEPTION_THROWN(parallel.validate(),RError);
    parallel.setRequestedThreads(0);
    parallel.validate();
}

void TestProblemSetup::readsLegacyProblemSetup_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.5") << false;
    QTest::newRow("binary-1.3.5") << true;
}

void TestProblemSetup::readsLegacyProblemSetup()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? "old-setup.bin" : "old-setup.txt");
    RProblemSetup source;
    source.setMassMatrixType(RMassMatrixType::Lumped);
    {
        RSaveFile file(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        QVERIFY(file.open(QIODevice::WriteOnly));
        if (binary)
        {
            RFileIO::writeBinary(file,source.getRestart());
            RFileIO::writeBinary(file,source.getRadiationSetup());
            RFileIO::writeBinary(file,source.getModalSetup());
            RFileIO::writeBinary(file,source.getMeshSetup());
            RFileIO::writeBinary(file,source.getAcousticSetup());
            RFileIO::writeBinary(file,int(source.getMassMatrixType()));
            RFileIO::writeBinary(file,source.getStructuralInteractionSetup());
        }
        else
        {
            RFileIO::writeAscii(file,source.getRestart());
            RFileIO::writeAscii(file,source.getRadiationSetup());
            RFileIO::writeAscii(file,source.getModalSetup());
            RFileIO::writeAscii(file,source.getMeshSetup());
            RFileIO::writeAscii(file,source.getAcousticSetup());
            RFileIO::writeAscii(file,int(source.getMassMatrixType()));
            RFileIO::writeAscii(file,source.getStructuralInteractionSetup());
        }
        QVERIFY(file.commit());
    }
    RFile file(path,binary ? RFile::BINARY : RFile::ASCII);
    file.setVersion(RVersion(1,3,5));
    QVERIFY(file.open(QIODevice::ReadOnly));
    RProblemSetup restored;
    restored.getStructuralDynamicsSetup().setEnabled(true);
    restored.getParallelExecutionConfig().setMode(RParallelExecutionMode::FixedThreads);
    restored.getParallelExecutionConfig().setRequestedThreads(2);
    if (binary) RFileIO::readBinary(file,restored);
    else RFileIO::readAscii(file,restored);
    QCOMPARE(restored.getMassMatrixType(),RMassMatrixType::Lumped);
    QVERIFY(!restored.getStructuralDynamicsSetup().getEnabled());
    QCOMPARE(restored.getParallelExecutionConfig().getMode(),RParallelExecutionMode::Auto);
}

void TestProblemSetup::angularKinematicsDescriptors()
{
    QCOMPARE(RVariable::getId(R_VARIABLE_ANGULAR_VELOCITY),QString("var-angular_velocity"));
    QCOMPARE(RVariable::getUnits(R_VARIABLE_ANGULAR_VELOCITY),QString("rad/s"));
    QCOMPARE(RVariable::getUnits(R_VARIABLE_ANGULAR_ACCELERATION),QString("rad/s^2"));
    QCOMPARE(RVariable::guessVectorName(R_VARIABLE_ANGULAR_VELOCITY_X,
                                       R_VARIABLE_ANGULAR_VELOCITY_Y,
                                       R_VARIABLE_ANGULAR_VELOCITY_Z),
             RVariable::getName(R_VARIABLE_ANGULAR_VELOCITY));
    QCOMPARE(RVariable::guessVectorUnits(R_VARIABLE_ANGULAR_ACCELERATION_X,
                                        R_VARIABLE_ANGULAR_ACCELERATION_Y,
                                        R_VARIABLE_ANGULAR_ACCELERATION_Z),
             RVariable::getUnits(R_VARIABLE_ANGULAR_ACCELERATION));
}

static RModel makeInteractionModel()
{
    RModel model;
    model.addNode(RNode(0.0,0.0,0.0));
    model.addNode(RNode(1.0,0.0,0.0));
    model.addNode(RNode(2.0,0.0,0.0));
    RElement first(R_ELEMENT_TRUSS1); first.setNodeId(0,0); first.setNodeId(1,1);
    RElement second(R_ELEMENT_TRUSS1); second.setNodeId(0,1); second.setNodeId(1,2);
    model.addElement(first); model.addElement(second);
    RLine line; line.setName("coupled-edge"); line.add(0); line.add(1); model.addLine(line);
    return model;
}

void TestProblemSetup::structuralCatalogAndCouplings()
{
    RModel model = makeInteractionModel();
    RStructuralInteractionSetup &setup = model.getProblemSetup().getStructuralInteractionSetup();

    RStructuralCoupling rigid;
    rigid.id = "rigid-1";
    rigid.name = "Rigid edge";
    rigid.type = RStructuralCouplingType::RigidBody;
    rigid.referenceNode = 0;
    rigid.slaveSet = "coupled-edge";
    rigid.entityType = R_ENTITY_GROUP_LINE;
    rigid.dofs = {RDofType::UX,RDofType::UY,RDofType::UZ};
    setup.getCouplings().push_back(rigid);

    const std::vector<uint> nodes = RStructuralInteractionSetup::resolveNamedSetNodes(model,"coupled-edge",R_ENTITY_GROUP_LINE);
    QCOMPARE(nodes.size(),size_t(3));
    const std::vector<RLinearConstraint> equations = setup.buildLinearConstraints(model);
    QCOMPARE(equations.size(),size_t(6));
    QVERIFY(equations[0].id.startsWith("rigid-1:"));
    QVERIFY(equations[1].terms.size() >= size_t(2));
}

void TestProblemSetup::structuralValidation()
{
    RModel model = makeInteractionModel();
    RStructuralInteractionSetup &setup = model.getProblemSetup().getStructuralInteractionSetup();

    RLinearConstraint a;
    a.id = "held-x";
    a.terms = {{{0,RDofType::UX},1.0}};
    setup.getLinearConstraints().push_back(a);
    RLinearConstraint b = a;
    b.id = "held-x-conflict";
    b.rhs = 1.0;
    setup.getLinearConstraints().push_back(b);

    RStructuralSensor sensor;
    sensor.id = "strain-edge";
    sensor.target.scope = RStructuralTargetScope::NamedSet;
    sensor.target.setName = "coupled-edge";
    sensor.target.entityType = R_ENTITY_GROUP_LINE;
    sensor.quantity = RStructuralSensorQuantity::Strain;
    sensor.unit = "1";
    setup.getSensors().push_back(sensor);

    RStructuralActuator actuator;
    actuator.id = "base-x";
    actuator.type = RStructuralActuatorType::BaseMotion;
    actuator.target.nodeId = 0;
    actuator.variable = R_VARIABLE_ACCELERATION;
    actuator.unit = RVariable::getUnits(R_VARIABLE_ACCELERATION);
    actuator.minimum = -10.0;
    actuator.maximum = 10.0;
    actuator.rateLimit = 100.0;
    setup.getActuators().push_back(actuator);

    const RStructuralValidationReport report = setup.validate(model);
    QVERIFY(report.hasErrors());
    bool contradiction = false;
    bool rigidWarning = false;
    for (const RStructuralDiagnostic &diagnostic : report.getDiagnostics())
    {
        contradiction |= diagnostic.code == "contradictory-equations";
        rigidWarning |= diagnostic.code == "rigid-mode-rank";
    }
    QVERIFY(contradiction);
    QVERIFY(rigidWarning);
}

QTEST_APPLESS_MAIN(TestProblemSetup)

#include "tst_problem_setup.moc"
