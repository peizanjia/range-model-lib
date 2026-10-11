#include <QtTest>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

#include "rml_file_io.h"
#include "rml_model.h"
#include "rml_problem.h"
#include "rml_structural_product_profile.h"
#include "rml_structural_dynamics_restart_data.h"

static RModel makePiezoSetupModel(RElementType type = R_ELEMENT_TRUSS1)
{
    RModel model;
    model.addNode(RNode(0.0,0.0,0.0));
    model.addNode(RNode(1.0,0.0,0.0));
    RElement element(type);
    element.setNodeId(0,0);
    element.setNodeId(1,1);
    model.addElement(element);
    return model;
}

static RModel makePiezoNetworkModel()
{
    RModel model;
    model.addNode(RNode(0.0,0.0,0.0));
    model.addNode(RNode(1.000000000000009,0.0,0.0));
    model.addNode(RNode(2.23456789012345,0.0,0.0));
    for (uint i = 0; i < 2; ++i)
    {
        RElement element(R_ELEMENT_TRUSS1);
        element.setNodeId(0,i);
        element.setNodeId(1,i+1);
        model.addElement(element);
    }
    return model;
}

static RStructuralPiezoPort makePiezoPort(const QString &id = QStringLiteral("port-1"),
                                          uint elementId = 0)
{
    RStructuralPiezoPort port;
    port.id = id;
    port.elementId = elementId;
    port.coupling = -1.2345678901234567;
    port.permittivity = 8.854187812800123e-12;
    port.voltage = 120.12345678901234;
    return port;
}

class TestStructuralPiezoSetup : public QObject
{
    Q_OBJECT

private slots:
    void mixedClassificationAndBinding();
    void mixedPersistence_data();
    void mixedPersistence();
    void defaultsAndCopy();
    void modelRoundTrip_data();
    void modelRoundTrip();
    void nodalPotentialModelRoundTrip_data();
    void nodalPotentialModelRoundTrip();
    void readsVersion1314AsLegacy_data();
    void readsVersion1314AsLegacy();
    void readsVersion1313WithoutPiezoSetup_data();
    void readsVersion1313WithoutPiezoSetup();
    void rejectsTruncatedAndInvalidBinaryWithoutMutation();
    void rejectsInvalidValuesAndTopology();
    void rejectsStaleMeshBinding();
    void validatesNodalNetworkAndExactBinding();
    void rejectsNodalWriteToLegacyVersion();
    void electricChargeVariableContract();
};

void TestStructuralPiezoSetup::defaultsAndCopy()
{
    RProblemSetup setup;
    const RStructuralPiezoSetup &defaults = setup.getStructuralPiezoSetup();
    QVERIFY(!defaults.enabled);
    QVERIFY(defaults.meshFingerprint.isEmpty());
    QVERIFY(defaults.ports.empty());
    QVERIFY(defaults.passiveElementIds.empty());
    defaults.validate();

    RModel model = makePiezoSetupModel();
    RStructuralPiezoSetup piezo;
    piezo.enabled = true;
    piezo.ports = {makePiezoPort(QStringLiteral("port 1"))};
    piezo.bindToModel(model);
    setup.setStructuralPiezoSetup(piezo);

    const RProblemSetup copied(setup);
    QCOMPARE(copied.getStructuralPiezoSetup().enabled,true);
    QCOMPARE(copied.getStructuralPiezoSetup().meshFingerprint,piezo.meshFingerprint);
    QCOMPARE(copied.getStructuralPiezoSetup().ports.size(),size_t(1));
    QCOMPARE(copied.getStructuralPiezoSetup().ports[0].id,piezo.ports[0].id);
    QCOMPARE(copied.getStructuralPiezoSetup().ports[0].coupling,piezo.ports[0].coupling);
    QCOMPARE(copied.getStructuralPiezoSetup().ports[0].permittivity,piezo.ports[0].permittivity);
    QCOMPARE(copied.getStructuralPiezoSetup().ports[0].voltage,piezo.ports[0].voltage);

    RProblemSetup assigned;
    assigned = setup;
    QCOMPARE(assigned.getStructuralPiezoSetup().meshFingerprint,piezo.meshFingerprint);
    QCOMPARE(assigned.getStructuralPiezoSetup().ports[0].elementId,uint(0));
}

void TestStructuralPiezoSetup::modelRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii") << false;
    QTest::newRow("binary") << true;
}

void TestStructuralPiezoSetup::modelRoundTrip()
{
    QFETCH(bool,binary);
    QCOMPARE(RModel::version.toString(),QStringLiteral("1.3.20"));
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RModel source = makePiezoSetupModel();
    RStructuralPiezoSetup piezo;
    piezo.enabled = true;
    piezo.ports = {makePiezoPort(QStringLiteral("right electrode 1"))};
    piezo.bindToModel(source);
    source.getProblemSetup().setStructuralPiezoSetup(piezo);

    const uint variablePosition = source.addVariable(RVariable(R_VARIABLE_ELECTRIC_CHARGE));
    RVariable &charge = source.getVariable(variablePosition);
    charge.setApplyType(R_VARIABLE_APPLY_ELEMENT);
    charge.resize(1,source.getNElements());
    charge.setValue(0,0,-2.345678901234567);

    const QString extension = RModel::getDefaultFileExtension(binary);
    const QString path = directory.filePath(QStringLiteral("piezo-setup.") + extension);
    RModel restored;
    QCOMPARE(source.write(path,false),path);
    restored.read(path);

    const RStructuralPiezoSetup &actual = restored.getProblemSetup().getStructuralPiezoSetup();
    QCOMPARE(actual.enabled,true);
    QCOMPARE(actual.meshFingerprint,piezo.meshFingerprint);
    QCOMPARE(actual.ports.size(),size_t(1));
    QCOMPARE(actual.ports[0].id,piezo.ports[0].id);
    QCOMPARE(actual.ports[0].elementId,uint(0));
    QCOMPARE(actual.ports[0].coupling,piezo.ports[0].coupling);
    QCOMPARE(actual.ports[0].permittivity,piezo.ports[0].permittivity);
    QCOMPARE(actual.ports[0].voltage,piezo.ports[0].voltage);
    actual.validate(restored);

    const uint restoredVariablePosition = restored.findVariable(R_VARIABLE_ELECTRIC_CHARGE);
    QVERIFY(restoredVariablePosition < restored.getNVariables());
    const RVariable &restoredCharge = restored.getVariable(restoredVariablePosition);
    QCOMPARE(restoredCharge.getApplyType(),R_VARIABLE_APPLY_ELEMENT);
    QCOMPARE(restoredCharge.getNValues(),uint(1));
    if (binary)
    {
        QCOMPARE(restoredCharge.getValue(0),-2.345678901234567);
    }
    else
    {
        // Generic result vectors keep the existing default ASCII precision;
        // setup scalars above use their dedicated 17-digit writer.
        QVERIFY(std::abs(restoredCharge.getValue(0)+2.345678901234567) < 2.0e-6);
    }
}

void TestStructuralPiezoSetup::nodalPotentialModelRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-nodal-potential") << false;
    QTest::newRow("binary-nodal-potential") << true;
}

void TestStructuralPiezoSetup::nodalPotentialModelRoundTrip()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RModel source = makePiezoNetworkModel();
    RStructuralPiezoSetup piezo;
    piezo.enabled = true;
    piezo.electricalMode = RStructuralPiezoElectricalMode::NodalPotential;
    piezo.ports = {makePiezoPort(QStringLiteral("left segment"),0),
                   makePiezoPort(QStringLiteral("right segment"),1)};
    for (RStructuralPiezoPort &port : piezo.ports) port.voltage = 0.0;
    piezo.nodalBoundaryConditions = {
        {0,RStructuralPiezoBoundaryType::Potential,17.123456789012345},
        {2,RStructuralPiezoBoundaryType::ConductorCharge,-2.345678901234567e-6}
    };
    piezo.bindToModel(source);
    source.getProblemSetup().setStructuralPiezoSetup(piezo);

    const QString extension = RModel::getDefaultFileExtension(binary);
    const QString path = directory.filePath(QStringLiteral("piezo-nodal.") + extension);
    RModel restored;
    QCOMPARE(source.write(path,false),path);
    restored.read(path);

    const RStructuralPiezoSetup &actual = restored.getProblemSetup().getStructuralPiezoSetup();
    QCOMPARE(actual.enabled,true);
    QCOMPARE(int(actual.electricalMode),int(RStructuralPiezoElectricalMode::NodalPotential));
    QCOMPARE(actual.meshFingerprint,piezo.meshFingerprint);
    QCOMPARE(actual.ports.size(),size_t(2));
    QCOMPARE(actual.ports[0].coupling,piezo.ports[0].coupling);
    QCOMPARE(actual.ports[1].permittivity,piezo.ports[1].permittivity);
    QCOMPARE(actual.nodalBoundaryConditions.size(),size_t(2));
    QCOMPARE(actual.nodalBoundaryConditions[0].nodeId,uint(0));
    QCOMPARE(int(actual.nodalBoundaryConditions[0].type),int(RStructuralPiezoBoundaryType::Potential));
    QCOMPARE(actual.nodalBoundaryConditions[0].value,piezo.nodalBoundaryConditions[0].value);
    QCOMPARE(actual.nodalBoundaryConditions[1].nodeId,uint(2));
    QCOMPARE(int(actual.nodalBoundaryConditions[1].type),int(RStructuralPiezoBoundaryType::ConductorCharge));
    QCOMPARE(actual.nodalBoundaryConditions[1].value,piezo.nodalBoundaryConditions[1].value);
    actual.validate(restored);
}

void TestStructuralPiezoSetup::readsVersion1314AsLegacy_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.14") << false;
    QTest::newRow("binary-1.3.14") << true;
}

void TestStructuralPiezoSetup::readsVersion1314AsLegacy()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? QStringLiteral("legacy-1.3.14.bin")
                                                   : QStringLiteral("legacy-1.3.14.txt"));

    RModel model = makePiezoSetupModel();
    RStructuralPiezoSetup source;
    source.enabled = true;
    source.ports = {makePiezoPort(QStringLiteral("legacy terminal"))};
    source.bindToModel(model);
    {
        RSaveFile out(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        out.setVersion(RVersion(1,3,14));
        QVERIFY(out.open(QIODevice::WriteOnly));
        if (binary) RFileIO::writeBinary(out,source);
        else RFileIO::writeAscii(out,source);
        QVERIFY(out.commit());
    }

    RFile in(path,binary ? RFile::BINARY : RFile::ASCII);
    in.setVersion(RVersion(1,3,14));
    QVERIFY(in.open(QIODevice::ReadOnly));
    RStructuralPiezoSetup restored;
    if (binary) RFileIO::readBinary(in,restored);
    else RFileIO::readAscii(in,restored);
    QCOMPARE(int(restored.electricalMode),int(RStructuralPiezoElectricalMode::LegacyTerminalVoltage));
    QVERIFY(restored.nodalBoundaryConditions.empty());
    QCOMPARE(restored.ports.size(),size_t(1));
    QCOMPARE(restored.ports[0].voltage,source.ports[0].voltage);
}

void TestStructuralPiezoSetup::readsVersion1313WithoutPiezoSetup_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.13") << false;
    QTest::newRow("binary-1.3.13") << true;
}

void TestStructuralPiezoSetup::readsVersion1313WithoutPiezoSetup()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? QStringLiteral("legacy.bin")
                                                   : QStringLiteral("legacy.txt"));

    RProblemSetup source;
    {
        RSaveFile out(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        QVERIFY(out.open(QIODevice::WriteOnly));
        auto write = [&](const auto &value)
        {
            if (binary) RFileIO::writeBinary(out,value);
            else RFileIO::writeAscii(out,value);
        };
        write(source.getRestart());
        write(source.getRadiationSetup());
        write(source.getModalSetup());
        write(source.getMeshSetup());
        write(source.getAcousticSetup());
        write(int(source.getMassMatrixType()));
        write(source.getStructuralInteractionSetup());
        write(source.getStructuralDynamicsSetup());
        write(source.getParallelExecutionConfig());
        write(source.getStructuralDynamicsRestartData());
        write(source.getStructuralControlSetup());
        write(source.getStructuralCableSetup());
        QVERIFY(out.commit());
    }

    RFile in(path,binary ? RFile::BINARY : RFile::ASCII);
    in.setVersion(RVersion(1,3,13));
    QVERIFY(in.open(QIODevice::ReadOnly));
    RProblemSetup restored;
    RStructuralPiezoSetup &target = restored.getStructuralPiezoSetup();
    target.enabled = true;
    target.meshFingerprint = QString(64,QLatin1Char('a'));
    target.ports = {makePiezoPort()};
    if (binary) RFileIO::readBinary(in,restored);
    else RFileIO::readAscii(in,restored);

    const RStructuralPiezoSetup &actual = restored.getStructuralPiezoSetup();
    QVERIFY(!actual.enabled);
    QVERIFY(actual.meshFingerprint.isEmpty());
    QVERIFY(actual.ports.empty());
}

void TestStructuralPiezoSetup::rejectsTruncatedAndInvalidBinaryWithoutMutation()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RStructuralPiezoSetup target;
    target.enabled = true;
    target.meshFingerprint = QString(64,QLatin1Char('b'));
    target.ports = {makePiezoPort(QStringLiteral("kept"),7)};

    const QString emptyPath = directory.filePath(QStringLiteral("empty-tail.bin"));
    {
        RSaveFile out(emptyPath,RSaveFile::BINARY);
        QVERIFY(out.open(QIODevice::WriteOnly));
        QVERIFY(out.commit());
    }
    {
        RFile in(emptyPath,RFile::BINARY);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,target),RError);
    }
    QVERIFY(target.enabled);
    QCOMPARE(target.meshFingerprint,QString(64,QLatin1Char('b')));
    QCOMPARE(target.ports.size(),size_t(1));
    QCOMPARE(target.ports[0].id,QStringLiteral("kept"));
    QCOMPARE(target.ports[0].elementId,uint(7));

    const QString cutPath = directory.filePath(QStringLiteral("before-port-voltage.bin"));
    {
        RSaveFile out(cutPath,RSaveFile::BINARY);
        QVERIFY(out.open(QIODevice::WriteOnly));
        RFileIO::writeBinary(out,true);
        RFileIO::writeBinary(out,QString(64,QLatin1Char('a')));
        RFileIO::writeBinary(out,uint(1));
        RFileIO::writeBinary(out,QStringLiteral("port-cut"));
        RFileIO::writeBinary(out,uint(0));
        RFileIO::writeBinary(out,-2.0);
        RFileIO::writeBinary(out,8.85e-12);
        QVERIFY(out.commit());
    }
    {
        RFile in(cutPath,RFile::BINARY);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,target),RError);
    }
    QVERIFY(target.enabled);
    QCOMPARE(target.meshFingerprint,QString(64,QLatin1Char('b')));
    QCOMPARE(target.ports.size(),size_t(1));
    QCOMPARE(target.ports[0].id,QStringLiteral("kept"));
    QCOMPARE(target.ports[0].voltage,makePiezoPort(QStringLiteral("kept"),7).voltage);

    const QString invalidBoolPath = directory.filePath(QStringLiteral("invalid-bool.bin"));
    {
        RSaveFile out(invalidBoolPath,RSaveFile::BINARY);
        QVERIFY(out.open(QIODevice::WriteOnly));
        const unsigned char invalidBool = 2;
        QCOMPARE(out.write(reinterpret_cast<const char*>(&invalidBool),qint64(sizeof(invalidBool))),
                 qint64(sizeof(invalidBool)));
        QVERIFY(out.commit());
    }
    {
        RFile in(invalidBoolPath,RFile::BINARY);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,target),RError);
    }
    QVERIFY(target.enabled);
    QCOMPARE(target.meshFingerprint,QString(64,QLatin1Char('b')));
    QCOMPARE(target.ports[0].id,QStringLiteral("kept"));

    const QString invalidAsciiBoolPath = directory.filePath(QStringLiteral("invalid-bool.txt"));
    {
        RSaveFile out(invalidAsciiBoolPath,RSaveFile::ASCII);
        QVERIFY(out.open(QIODevice::WriteOnly));
        RFileIO::writeAscii(out,'2',false);
        QVERIFY(out.commit());
    }
    {
        RFile in(invalidAsciiBoolPath,RFile::ASCII);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(in,target),RError);
    }
    QVERIFY(target.enabled);
    QCOMPARE(target.meshFingerprint,QString(64,QLatin1Char('b')));
    QCOMPARE(target.ports[0].id,QStringLiteral("kept"));

    const QString invalidAsciiNumberPath = directory.filePath(QStringLiteral("invalid-number.txt"));
    {
        RSaveFile out(invalidAsciiNumberPath,RSaveFile::ASCII);
        QVERIFY(out.open(QIODevice::WriteOnly));
        RFileIO::writeAscii(out,true);
        RFileIO::writeAscii(out,QString(64,QLatin1Char('a')));
        RFileIO::writeAscii(out,uint(1));
        RFileIO::writeAscii(out,QStringLiteral("port"));
        RFileIO::writeAscii(out,uint(0));
        RFileIO::writeAscii(out,QStringLiteral("not-a-number"));
        QVERIFY(out.commit());
    }
    {
        RFile in(invalidAsciiNumberPath,RFile::ASCII);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(in,target),RError);
    }
    QVERIFY(target.enabled);
    QCOMPARE(target.meshFingerprint,QString(64,QLatin1Char('b')));
    QCOMPARE(target.ports[0].id,QStringLiteral("kept"));

    const QString leadingWhitespaceIdPath = directory.filePath(QStringLiteral("leading-port-id.txt"));
    {
        RSaveFile out(leadingWhitespaceIdPath,RSaveFile::ASCII);
        QVERIFY(out.open(QIODevice::WriteOnly));
        RFileIO::writeAscii(out,true);
        RFileIO::writeAscii(out,QString(64,QLatin1Char('a')));
        RFileIO::writeAscii(out,uint(1));
        RFileIO::writeAscii(out,QStringLiteral("\" port\""));
        RFileIO::writeAscii(out,uint(0));
        RFileIO::writeAscii(out,-1.0);
        RFileIO::writeAscii(out,8.85e-12);
        RFileIO::writeAscii(out,1.0);
        QVERIFY(out.commit());
    }
    {
        RFile in(leadingWhitespaceIdPath,RFile::ASCII);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(in,target),RError);
    }
    QVERIFY(target.enabled);
    QCOMPARE(target.meshFingerprint,QString(64,QLatin1Char('b')));
    QCOMPARE(target.ports[0].id,QStringLiteral("kept"));
}

void TestStructuralPiezoSetup::rejectsInvalidValuesAndTopology()
{
    RStructuralPiezoSetup setup;
    setup.enabled = true;
    setup.ports = {makePiezoPort()};
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError); // Missing fingerprint.
    setup.meshFingerprint = QString(64,QLatin1Char('a'));

    setup.ports.push_back(makePiezoPort(QStringLiteral("port-2"),0));
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError); // Duplicate element ID.
    setup.ports[1] = makePiezoPort(QStringLiteral("port-1"),1);
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError); // Duplicate port ID.
    setup.ports.resize(1);
    setup.ports[0].id = QStringLiteral(" ");
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.ports[0].id = QStringLiteral(" leading");
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.ports[0].id = QStringLiteral("trailing ");
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.ports[0] = makePiezoPort();

    setup.ports[0].coupling = std::numeric_limits<double>::infinity();
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.ports[0].coupling = 0.0;
    setup.ports[0].permittivity = 0.0;
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.ports[0].permittivity = 1.0;
    setup.ports[0].voltage = std::numeric_limits<double>::quiet_NaN();
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);

    RModel trussModel = makePiezoSetupModel();
    setup = RStructuralPiezoSetup();
    setup.enabled = true;
    setup.ports = {makePiezoPort(QStringLiteral("unknown"),1)};
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(trussModel),RError); // Unknown element.
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel beamModel = makePiezoSetupModel(R_ELEMENT_BEAM1);
    setup.ports = {makePiezoPort()};
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(beamModel),RError); // Non-TRUSS1 topology.
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel outOfRangeNode = makePiezoSetupModel();
    outOfRangeNode.getElement(0).setNodeId(1,9);
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(outOfRangeNode),RError);
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel repeatedNode = makePiezoSetupModel();
    repeatedNode.getElement(0).setNodeId(1,0);
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(repeatedNode),RError);
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel zeroLength = makePiezoSetupModel();
    zeroLength.getNode(1).set(0.0,0.0,0.0);
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(zeroLength),RError);
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel nonFiniteNode = makePiezoSetupModel();
    nonFiniteNode.getNode(1).setX(std::numeric_limits<double>::infinity());
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(nonFiniteNode),RError);
    QVERIFY(setup.meshFingerprint.isEmpty());
}

void TestStructuralPiezoSetup::rejectsStaleMeshBinding()
{
    RModel model = makePiezoSetupModel();
    RStructuralPiezoSetup setup;
    setup.enabled = true;
    setup.ports = {makePiezoPort()};
    setup.bindToModel(model);
    setup.validate(model);

    model.getNode(1).setX(1.00000001);
    setup.validate(model); // Legacy binding keeps its historical five-digit tolerance.

    model.getNode(1).setX(1.1);
    QVERIFY_EXCEPTION_THROWN(setup.validate(model),RError);
}

void TestStructuralPiezoSetup::validatesNodalNetworkAndExactBinding()
{
    RModel model = makePiezoNetworkModel();
    RStructuralPiezoSetup setup;
    setup.enabled = true;
    setup.electricalMode = RStructuralPiezoElectricalMode::NodalPotential;
    setup.ports = {makePiezoPort(QStringLiteral("segment-1"),0),
                   makePiezoPort(QStringLiteral("segment-2"),1)};
    for (RStructuralPiezoPort &port : setup.ports) port.voltage = 0.0;
    setup.nodalBoundaryConditions = {
        {0,RStructuralPiezoBoundaryType::Potential,0.0},
        {2,RStructuralPiezoBoundaryType::ConductorCharge,1.5e-6}
    };
    setup.bindToModel(model);
    setup.validate(model);

    RModel perturbed = model;
    perturbed.getNode(1).setX(1.0000000001);
    QVERIFY_EXCEPTION_THROWN(setup.validate(perturbed),RError);

    RModel reversed = model;
    reversed.getElement(1).setNodeId(0,2);
    reversed.getElement(1).setNodeId(1,1);
    QVERIFY_EXCEPTION_THROWN(setup.validate(reversed),RError);

    RStructuralPiezoSetup changedMaterial = setup;
    changedMaterial.ports[0].coupling *= 1.01;
    QVERIFY_EXCEPTION_THROWN(changedMaterial.validate(model),RError);
    changedMaterial = setup;
    changedMaterial.ports[0].permittivity *= 1.01;
    QVERIFY_EXCEPTION_THROWN(changedMaterial.validate(model),RError);

    RStructuralPiezoSetup duplicateBoundary = setup;
    duplicateBoundary.nodalBoundaryConditions.push_back(
        {0,RStructuralPiezoBoundaryType::ConductorCharge,0.0});
    QVERIFY_EXCEPTION_THROWN(duplicateBoundary.validate(),RError);

    RStructuralPiezoSetup mixedDrive = setup;
    mixedDrive.ports[0].voltage = 5.0;
    QVERIFY_EXCEPTION_THROWN(mixedDrive.validate(),RError);

    RModel disconnected;
    disconnected.addNode(RNode(0.0,0.0,0.0));
    disconnected.addNode(RNode(1.0,0.0,0.0));
    disconnected.addNode(RNode(0.0,1.0,0.0));
    disconnected.addNode(RNode(1.0,1.0,0.0));
    for (const auto endpoints : {std::pair<uint,uint>{0,1},std::pair<uint,uint>{2,3}})
    {
        RElement element(R_ELEMENT_TRUSS1);
        element.setNodeId(0,endpoints.first);
        element.setNodeId(1,endpoints.second);
        disconnected.addElement(element);
    }
    RStructuralPiezoSetup missingComponentGauge;
    missingComponentGauge.enabled = true;
    missingComponentGauge.electricalMode = RStructuralPiezoElectricalMode::NodalPotential;
    missingComponentGauge.ports = {makePiezoPort(QStringLiteral("component-1"),0),
                                   makePiezoPort(QStringLiteral("component-2"),1)};
    for (RStructuralPiezoPort &port : missingComponentGauge.ports) port.voltage = 0.0;
    missingComponentGauge.nodalBoundaryConditions = {
        {0,RStructuralPiezoBoundaryType::Potential,0.0},
        {3,RStructuralPiezoBoundaryType::ConductorCharge,0.0}
    };
    QVERIFY_EXCEPTION_THROWN(missingComponentGauge.bindToModel(disconnected),RError);
    QVERIFY(missingComponentGauge.meshFingerprint.isEmpty());
    missingComponentGauge.nodalBoundaryConditions.push_back(
        {2,RStructuralPiezoBoundaryType::Potential,0.0});
    missingComponentGauge.bindToModel(disconnected);
    missingComponentGauge.validate(disconnected);
}

void TestStructuralPiezoSetup::rejectsNodalWriteToLegacyVersion()
{
    RModel model = makePiezoNetworkModel();
    RStructuralPiezoSetup setup;
    setup.enabled = true;
    setup.electricalMode = RStructuralPiezoElectricalMode::NodalPotential;
    setup.ports = {makePiezoPort(QStringLiteral("port-1"),0),
                   makePiezoPort(QStringLiteral("port-2"),1)};
    for (RStructuralPiezoPort &port : setup.ports) port.voltage = 0.0;
    setup.nodalBoundaryConditions = {{0,RStructuralPiezoBoundaryType::Potential,0.0}};
    setup.bindToModel(model);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString asciiPath = directory.filePath(QStringLiteral("lossy.txt"));
    {
        RSaveFile out(asciiPath,RSaveFile::ASCII);
        out.setVersion(RVersion(1,3,14));
        QVERIFY(out.open(QIODevice::WriteOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::writeAscii(out,setup),RError);
        out.cancelWriting();
    }

    const QString binaryPath = directory.filePath(QStringLiteral("lossy.bin"));
    {
        RSaveFile out(binaryPath,RSaveFile::BINARY);
        out.setVersion(RVersion(1,3,14));
        QVERIFY(out.open(QIODevice::WriteOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::writeBinary(out,setup),RError);
        out.cancelWriting();
    }
}

void TestStructuralPiezoSetup::electricChargeVariableContract()
{
    QCOMPARE(RVariable::getId(R_VARIABLE_ELECTRIC_CHARGE),QStringLiteral("var-electric_charge"));
    QCOMPARE(RVariable::getTypeFromId(QStringLiteral("var-electric_charge")),R_VARIABLE_ELECTRIC_CHARGE);
    QCOMPARE(RVariable::getName(R_VARIABLE_ELECTRIC_CHARGE),QStringLiteral("Electric conductor charge"));
    QCOMPARE(RVariable::getUnits(R_VARIABLE_ELECTRIC_CHARGE),QStringLiteral("C"));
    QCOMPARE(RVariable::getDataType(R_VARIABLE_ELECTRIC_CHARGE),R_VARIABLE_DATA_DOUBLE);
    QCOMPARE(RVariable::getInitValue(R_VARIABLE_ELECTRIC_CHARGE),0.0);
    QVERIFY(RVariable::getResultsProblemTypeMask(R_VARIABLE_ELECTRIC_CHARGE) & R_PROBLEM_STRESS);
    QVERIFY(RStructuralProductProfile::isResultVariableEnabled(R_VARIABLE_ELECTRIC_CHARGE));

    const std::vector<RVariableType> stressVariableTypes = RProblem::getVariableTypes(R_PROBLEM_STRESS);
    QVERIFY(std::find(stressVariableTypes.begin(),stressVariableTypes.end(),R_VARIABLE_ELECTRIC_CHARGE) !=
            stressVariableTypes.end());
}

void TestStructuralPiezoSetup::mixedClassificationAndBinding()
{
    RModel model = makePiezoNetworkModel();
    model.getNode(1).set(1.0,2.0,3.0);
    RElement point(R_ELEMENT_POINT);
    point.setNodeId(0,0);
    model.addElement(point);
    RStructuralPiezoSetup setup;
    setup.enabled = true;
    setup.ports = {makePiezoPort()};
    setup.passiveElementIds = {1};
    setup.bindToModel(model);
    setup.validate(model);
    const QString fingerprint = setup.meshFingerprint;
    QVERIFY(fingerprint != RStructuralDynamicsRestartData::computeMeshFingerprint(model));
    RStructuralPiezoSetup swapped = setup;
    swapped.ports[0].elementId = 1;
    swapped.passiveElementIds = {0};
    QVERIFY_EXCEPTION_THROWN(swapped.validate(model),RError);
    swapped.bindToModel(model);
    QVERIFY(swapped.meshFingerprint != fingerprint);

    RStructuralPiezoSetup invalid = setup;
    invalid.passiveElementIds = {1,1};
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    invalid.passiveElementIds = {0};
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    invalid.passiveElementIds = {9};
    QVERIFY_EXCEPTION_THROWN(invalid.bindToModel(model),RError);
    QCOMPARE(invalid.meshFingerprint,fingerprint);
    invalid.passiveElementIds = {2}; // POINT cannot be a passive rod.
    QVERIFY_EXCEPTION_THROWN(invalid.bindToModel(model),RError);
    RModel missing = model;
    RElement extra(R_ELEMENT_TRUSS1);
    extra.setNodeId(0,0);
    extra.setNodeId(1,2);
    missing.addElement(extra);
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(missing),RError);
    QCOMPARE(setup.meshFingerprint,fingerprint);
    invalid = setup;
    invalid.enabled = false;
    invalid.passiveElementIds = {1,1};
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    invalid = setup;
    invalid.passiveElementIds.resize(1000001u,1);
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    RModel nonFinite = model;
    nonFinite.getNode(2).setZ(std::numeric_limits<double>::infinity());
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(nonFinite),RError);
    RProblemSetup stored;
    stored.setStructuralPiezoSetup(setup);
    const RProblemSetup copied(stored);
    QCOMPARE(copied.getStructuralPiezoSetup().passiveElementIds.size(),size_t(1));
    QCOMPARE(copied.getStructuralPiezoSetup().passiveElementIds[0],uint(1));
    RModel degenerate = model;
    degenerate.getElement(1).setNodeId(1,1);
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(degenerate),RError);

    invalid = setup;
    invalid.electricalMode = RStructuralPiezoElectricalMode::NodalPotential;
    invalid.ports[0].voltage = 0.0;
    invalid.nodalBoundaryConditions = {{0,RStructuralPiezoBoundaryType::Potential,0.0}};
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);
    invalid.enabled = false;
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError); // Retained disabled data is checked too.
    invalid = setup;
    invalid.ports.clear();
    QVERIFY_EXCEPTION_THROWN(invalid.validate(),RError);

    RStructuralPiezoSetup legacy;
    legacy.enabled = true;
    legacy.ports = {makePiezoPort(),makePiezoPort(QStringLiteral("second"),1)};
    legacy.bindToModel(model);
    QCOMPARE(legacy.meshFingerprint,RStructuralDynamicsRestartData::computeMeshFingerprint(model));
    setup.meshFingerprint = legacy.meshFingerprint;
    QVERIFY_EXCEPTION_THROWN(setup.validate(model),RError);
}

void TestStructuralPiezoSetup::mixedPersistence_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii") << false;
    QTest::newRow("binary") << true;
}

void TestStructuralPiezoSetup::mixedPersistence()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    RModel source = makePiezoNetworkModel();
    source.getNode(1).set(1.0,2.0,3.0);
    RStructuralPiezoSetup setup;
    setup.enabled = true;
    setup.ports = {makePiezoPort()};
    setup.passiveElementIds = {1};
    setup.bindToModel(source);
    source.getProblemSetup().setStructuralPiezoSetup(setup);
    const QString modelPath = directory.filePath(QStringLiteral("mixed.")+RModel::getDefaultFileExtension(binary));
    source.write(modelPath,false);
    RModel restored;
    restored.read(modelPath);
    const auto &actual = restored.getProblemSetup().getStructuralPiezoSetup();
    QCOMPARE(actual.passiveElementIds.size(),size_t(1));
    QCOMPARE(actual.passiveElementIds[0],uint(1));
    QCOMPARE(actual.ports[0].voltage,setup.ports[0].voltage);
    QCOMPARE(actual.meshFingerprint,setup.meshFingerprint);
    actual.validate(restored);

    const QString path = directory.filePath(QStringLiteral("setup"));
    for (const int release : {14,18})
    {
        RSaveFile output(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        output.setVersion(RVersion(1,3,release));
        QVERIFY(output.open(QIODevice::WriteOnly));
        if (binary) QVERIFY_EXCEPTION_THROWN(RFileIO::writeBinary(output,setup),RError);
        else QVERIFY_EXCEPTION_THROWN(RFileIO::writeAscii(output,setup),RError);
        output.cancelWriting();
    }
    RStructuralPiezoSetup legacy = setup;
    legacy.passiveElementIds.clear();
    legacy.bindToModel(source);
    {
        RSaveFile output(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        output.setVersion(RVersion(1,3,18));
        QVERIFY(output.open(QIODevice::WriteOnly));
        if (binary) RFileIO::writeBinary(output,legacy);
        else RFileIO::writeAscii(output,legacy);
        QVERIFY(output.commit());
    }
    {
        RFile input(path,binary ? RFile::BINARY : RFile::ASCII);
        input.setVersion(RVersion(1,3,18));
        QVERIFY(input.open(QIODevice::ReadOnly));
        RStructuralPiezoSetup target = setup;
        if (binary) RFileIO::readBinary(input,target);
        else RFileIO::readAscii(input,target);
        QVERIFY(target.passiveElementIds.empty());
        QCOMPARE(target.meshFingerprint,legacy.meshFingerprint);
    }
    // A valid pre-19 prefix followed by malformed new tail must never replace target.
    QFile prefixFile(path);
    QVERIFY(prefixFile.open(QIODevice::ReadOnly));
    const QByteArray prefix = prefixFile.readAll();
    prefixFile.close();
    std::vector<QByteArray> tails;
    if (binary)
    {
        for (const uint count : {1u,1000001u})
            tails.emplace_back(reinterpret_cast<const char*>(&count),sizeof(count));
        const uint count = 2, elementId = 1;
        QByteArray duplicate(reinterpret_cast<const char*>(&count),sizeof(count));
        duplicate.append(reinterpret_cast<const char*>(&elementId),sizeof(elementId));
        duplicate.append(reinterpret_cast<const char*>(&elementId),sizeof(elementId));
        tails.push_back(duplicate);
        tails.push_back(QByteArray(1,char(1))); // Partial count.
    }
    else
    {
        tails = {QByteArray("\n1\n"),QByteArray("\n1000001\n"),QByteArray("\n2\n1\n1\n"),
                 QByteArray("\n1\n-1\n"),QByteArray("\n1\ninvalid\n")};
    }
    tails.push_back(QByteArray()); // Missing tail count.
    for (const QByteArray &tail : tails)
    {
        QFile corrupt(path);
        QVERIFY(corrupt.open(QIODevice::WriteOnly));
        QCOMPARE(corrupt.write(prefix+tail),qint64(prefix.size()+tail.size()));
        corrupt.close();
        RFile input(path,binary ? RFile::BINARY : RFile::ASCII);
        input.setVersion(RVersion(1,3,19));
        QVERIFY(input.open(QIODevice::ReadOnly));
        RStructuralPiezoSetup target = setup;
        if (binary) QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(input,target),RError);
        else QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(input,target),RError);
        QCOMPARE(target.passiveElementIds.size(),size_t(1));
        QCOMPARE(target.passiveElementIds[0],uint(1));
        QCOMPARE(target.meshFingerprint,setup.meshFingerprint);
        QCOMPARE(target.ports[0].id,setup.ports[0].id);
    }
}

QTEST_APPLESS_MAIN(TestStructuralPiezoSetup)

#include "tst_structural_piezo_setup.moc"
