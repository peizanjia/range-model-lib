#include <QtTest>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <limits>

#include "rml_file_io.h"
#include "rml_model.h"
#include "rml_problem.h"
#include "rml_structural_product_profile.h"

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
    void defaultsAndCopy();
    void modelRoundTrip_data();
    void modelRoundTrip();
    void readsVersion1313WithoutPiezoSetup_data();
    void readsVersion1313WithoutPiezoSetup();
    void rejectsTruncatedAndInvalidBinaryWithoutMutation();
    void rejectsInvalidValuesAndTopology();
    void rejectsStaleMeshBinding();
    void electricChargeVariableContract();
};

void TestStructuralPiezoSetup::defaultsAndCopy()
{
    RProblemSetup setup;
    const RStructuralPiezoSetup &defaults = setup.getStructuralPiezoSetup();
    QVERIFY(!defaults.enabled);
    QVERIFY(defaults.meshFingerprint.isEmpty());
    QVERIFY(defaults.ports.empty());
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
    QCOMPARE(RModel::version.toString(),QStringLiteral("1.3.14"));
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

    model.getNode(1).setX(1.1);
    QVERIFY_EXCEPTION_THROWN(setup.validate(model),RError);
}

void TestStructuralPiezoSetup::electricChargeVariableContract()
{
    QCOMPARE(RVariable::getId(R_VARIABLE_ELECTRIC_CHARGE),QStringLiteral("var-electric_charge"));
    QCOMPARE(RVariable::getTypeFromId(QStringLiteral("var-electric_charge")),R_VARIABLE_ELECTRIC_CHARGE);
    QCOMPARE(RVariable::getName(R_VARIABLE_ELECTRIC_CHARGE),QStringLiteral("Right electrode charge"));
    QCOMPARE(RVariable::getUnits(R_VARIABLE_ELECTRIC_CHARGE),QStringLiteral("C"));
    QCOMPARE(RVariable::getDataType(R_VARIABLE_ELECTRIC_CHARGE),R_VARIABLE_DATA_DOUBLE);
    QCOMPARE(RVariable::getInitValue(R_VARIABLE_ELECTRIC_CHARGE),0.0);
    QVERIFY(RVariable::getResultsProblemTypeMask(R_VARIABLE_ELECTRIC_CHARGE) & R_PROBLEM_STRESS);
    QVERIFY(RStructuralProductProfile::isResultVariableEnabled(R_VARIABLE_ELECTRIC_CHARGE));

    const std::vector<RVariableType> stressVariableTypes = RProblem::getVariableTypes(R_PROBLEM_STRESS);
    QVERIFY(std::find(stressVariableTypes.begin(),stressVariableTypes.end(),R_VARIABLE_ELECTRIC_CHARGE) !=
            stressVariableTypes.end());
}

QTEST_APPLESS_MAIN(TestStructuralPiezoSetup)

#include "tst_structural_piezo_setup.moc"
