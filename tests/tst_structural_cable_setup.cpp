#include <QtTest>
#include <QTemporaryDir>

#include <limits>

#include "rml_file_io.h"
#include "rml_model.h"

static RModel makeCableSetupModel(RElementType type = R_ELEMENT_TRUSS1)
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

class TestStructuralCableSetup : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndCopy();
    void problemSetupRoundTrip_data();
    void problemSetupRoundTrip();
    void readsVersion1312WithoutCableSetup_data();
    void readsVersion1312WithoutCableSetup();
    void rejectsTruncatedBinarySetupWithoutMutation();
    void rejectsInvalidValuesAndTopology();
    void rejectsStaleMeshBinding();
};

void TestStructuralCableSetup::defaultsAndCopy()
{
    RProblemSetup setup;
    const RStructuralCableSetup &defaults = setup.getStructuralCableSetup();
    QVERIFY(!defaults.enabled);
    QVERIFY(defaults.meshFingerprint.isEmpty());
    QVERIFY(defaults.members.empty());
    QCOMPARE(defaults.maxIterations,uint(50));
    QCOMPARE(defaults.maxLineSearchIterations,uint(24));
    QCOMPARE(defaults.absoluteForceTolerance,1.0e-8);
    QCOMPARE(defaults.relativeForceTolerance,1.0e-8);
    defaults.validate();

    RModel model = makeCableSetupModel();
    RStructuralCableSetup cable;
    cable.enabled = true;
    cable.members.push_back({0,1.2345678901234567,123.45678901234567});
    cable.maxIterations = 83;
    cable.absoluteForceTolerance = 2.345678901234567e-11;
    cable.bindToModel(model);
    setup.setStructuralCableSetup(cable);

    const RProblemSetup copied(setup);
    QCOMPARE(copied.getStructuralCableSetup().meshFingerprint,cable.meshFingerprint);
    QCOMPARE(copied.getStructuralCableSetup().members.size(),size_t(1));
    QCOMPARE(copied.getStructuralCableSetup().members[0].referenceLength,
             cable.members[0].referenceLength);
    QCOMPARE(copied.getStructuralCableSetup().members[0].initialTension,
             cable.members[0].initialTension);
    QCOMPARE(copied.getStructuralCableSetup().maxIterations,uint(83));

    RProblemSetup assigned;
    assigned = setup;
    QCOMPARE(assigned.getStructuralCableSetup().meshFingerprint,cable.meshFingerprint);
    QCOMPARE(assigned.getStructuralCableSetup().absoluteForceTolerance,
             cable.absoluteForceTolerance);
}

void TestStructuralCableSetup::problemSetupRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii") << false;
    QTest::newRow("binary") << true;
}

void TestStructuralCableSetup::problemSetupRoundTrip()
{
    QFETCH(bool,binary);
    QCOMPARE(RModel::version.toString(),QString("1.3.17"));
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RModel source = makeCableSetupModel();
    RStructuralCableSetup cable;
    cable.enabled = true;
    cable.members = {{0,1.2345678901234567,123.45678901234567}};
    cable.maxIterations = 83;
    cable.maxLineSearchIterations = 31;
    cable.absoluteForceTolerance = 2.345678901234567e-11;
    cable.relativeForceTolerance = 4.567890123456789e-12;
    cable.bindToModel(source);
    source.getProblemSetup().setStructuralCableSetup(cable);

    const QString extension = RModel::getDefaultFileExtension(binary);
    const QString path = directory.filePath("cable-setup." + extension);
    RModel restored;
    QCOMPARE(source.write(path,false),path);
    restored.read(path);

    const RStructuralCableSetup &actual = restored.getProblemSetup().getStructuralCableSetup();
    QCOMPARE(actual.enabled,true);
    QCOMPARE(actual.meshFingerprint,cable.meshFingerprint);
    QCOMPARE(actual.members.size(),size_t(1));
    QCOMPARE(actual.members[0].elementId,uint(0));
    QCOMPARE(actual.members[0].referenceLength,cable.members[0].referenceLength);
    QCOMPARE(actual.members[0].initialTension,cable.members[0].initialTension);
    QCOMPARE(actual.maxIterations,cable.maxIterations);
    QCOMPARE(actual.maxLineSearchIterations,cable.maxLineSearchIterations);
    QCOMPARE(actual.absoluteForceTolerance,cable.absoluteForceTolerance);
    QCOMPARE(actual.relativeForceTolerance,cable.relativeForceTolerance);
    actual.validate(restored);
}

void TestStructuralCableSetup::readsVersion1312WithoutCableSetup_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii-1.3.12") << false;
    QTest::newRow("binary-1.3.12") << true;
}

void TestStructuralCableSetup::readsVersion1312WithoutCableSetup()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(binary ? "legacy.bin" : "legacy.txt");

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
        QVERIFY(out.commit());
    }

    RFile in(path,binary ? RFile::BINARY : RFile::ASCII);
    in.setVersion(RVersion(1,3,12));
    QVERIFY(in.open(QIODevice::ReadOnly));
    RProblemSetup restored;
    restored.getStructuralCableSetup().enabled = true;
    restored.getStructuralCableSetup().meshFingerprint = QString(64,QLatin1Char('a'));
    if (binary) RFileIO::readBinary(in,restored);
    else RFileIO::readAscii(in,restored);

    const RStructuralCableSetup &cable = restored.getStructuralCableSetup();
    QVERIFY(!cable.enabled);
    QVERIFY(cable.meshFingerprint.isEmpty());
    QVERIFY(cable.members.empty());
    QCOMPARE(cable.maxIterations,uint(50));
    QCOMPARE(cable.maxLineSearchIterations,uint(24));
    QCOMPARE(cable.absoluteForceTolerance,1.0e-8);
    QCOMPARE(cable.relativeForceTolerance,1.0e-8);
}

void TestStructuralCableSetup::rejectsTruncatedBinarySetupWithoutMutation()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RStructuralCableSetup target;
    target.enabled = true;
    target.meshFingerprint = QString(64,QLatin1Char('b'));
    target.members = {{7,2.0,3.0}};
    target.maxIterations = 77;
    target.maxLineSearchIterations = 19;
    target.absoluteForceTolerance = 5.0e-9;
    target.relativeForceTolerance = 6.0e-9;

    const QString emptyPath = directory.filePath("empty-tail.bin");
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
    QCOMPARE(target.members.size(),size_t(1));
    QCOMPARE(target.members[0].elementId,uint(7));
    QCOMPARE(target.maxIterations,uint(77));

    const QString cutPath = directory.filePath("before-max-iterations.bin");
    {
        RSaveFile out(cutPath,RSaveFile::BINARY);
        QVERIFY(out.open(QIODevice::WriteOnly));
        RFileIO::writeBinary(out,true);
        RFileIO::writeBinary(out,QString(64,QLatin1Char('a')));
        RFileIO::writeBinary(out,uint(1));
        RFileIO::writeBinary(out,uint(0));
        RFileIO::writeBinary(out,1.25);
        RFileIO::writeBinary(out,2.5);
        QVERIFY(out.commit());
    }
    {
        RFile in(cutPath,RFile::BINARY);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,target),RError);
    }
    QVERIFY(target.enabled);
    QCOMPARE(target.meshFingerprint,QString(64,QLatin1Char('b')));
    QCOMPARE(target.members.size(),size_t(1));
    QCOMPARE(target.members[0].referenceLength,2.0);
    QCOMPARE(target.members[0].initialTension,3.0);
    QCOMPARE(target.maxIterations,uint(77));
    QCOMPARE(target.maxLineSearchIterations,uint(19));
    QCOMPARE(target.absoluteForceTolerance,5.0e-9);
    QCOMPARE(target.relativeForceTolerance,6.0e-9);
}

void TestStructuralCableSetup::rejectsInvalidValuesAndTopology()
{
    RStructuralCableSetup setup;
    setup.enabled = true;
    setup.members = {{0,1.0,0.0}};
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError); // Missing mesh fingerprint.

    setup.meshFingerprint = QString(64,QLatin1Char('a'));
    setup.members.push_back({0,1.0,1.0});
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError); // Duplicate element ID.

    setup.members.resize(1);
    setup.members[0].referenceLength = std::numeric_limits<double>::infinity();
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.members[0].referenceLength = 1.0;
    setup.members[0].initialTension = -1.0;
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.members[0].initialTension = 0.0;
    setup.maxIterations = 0;
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);
    setup.maxIterations = 1;
    setup.relativeForceTolerance = std::numeric_limits<double>::quiet_NaN();
    QVERIFY_EXCEPTION_THROWN(setup.validate(),RError);

    RModel trussModel = makeCableSetupModel();
    setup = RStructuralCableSetup();
    setup.enabled = true;
    setup.members = {{1,1.0,1.0}};
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(trussModel),RError); // Unknown element ID.
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel beamModel = makeCableSetupModel(R_ELEMENT_BEAM1);
    setup.members = {{0,1.0,1.0}};
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(beamModel),RError); // Non-TRUSS1 topology.
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel outOfRangeNode = makeCableSetupModel();
    outOfRangeNode.getElement(0).setNodeId(1,9);
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(outOfRangeNode),RError);
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel repeatedNode = makeCableSetupModel();
    repeatedNode.getElement(0).setNodeId(1,0);
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(repeatedNode),RError);
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel zeroLength = makeCableSetupModel();
    zeroLength.getNode(1).set(0.0,0.0,0.0);
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(zeroLength),RError);
    QVERIFY(setup.meshFingerprint.isEmpty());

    RModel nonFiniteNode = makeCableSetupModel();
    nonFiniteNode.getNode(1).setX(std::numeric_limits<double>::infinity());
    QVERIFY_EXCEPTION_THROWN(setup.bindToModel(nonFiniteNode),RError);
    QVERIFY(setup.meshFingerprint.isEmpty());
}

void TestStructuralCableSetup::rejectsStaleMeshBinding()
{
    RModel model = makeCableSetupModel();
    RStructuralCableSetup setup;
    setup.enabled = true;
    setup.members = {{0,1.0,10.0}};
    setup.bindToModel(model);
    setup.validate(model);

    model.getNode(1).setX(1.1);
    QVERIFY_EXCEPTION_THROWN(setup.validate(model),RError);
}

QTEST_APPLESS_MAIN(TestStructuralCableSetup)

#include "tst_structural_cable_setup.moc"
