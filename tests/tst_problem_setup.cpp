#include <QtTest>
#include <QTemporaryDir>
#include <QDir>

#include <algorithm>
#include <cmath>
#include <limits>

#include "rml_model.h"
#include "rml_file_io.h"
#include "rml_file_utils.h"

static void writeLegacyEmptyInteractionSetup(RSaveFile &file, bool binary)
{
    // Through 1.3.8 the interaction record had exactly four vector counts.
    for (int i=0;i<4;i++)
    {
        if (binary) RFileIO::writeBinary(file,0u);
        else RFileIO::writeAscii(file,0u);
    }
}

class TestProblemSetup : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndCopy();
    void rejectsInvalidMassMatrixType();
    void modelRoundTrip_data();
    void modelRoundTrip();
    void modalDampingAsciiPrecisionRoundTrip();
    void resultVariableWithWhitespaceRoundTrip_data();
    void resultVariableWithWhitespaceRoundTrip();
    void prescribedMotionHermiteAndValidation();
    void prescribedMotionRoundTrip_data();
    void prescribedMotionRoundTrip();
    void prescribedMotionAsciiPrecisionRoundTrip();
    void readsVersion138WithoutPrescribedMotion_data();
    void readsVersion138WithoutPrescribedMotion();
    void structuralCatalogAndCouplings();
    void structuralValidation();
    void dynamicsAndParallelValidation();
    void readsLegacyProblemSetup_data();
    void readsLegacyProblemSetup();
    void restartRoundTrip_data();
    void restartRoundTrip();
    void controlValidation();
    void readsVersion1311WithoutConditioning_data();
    void readsVersion1311WithoutConditioning();
    void readsVersion1310WithoutControlLedger_data();
    void readsVersion1310WithoutControlLedger();
    void readsVersion139WithoutSupportLedger_data();
    void readsVersion139WithoutSupportLedger();
    void restartRejectsChangedMeshAndDofOrder();
    void readsVersion136WithoutCheckpoint_data();
    void readsVersion136WithoutCheckpoint();
    void readsVersion137ButDisablesUnboundCheckpoint_data();
    void readsVersion137ButDisablesUnboundCheckpoint();
    void angularKinematicsDescriptors();
    void rotationalInitialConditions();
    void failedTargetWriteKeepsPreviousLink_data();
    void failedTargetWriteKeepsPreviousLink();
};

void TestProblemSetup::defaultsAndCopy()
{
    RModelData modelData;
    QVERIFY(!modelData.getSelected());
    modelData.setSelected(true);
    const RModelData copiedModelData(modelData);
    QVERIFY(copiedModelData.getSelected());

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

void TestProblemSetup::modalDampingAsciiPrecisionRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const std::vector<double> ratios = {0.05,0.05000000000000001,
                                         0.05000000000000002};
    QVERIFY(ratios[0] != ratios[1]);
    QVERIFY(ratios[1] != ratios[2]);

    RModel source;
    source.getProblemSetup().getStructuralDynamicsSetup().setModalDampingRatios(ratios);
    const QString fileName = directory.filePath(QStringLiteral("precise-ratios.ram"));
    RModel restored;
    try
    {
        QCOMPARE(source.write(fileName,false),fileName);
        restored.read(fileName);
    }
    catch (const RError &error)
    {
        QFAIL(qPrintable(QString("Modal damping ASCII round trip failed: %1")
                         .arg(error.getMessage())));
    }
    QCOMPARE(restored.getProblemSetup().getStructuralDynamicsSetup()
                 .getModalDampingRatios(),ratios);

    // The precision change must be scoped to this field: other writers that
    // reuse the stream must retain their caller-selected precision.
    const QString streamFile = directory.filePath(QStringLiteral("precision-scope.txt"));
    RSaveFile file(streamFile,RSaveFile::ASCII);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.getTextStream().setRealNumberPrecision(9);
    RFileIO::writeAscii(file,source.getProblemSetup().getStructuralDynamicsSetup());
    QCOMPARE(file.getTextStream().realNumberPrecision(),9);
    QVERIFY(file.commit());
}

void TestProblemSetup::resultVariableWithWhitespaceRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii") << false;
    QTest::newRow("binary") << true;
}

void TestProblemSetup::resultVariableWithWhitespaceRoundTrip()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RModel source;
    RVariable stress(R_VARIABLE_STRESS_VON_MISES,R_VARIABLE_APPLY_ELEMENT);
    stress.setUnits("Pa s");
    stress.resize(1,0);
    source.addVariable(stress);

    const QString fileName = directory.filePath(
        "result-variable." + RModel::getDefaultFileExtension(binary));
    RModel restored;
    try
    {
        QCOMPARE(source.write(fileName,false),fileName);
        restored.read(fileName);
    }
    catch (const RError &error)
    {
        QFAIL(qPrintable(QString("Result variable round trip failed: %1")
                         .arg(error.getMessage())));
    }
    QCOMPARE(restored.getNVariables(),1u);
    QCOMPARE(restored.getVariable(0).getType(),R_VARIABLE_STRESS_VON_MISES);
    QCOMPARE(restored.getVariable(0).getName(),QString("Von Mises stress"));
    QCOMPARE(restored.getVariable(0).getUnits(),QString("Pa s"));
    QCOMPARE(restored.getVariable(0).getNVectors(),1u);
}

void TestProblemSetup::prescribedMotionHermiteAndValidation()
{
    RStructuralPrescribedMotion motion;
    motion.id = "base-x";
    motion.target = {0,RDofType::UX};
    // Exact quintic g=t^5, v=5t^4, a=20t^3.
    motion.knots = {{0.0,0.0,0.0,0.0},{1.0,1.0,5.0,20.0}};
    motion.validate();
    const RStructuralMotionState middle = motion.evaluate(0.5);
    QCOMPARE(middle.displacement,0.03125);
    QCOMPARE(middle.velocity,0.3125);
    QCOMPARE(middle.acceleration,2.5);
    QCOMPARE(motion.evaluate(0.0).displacement,0.0);
    QCOMPARE(motion.evaluate(1.0).acceleration,20.0);
    QCOMPARE(motion.evaluate(std::nextafter(1.0,std::numeric_limits<double>::infinity())).displacement,1.0);
    QVERIFY_EXCEPTION_THROWN(motion.evaluate(-0.001),RError);
    QVERIFY_EXCEPTION_THROWN(motion.evaluate(1.001),RError);

    RStructuralPrescribedMotion invalid = motion;
    invalid.knots[1].time = invalid.knots[0].time;
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    invalid = motion;
    invalid.knots[1].acceleration = std::numeric_limits<double>::infinity();
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    invalid = motion;
    invalid.target.dof = RDofType::Temperature;
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    invalid = motion;
    invalid.knots[1].time = 1.0e-200;
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    invalid = motion;
    invalid.id.clear();
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);

    RModel model;
    model.setNNodes(1);
    auto &motions = model.getProblemSetup().getStructuralInteractionSetup().getPrescribedMotions();
    motions.push_back(motion);
    RStructuralPrescribedMotion duplicate = motion;
    duplicate.id = "base-x-duplicate";
    motions.push_back(duplicate);
    const auto report = model.getProblemSetup().getStructuralInteractionSetup().validate(model);
    QVERIFY(report.hasErrors());
    QVERIFY(std::any_of(report.getDiagnostics().begin(),report.getDiagnostics().end(),
                        [](const RStructuralDiagnostic &diagnostic)
                        { return diagnostic.code == "duplicate-motion-target"; }));
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY_EXCEPTION_THROWN(model.write(directory.filePath("duplicate-motion.ram"),false),RError);
}

void TestProblemSetup::prescribedMotionRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.9") << false;
    QTest::newRow("binary-1.3.9") << true;
}

void TestProblemSetup::prescribedMotionRoundTrip()
{
    QFETCH(bool,binary);
    QCOMPARE(RModel::version.toString(),QString("1.3.13"));
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    RModel source;
    source.setNNodes(2);
    RStructuralPrescribedMotion motion;
    motion.id = "base X";
    motion.target = {1,RDofType::UX};
    motion.knots = {{0.0,0.0,0.0,2.0},{0.5,0.25,1.0,2.0},{1.0,1.0,2.0,2.0}};
    source.getProblemSetup().getStructuralInteractionSetup().getPrescribedMotions().push_back(motion);

    const QString fileName = directory.filePath(
        "motion." + RModel::getDefaultFileExtension(binary));
    RModel restored;
    try
    {
        QCOMPARE(source.write(fileName,false),fileName);
        restored.read(fileName);
    }
    catch (const RError &error)
    {
        QFAIL(qPrintable(QString("Prescribed motion round trip failed: %1")
                         .arg(error.getMessage())));
    }
    const auto &motions = restored.getProblemSetup().getStructuralInteractionSetup().getPrescribedMotions();
    QCOMPARE(motions.size(),std::size_t(1));
    QCOMPARE(motions[0].id,QString("base X"));
    QCOMPARE(motions[0].target.nodeId,1u);
    QCOMPARE(motions[0].target.dof,RDofType::UX);
    QCOMPARE(motions[0].knots.size(),std::size_t(3));
    QCOMPARE(motions[0].evaluate(0.25).displacement,0.0625);
    QCOMPARE(motions[0].evaluate(0.25).velocity,0.5);
    QCOMPARE(motions[0].evaluate(0.25).acceleration,2.0);
}

void TestProblemSetup::prescribedMotionAsciiPrecisionRoundTrip()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    RModel source;
    source.setNNodes(1);
    RStructuralPrescribedMotion motion;
    motion.id = "precision";
    motion.target = {0,RDofType::UX};
    motion.knots = {
        {0.12345678901234567,0.98765432109876543,1.4142135623730951,-0.33333333333333331},
        {1.2345678901234567,-0.27182818284590452,2.7182818284590451,3.1415926535897931}
    };
    source.getProblemSetup().getStructuralInteractionSetup().getPrescribedMotions().push_back(motion);

    const QString path = directory.filePath("precise-motion.ram");
    RModel restored;
    try
    {
        QCOMPARE(source.write(path,false),path);
        restored.read(path);
    }
    catch (const RError &error)
    {
        QFAIL(qPrintable(QString("Precise motion ASCII round trip failed: %1")
                         .arg(error.getMessage())));
    }
    const auto &readMotions = restored.getProblemSetup().getStructuralInteractionSetup().getPrescribedMotions();
    QCOMPARE(readMotions.size(),std::size_t(1));
    QCOMPARE(readMotions[0].knots.size(),motion.knots.size());
    for (std::size_t i=0;i<motion.knots.size();i++)
    {
        QCOMPARE(readMotions[0].knots[i].time,motion.knots[i].time);
        QCOMPARE(readMotions[0].knots[i].displacement,motion.knots[i].displacement);
        QCOMPARE(readMotions[0].knots[i].velocity,motion.knots[i].velocity);
        QCOMPARE(readMotions[0].knots[i].acceleration,motion.knots[i].acceleration);
    }
}

void TestProblemSetup::readsVersion138WithoutPrescribedMotion_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.8") << false;
    QTest::newRow("binary-1.3.8") << true;
}

void TestProblemSetup::readsVersion138WithoutPrescribedMotion()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? "old-interaction.bin" : "old-interaction.txt");
    {
        RSaveFile file(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        QVERIFY(file.open(QIODevice::WriteOnly));
        // Version 1.3.8 contains four empty interaction vectors and no motion count.
        for (int i=0;i<4;i++)
        {
            if (binary) RFileIO::writeBinary(file,0u);
            else RFileIO::writeAscii(file,0u);
        }
        if (binary) RFileIO::writeBinary(file,12345);
        else RFileIO::writeAscii(file,12345);
        QVERIFY(file.commit());
    }
    RFile file(path,binary ? RFile::BINARY : RFile::ASCII);
    file.setVersion(RVersion(1,3,8));
    QVERIFY(file.open(QIODevice::ReadOnly));
    RStructuralInteractionSetup setup;
    RStructuralPrescribedMotion stale;
    stale.id = "stale";
    stale.knots = {{0.0,0.0,0.0,0.0},{1.0,0.0,0.0,0.0}};
    setup.getPrescribedMotions().push_back(stale);
    if (binary) RFileIO::readBinary(file,setup);
    else RFileIO::readAscii(file,setup);
    QVERIFY(setup.getPrescribedMotions().empty());
    int sentinel = 0;
    if (binary) RFileIO::readBinary(file,sentinel);
    else RFileIO::readAscii(file,sentinel);
    QCOMPARE(sentinel,12345);
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
            writeLegacyEmptyInteractionSetup(file,true);
        }
        else
        {
            RFileIO::writeAscii(file,source.getRestart());
            RFileIO::writeAscii(file,source.getRadiationSetup());
            RFileIO::writeAscii(file,source.getModalSetup());
            RFileIO::writeAscii(file,source.getMeshSetup());
            RFileIO::writeAscii(file,source.getAcousticSetup());
            RFileIO::writeAscii(file,int(source.getMassMatrixType()));
            writeLegacyEmptyInteractionSetup(file,false);
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
    QVERIFY(!restored.getStructuralDynamicsRestartData().available);
}

static RModel makeRestartModel()
{
    RModel model;
    model.addNode(RNode(0.0,0.0,0.0));
    model.addNode(RNode(1.23456789,0.0,0.0));
    RElement element(R_ELEMENT_TRUSS1);
    element.setNodeId(0,0);
    element.setNodeId(1,1);
    model.addElement(element);
    return model;
}

static std::vector<RStructuralDofRef> restartDofs()
{
    return {{0,RDofType::UX},{1,RDofType::UX}};
}

void TestProblemSetup::restartRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii") << false;
    QTest::newRow("binary") << true;
}

void TestProblemSetup::restartRoundTrip()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    RModel source = makeRestartModel();
    const auto dofs = restartDofs();
    RStructuralDynamicsRestartData checkpoint;
    checkpoint.available = true;
    checkpoint.completedSteps = 7;
    checkpoint.time = 0.0175;
    checkpoint.fixedTimeStep = 0.0025;
    checkpoint.meshNodeCount = source.getNNodes();
    checkpoint.meshElementCount = source.getNElements();
    checkpoint.physicalDofCount = uint(dofs.size());
    checkpoint.meshFingerprint = RStructuralDynamicsRestartData::computeMeshFingerprint(source);
    checkpoint.dofFingerprint = RStructuralDynamicsRestartData::computeDofFingerprint(dofs);
    checkpoint.systemFingerprint = QString(64,QLatin1Char('a'));
    checkpoint.q = {0.12345678901234566};
    checkpoint.v = {-0.98765432101234567};
    checkpoint.a = {0.33333333333333331};
    checkpoint.externalForce = {1.1234567890123456};
    checkpoint.cumulativeExternalWork = 0.12345678901234566;
    checkpoint.cumulativeDampingDissipation = 0.09876543210123456;
    checkpoint.initialMechanicalEnergy = 1.2345678901234567;
    checkpoint.supportResponseAvailable = true;
    checkpoint.supportFingerprint = QString(64,QLatin1Char('d'));
    checkpoint.supportReactions = {-123.45678901234566};
    checkpoint.supportPowers = {0.31415926535897931};
    checkpoint.cumulativeSupportWork = {1.4142135623730951};
    checkpoint.controlAvailable = true;
    checkpoint.controlFingerprint = QString(64,QLatin1Char('e'));
    checkpoint.controlChannelIds = {QStringLiteral("actuator.z"),QStringLiteral("actuator.x")};
    checkpoint.controlLastCommands = {1.2345678901234567,-2.3456789012345678};
    checkpoint.controlPowers = {0.22345678901234567,-0.4456789012345678};
    checkpoint.cumulativeControlWork = {0.012345678901234567,-0.023456789012345678};
    checkpoint.controlControllerState = QByteArray::fromHex("0001feff");
    checkpoint.controlDeterministicSeed = quint64(18446744073709551615ull);
    checkpoint.validate();
    source.getProblemSetup().setStructuralDynamicsRestartData(checkpoint);
    RStructuralControlSetup controlSetup;
    controlSetup.enabled = true;
    controlSetup.controllerId = QStringLiteral("controller.pid.v1");
    controlSetup.deterministicSeed = checkpoint.controlDeterministicSeed;
    controlSetup.channelConditioning = {
        {QStringLiteral("actuator.z"),2,3,0.12345678901234566},
        {QStringLiteral("actuator.x"),0,1,0.0}};
    source.getProblemSetup().setStructuralControlSetup(controlSetup);

    const QString path = directory.filePath(binary ? "checkpoint.rbm" : "checkpoint.ram");
    RModel restored;
    QCOMPARE(source.write(path,false),path);
    restored.read(path);
    const auto &readBack = restored.getProblemSetup().getStructuralDynamicsRestartData();
    QVERIFY(readBack.available);
    QCOMPARE(readBack.completedSteps,uint(7));
    QCOMPARE(readBack.time,checkpoint.time);
    QCOMPARE(readBack.fixedTimeStep,checkpoint.fixedTimeStep);
    QCOMPARE(readBack.q,checkpoint.q);
    QCOMPARE(readBack.v,checkpoint.v);
    QCOMPARE(readBack.a,checkpoint.a);
    QCOMPARE(readBack.externalForce,checkpoint.externalForce);
    QCOMPARE(readBack.cumulativeExternalWork,checkpoint.cumulativeExternalWork);
    QCOMPARE(readBack.cumulativeDampingDissipation,checkpoint.cumulativeDampingDissipation);
    QCOMPARE(readBack.initialMechanicalEnergy,checkpoint.initialMechanicalEnergy);
    QCOMPARE(readBack.supportResponseAvailable,checkpoint.supportResponseAvailable);
    QCOMPARE(readBack.supportFingerprint,checkpoint.supportFingerprint);
    QCOMPARE(readBack.supportReactions,checkpoint.supportReactions);
    QCOMPARE(readBack.supportPowers,checkpoint.supportPowers);
    QCOMPARE(readBack.cumulativeSupportWork,checkpoint.cumulativeSupportWork);
    QCOMPARE(readBack.controlAvailable,checkpoint.controlAvailable);
    QCOMPARE(readBack.controlProtocolVersion,checkpoint.controlProtocolVersion);
    QCOMPARE(readBack.controlFingerprint,checkpoint.controlFingerprint);
    QCOMPARE(readBack.controlChannelIds,checkpoint.controlChannelIds);
    QCOMPARE(readBack.controlLastCommands,checkpoint.controlLastCommands);
    QCOMPARE(readBack.controlPowers,checkpoint.controlPowers);
    QCOMPARE(readBack.cumulativeControlWork,checkpoint.cumulativeControlWork);
    QCOMPARE(readBack.controlControllerState,checkpoint.controlControllerState);
    QCOMPARE(readBack.controlDeterministicSeed,checkpoint.controlDeterministicSeed);
    const auto &restoredSetup = restored.getProblemSetup().getStructuralControlSetup();
    QCOMPARE(restoredSetup.enabled,controlSetup.enabled);
    QCOMPARE(restoredSetup.controlProtocolVersion,controlSetup.controlProtocolVersion);
    QCOMPARE(restoredSetup.controllerId,controlSetup.controllerId);
    QCOMPARE(restoredSetup.deterministicSeed,controlSetup.deterministicSeed);
    QCOMPARE(restoredSetup.channelConditioning.size(),controlSetup.channelConditioning.size());
    for (size_t i=0;i<controlSetup.channelConditioning.size();i++)
    {
        QCOMPARE(restoredSetup.channelConditioning[i].actuatorId,
                 controlSetup.channelConditioning[i].actuatorId);
        QCOMPARE(restoredSetup.channelConditioning[i].delaySteps,
                 controlSetup.channelConditioning[i].delaySteps);
        QCOMPARE(restoredSetup.channelConditioning[i].holdSteps,
                 controlSetup.channelConditioning[i].holdSteps);
        QCOMPARE(restoredSetup.channelConditioning[i].noiseAmplitude,
                 controlSetup.channelConditioning[i].noiseAmplitude);
    }
    QCOMPARE(readBack.systemFingerprint,checkpoint.systemFingerprint);
    QVERIFY(readBack.matches(restored,dofs));
}

void TestProblemSetup::controlValidation()
{
    RStructuralDynamicsRestartData unavailable;
    unavailable.controlAvailable = true;
    QVERIFY_EXCEPTION_THROWN(unavailable.validate(),RError);

    RStructuralControlSetup setup;
    setup.validate();
    setup.enabled = true;
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.controllerId = QStringLiteral("controller.v1");
    setup.deterministicSeed = 19;
    setup.validate();
    setup.controlProtocolVersion = 2;
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.controlProtocolVersion = 1;
    setup.controllerId = QStringLiteral("controller bad");
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.controllerId = QStringLiteral("controller.v1");
    setup.channelConditioning = {{QStringLiteral("actuator"),1,2,0.1}};
    setup.validate();
    setup.channelConditioning[0].holdSteps = 0;
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.channelConditioning[0].holdSteps = 2;
    setup.channelConditioning[0].noiseAmplitude = std::numeric_limits<double>::infinity();
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.channelConditioning[0].noiseAmplitude = -0.1;
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.channelConditioning[0].noiseAmplitude = 0.1;
    setup.channelConditioning.push_back(setup.channelConditioning[0]);
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.channelConditioning[1].actuatorId = QStringLiteral("other");
    setup.channelConditioning[0].delaySteps = 16384;
    setup.channelConditioning[1].delaySteps = 1;
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.channelConditioning.clear();
    setup.enabled = false;
    setup.controllerId.clear();
    setup.deterministicSeed = 0;
    setup.validate();
    setup.channelConditioning = {{QStringLiteral("actuator"),0,1,0.0}};
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);

    RStructuralDynamicsRestartData checkpoint;
    checkpoint.available = true;
    checkpoint.fixedTimeStep = 0.01;
    checkpoint.meshNodeCount = 2;
    checkpoint.meshElementCount = 1;
    checkpoint.physicalDofCount = 1;
    checkpoint.meshFingerprint = QString(64,QLatin1Char('a'));
    checkpoint.dofFingerprint = QString(64,QLatin1Char('b'));
    checkpoint.systemFingerprint = QString(64,QLatin1Char('c'));
    checkpoint.q = checkpoint.v = checkpoint.a = checkpoint.externalForce = {0.0};
    checkpoint.validate();
    checkpoint.controlAvailable = true;
    QVERIFY_EXCEPTION_THROWN(checkpoint.validate(),RError);
    checkpoint.controlFingerprint = QString(64,QLatin1Char('d'));
    checkpoint.controlChannelIds = {QStringLiteral("actuator.1"),QStringLiteral("actuator.2")};
    checkpoint.controlLastCommands = {0.0,0.0};
    checkpoint.controlPowers = {0.0,0.0};
    checkpoint.cumulativeControlWork = {0.0,0.0};
    checkpoint.validate();
    checkpoint.controlChannelIds[1] = checkpoint.controlChannelIds[0];
    QVERIFY_EXCEPTION_THROWN(checkpoint.validate(),RError);
    checkpoint.controlChannelIds[1] = QStringLiteral("actuator.2");
    checkpoint.controlPowers[1] = std::numeric_limits<double>::infinity();
    QVERIFY_EXCEPTION_THROWN(checkpoint.validate(),RError);
    checkpoint.controlPowers[1] = 0.0;
    checkpoint.controlControllerState = QByteArray(1048577,'x');
    QVERIFY_EXCEPTION_THROWN(checkpoint.validate(),RError);
    checkpoint.controlControllerState.clear();
    checkpoint.controlProtocolVersion = 2;
    QVERIFY_EXCEPTION_THROWN(checkpoint.validate(),RError);
    checkpoint.controlProtocolVersion = 1;
    checkpoint.validate();
    checkpoint.controlAvailable = false;
    QVERIFY_EXCEPTION_THROWN(checkpoint.validate(),RError);
}

void TestProblemSetup::readsVersion1311WithoutConditioning_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.11") << false;
    QTest::newRow("binary-1.3.11") << true;
}

void TestProblemSetup::readsVersion1311WithoutConditioning()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? "control-1311.bin" : "control-1311.txt");
    {
        RSaveFile file(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        QVERIFY(file.open(QIODevice::WriteOnly));
        auto write = [&](const auto &value)
        {
            if (binary) RFileIO::writeBinary(file,value);
            else RFileIO::writeAscii(file,value);
        };
        write(true);
        write(uint(1));
        write(QStringLiteral("controller.old"));
        if (binary)
        {
            write(uint(123)); // seed low word
            write(uint(0));   // seed high word
        }
        else write(QStringLiteral("123"));
        QVERIFY(file.commit());
    }
    RFile file(path,binary ? RFile::BINARY : RFile::ASCII);
    file.setVersion(RVersion(1,3,11));
    QVERIFY(file.open(QIODevice::ReadOnly));
    RStructuralControlSetup restored;
    if (binary) RFileIO::readBinary(file,restored);
    else RFileIO::readAscii(file,restored);
    QVERIFY(restored.enabled);
    QCOMPARE(restored.controllerId,QStringLiteral("controller.old"));
    QCOMPARE(restored.deterministicSeed,quint64(123));
    QVERIFY(restored.channelConditioning.empty());
}

void TestProblemSetup::readsVersion1310WithoutControlLedger_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.10") << false;
    QTest::newRow("binary-1.3.10") << true;
}

void TestProblemSetup::readsVersion1310WithoutControlLedger()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? "checkpoint-1310.bin" : "checkpoint-1310.txt");
    {
        RSaveFile file(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        QVERIFY(file.open(QIODevice::WriteOnly));
        auto write = [&](const auto &value)
        {
            if (binary) RFileIO::writeBinary(file,value);
            else RFileIO::writeAscii(file,value);
        };
        write(true);
        write(uint(2));
        write(0.02);
        write(0.01);
        write(uint(2));
        write(uint(1));
        write(uint(1));
        write(QString(64,QLatin1Char('a')));
        write(QString(64,QLatin1Char('b')));
        write(QString(64,QLatin1Char('c')));
        write(uint(1));
        for (uint value=0;value<4;value++) write(double(value));
        for (uint value=0;value<3;value++) write(double(value));
        write(false); // 1.3.10 support-response ledger flag is the last field.
        QVERIFY(file.commit());
    }
    RFile file(path,binary ? RFile::BINARY : RFile::ASCII);
    file.setVersion(RVersion(1,3,10));
    QVERIFY(file.open(QIODevice::ReadOnly));
    RStructuralDynamicsRestartData restored;
    if (binary) RFileIO::readBinary(file,restored);
    else RFileIO::readAscii(file,restored);
    QVERIFY(restored.available);
    QVERIFY(!restored.controlAvailable);
    QVERIFY(restored.controlChannelIds.empty());
    QCOMPARE(restored.controlProtocolVersion,uint(1));
}

void TestProblemSetup::readsVersion139WithoutSupportLedger_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.9") << false;
    QTest::newRow("binary-1.3.9") << true;
}

void TestProblemSetup::readsVersion139WithoutSupportLedger()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? "checkpoint-139.bin" : "checkpoint-139.txt");
    {
        RSaveFile file(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        QVERIFY(file.open(QIODevice::WriteOnly));
        auto write = [&](const auto &value)
        {
            if (binary) RFileIO::writeBinary(file,value);
            else RFileIO::writeAscii(file,value);
        };
        write(true);
        write(uint(2));
        write(0.02);
        write(0.01);
        write(uint(2));
        write(uint(1));
        write(uint(1));
        write(QString(64,QLatin1Char('a')));
        write(QString(64,QLatin1Char('b')));
        write(QString(64,QLatin1Char('c')));
        write(uint(1));
        for (uint value=0;value<4;value++) write(double(value));
        for (uint value=0;value<3;value++) write(double(value));
        QVERIFY(file.commit());
    }
    RFile file(path,binary ? RFile::BINARY : RFile::ASCII);
    file.setVersion(RVersion(1,3,9));
    QVERIFY(file.open(QIODevice::ReadOnly));
    RStructuralDynamicsRestartData restored;
    if (binary) RFileIO::readBinary(file,restored);
    else RFileIO::readAscii(file,restored);
    QVERIFY(restored.available);
    QCOMPARE(restored.completedSteps,uint(2));
    QVERIFY(!restored.supportResponseAvailable);
    QVERIFY(restored.supportReactions.empty());
    QVERIFY(restored.supportPowers.empty());
    QVERIFY(restored.cumulativeSupportWork.empty());
}

void TestProblemSetup::restartRejectsChangedMeshAndDofOrder()
{
    RModel model = makeRestartModel();
    const auto dofs = restartDofs();
    RStructuralDynamicsRestartData checkpoint;
    checkpoint.available = true;
    checkpoint.fixedTimeStep = 0.01;
    checkpoint.meshNodeCount = model.getNNodes();
    checkpoint.meshElementCount = model.getNElements();
    checkpoint.physicalDofCount = uint(dofs.size());
    checkpoint.meshFingerprint = RStructuralDynamicsRestartData::computeMeshFingerprint(model);
    checkpoint.dofFingerprint = RStructuralDynamicsRestartData::computeDofFingerprint(dofs);
    checkpoint.systemFingerprint = QString(64,QLatin1Char('b'));
    checkpoint.q = {0.0};
    checkpoint.v = {0.0};
    checkpoint.a = {0.0};
    checkpoint.externalForce = {0.0};
    QVERIFY(checkpoint.matches(model,dofs));
    model.getNode(1).scale(0.1);
    model.getNode(1).scale(10.0);
    QVERIFY(checkpoint.matches(model,dofs));
    QVERIFY(!checkpoint.matches(model,{{1,RDofType::UX},{0,RDofType::UX}}));
    model.getNode(1).setX(1.4);
    QVERIFY(!checkpoint.matches(model,dofs));
    checkpoint.q.push_back(1.0);
    QVERIFY_EXCEPTION_THROWN(checkpoint.validate(),RError);
}

void TestProblemSetup::readsVersion137ButDisablesUnboundCheckpoint_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.7") << false;
    QTest::newRow("binary-1.3.7") << true;
}

void TestProblemSetup::readsVersion137ButDisablesUnboundCheckpoint()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? "old-137.bin" : "old-137.txt");
    RProblemSetup source;
    source.getStructuralDynamicsSetup().setEnabled(true);
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
            writeLegacyEmptyInteractionSetup(file,true);
            RFileIO::writeBinary(file,source.getStructuralDynamicsSetup());
            RFileIO::writeBinary(file,source.getParallelExecutionConfig());
            RFileIO::writeBinary(file,true);
            RFileIO::writeBinary(file,uint(2));
            RFileIO::writeBinary(file,0.02);
            RFileIO::writeBinary(file,0.01);
            RFileIO::writeBinary(file,uint(2));
            RFileIO::writeBinary(file,uint(1));
            RFileIO::writeBinary(file,uint(2));
            RFileIO::writeBinary(file,QString(64,QLatin1Char('a')));
            RFileIO::writeBinary(file,QString(64,QLatin1Char('b')));
            RFileIO::writeBinary(file,uint(1));
            for (uint i=0;i<4;i++) RFileIO::writeBinary(file,0.0);
            for (uint i=0;i<3;i++) RFileIO::writeBinary(file,0.0);
        }
        else
        {
            RFileIO::writeAscii(file,source.getRestart());
            RFileIO::writeAscii(file,source.getRadiationSetup());
            RFileIO::writeAscii(file,source.getModalSetup());
            RFileIO::writeAscii(file,source.getMeshSetup());
            RFileIO::writeAscii(file,source.getAcousticSetup());
            RFileIO::writeAscii(file,int(source.getMassMatrixType()));
            writeLegacyEmptyInteractionSetup(file,false);
            RFileIO::writeAscii(file,source.getStructuralDynamicsSetup());
            RFileIO::writeAscii(file,source.getParallelExecutionConfig());
            RFileIO::writeAscii(file,true);
            RFileIO::writeAscii(file,uint(2));
            RFileIO::writeAscii(file,0.02);
            RFileIO::writeAscii(file,0.01);
            RFileIO::writeAscii(file,uint(2));
            RFileIO::writeAscii(file,uint(1));
            RFileIO::writeAscii(file,uint(2));
            RFileIO::writeAscii(file,QString(64,QLatin1Char('a')));
            RFileIO::writeAscii(file,QString(64,QLatin1Char('b')));
            RFileIO::writeAscii(file,uint(1));
            for (uint i=0;i<4;i++) RFileIO::writeAscii(file,0.0);
            for (uint i=0;i<3;i++) RFileIO::writeAscii(file,0.0);
        }
        QVERIFY(file.commit());
    }
    RFile file(path,binary ? RFile::BINARY : RFile::ASCII);
    file.setVersion(RVersion(1,3,7));
    QVERIFY(file.open(QIODevice::ReadOnly));
    RProblemSetup restored;
    if (binary) RFileIO::readBinary(file,restored);
    else RFileIO::readAscii(file,restored);
    QVERIFY(restored.getStructuralDynamicsSetup().getEnabled());
    QVERIFY(!restored.getStructuralDynamicsRestartData().available);
}

void TestProblemSetup::readsVersion136WithoutCheckpoint_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.6") << false;
    QTest::newRow("binary-1.3.6") << true;
}

void TestProblemSetup::readsVersion136WithoutCheckpoint()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? "old-136.bin" : "old-136.txt");
    RProblemSetup source;
    source.getStructuralDynamicsSetup().setEnabled(true);
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
            writeLegacyEmptyInteractionSetup(file,true);
            RFileIO::writeBinary(file,source.getStructuralDynamicsSetup());
            RFileIO::writeBinary(file,source.getParallelExecutionConfig());
        }
        else
        {
            RFileIO::writeAscii(file,source.getRestart());
            RFileIO::writeAscii(file,source.getRadiationSetup());
            RFileIO::writeAscii(file,source.getModalSetup());
            RFileIO::writeAscii(file,source.getMeshSetup());
            RFileIO::writeAscii(file,source.getAcousticSetup());
            RFileIO::writeAscii(file,int(source.getMassMatrixType()));
            writeLegacyEmptyInteractionSetup(file,false);
            RFileIO::writeAscii(file,source.getStructuralDynamicsSetup());
            RFileIO::writeAscii(file,source.getParallelExecutionConfig());
        }
        QVERIFY(file.commit());
    }
    RFile file(path,binary ? RFile::BINARY : RFile::ASCII);
    file.setVersion(RVersion(1,3,6));
    QVERIFY(file.open(QIODevice::ReadOnly));
    RProblemSetup restored;
    restored.getStructuralDynamicsRestartData().available = true;
    if (binary) RFileIO::readBinary(file,restored);
    else RFileIO::readAscii(file,restored);
    QVERIFY(restored.getStructuralDynamicsSetup().getEnabled());
    QVERIFY(!restored.getStructuralDynamicsRestartData().available);
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

void TestProblemSetup::rotationalInitialConditions()
{
    QCOMPARE(int(R_INITIAL_CONDITION_ROTATION),int(R_INITIAL_CONDITION_CUSTOM)+1);
    QCOMPARE(RInitialCondition::getTypeFromId("ic-rotation"),R_INITIAL_CONDITION_ROTATION);
    QCOMPARE(RInitialCondition::getTypeFromId("ic-angular_velocity"),R_INITIAL_CONDITION_ANGULAR_VELOCITY);
    QCOMPARE(RInitialCondition::getDefaultComponents(R_INITIAL_CONDITION_ROTATION),
             (std::vector<RVariableType>{R_VARIABLE_ROTATION_X,R_VARIABLE_ROTATION_Y,R_VARIABLE_ROTATION_Z}));
    QCOMPARE(RInitialCondition::getDefaultComponents(R_INITIAL_CONDITION_ANGULAR_VELOCITY),
             (std::vector<RVariableType>{R_VARIABLE_ANGULAR_VELOCITY_X,R_VARIABLE_ANGULAR_VELOCITY_Y,
                                         R_VARIABLE_ANGULAR_VELOCITY_Z}));
    QCOMPARE(RInitialCondition::getProblemTypeMask(R_INITIAL_CONDITION_ROTATION) & R_PROBLEM_STRESS,
             RProblemTypeMask(R_PROBLEM_STRESS));
}

void TestProblemSetup::failedTargetWriteKeepsPreviousLink_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii") << false;
    QTest::newRow("binary") << true;
}

void TestProblemSetup::failedTargetWriteKeepsPreviousLink()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString linkPath = directory.filePath(binary ? "latest.rbm" : "latest.ram");
    RModel source;
    source.setName("accepted");
    source.getTimeSolver().setEnabled(true);
    source.getTimeSolver().setTimes({0.01,0.02});
    QCOMPARE(source.write(linkPath,true),linkPath);

    RModel first;
    first.read(linkPath);
    QCOMPARE(first.getName(),QString("accepted"));

    source.setName("uncommitted");
    source.getTimeSolver().setCurrentTimeStep(1);
    const QString blockedTarget = RFileUtils::getFileNameWithTimeStep(linkPath,2);
    QVERIFY(QDir().mkpath(blockedTarget));
    QVERIFY_EXCEPTION_THROWN(source.write(linkPath,true),RError);

    RModel afterFailure;
    afterFailure.read(linkPath);
    QCOMPARE(afterFailure.getName(),QString("accepted"));
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
