#include <QtTest>
#include <QTemporaryDir>

#include "rml_file_io.h"
#include "rml_line.h"
#include "rml_model.h"

class TestElementGroupIO : public QObject
{
    Q_OBJECT

private slots:
    void groupReadReplacesConditionSnapshot_data();
    void groupReadReplacesConditionSnapshot();
};

void TestElementGroupIO::groupReadReplacesConditionSnapshot_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii") << false;
    QTest::newRow("binary") << true;
}

void TestElementGroupIO::groupReadReplacesConditionSnapshot()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RLine source;
    source.removeEnvironmentCondition(R_ENVIRONMENT_CONDITION_G_ACCELERATION);
    source.addBoundaryCondition(RBoundaryCondition(R_BOUNDARY_CONDITION_FORCE));
    source.addInitialCondition(RInitialCondition(R_INITIAL_CONDITION_DISPLACEMENT));
    QVERIFY(!source.hasEnvironmentCondition(R_ENVIRONMENT_CONDITION_G_ACCELERATION));
    QVERIFY(source.hasEnvironmentCondition(R_ENVIRONMENT_CONDITION_TEMPERATURE));

    const QString path = directory.filePath(binary ? "group.bin" : "group.txt");
    {
        RSaveFile out(path,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        QVERIFY(out.open(QIODevice::WriteOnly));
        if (binary) RFileIO::writeBinary(out,source);
        else RFileIO::writeAscii(out,source);
        QVERIFY(out.commit());
    }

    // A fresh line has constructor gravity and temperature defaults. Reading a
    // snapshot with gravity omitted must remove that default and preserve only
    // the serialized temperature environment condition.
    RLine target;
    {
        RFile in(path,binary ? RFile::BINARY : RFile::ASCII);
        in.setVersion(RVersion(1,3,13));
        QVERIFY(in.open(QIODevice::ReadOnly));
        if (binary) RFileIO::readBinary(in,target);
        else RFileIO::readAscii(in,target);
    }
    QVERIFY(!target.hasEnvironmentCondition(R_ENVIRONMENT_CONDITION_G_ACCELERATION));
    QVERIFY(target.hasEnvironmentCondition(R_ENVIRONMENT_CONDITION_TEMPERATURE));
    QCOMPARE(target.getNEnvironmentConditions(),uint(1));
    QVERIFY(target.hasBoundaryCondition(R_BOUNDARY_CONDITION_FORCE));
    QVERIFY(!target.hasBoundaryCondition(R_BOUNDARY_CONDITION_VELOCITY_NORMAL));
    QCOMPARE(target.getNBoundaryConditions(),uint(1));
    QVERIFY(target.hasInitialCondition(R_INITIAL_CONDITION_DISPLACEMENT));
    QVERIFY(!target.hasInitialCondition(R_INITIAL_CONDITION_TEMPERATURE));
    QCOMPARE(target.getNInitialConditions(),uint(1));

    // Reuse the same instance after adding conditions absent from the stream.
    // Deserialization must replace each map rather than merge into it.
    target.addBoundaryCondition(RBoundaryCondition(R_BOUNDARY_CONDITION_VELOCITY_NORMAL));
    target.addEnvironmentCondition(REnvironmentCondition(R_ENVIRONMENT_CONDITION_CUSTOM));
    target.addInitialCondition(RInitialCondition(R_INITIAL_CONDITION_TEMPERATURE));
    {
        RFile in(path,binary ? RFile::BINARY : RFile::ASCII);
        in.setVersion(RVersion(1,3,13));
        QVERIFY(in.open(QIODevice::ReadOnly));
        if (binary) RFileIO::readBinary(in,target);
        else RFileIO::readAscii(in,target);
    }
    QVERIFY(!target.hasEnvironmentCondition(R_ENVIRONMENT_CONDITION_G_ACCELERATION));
    QVERIFY(target.hasEnvironmentCondition(R_ENVIRONMENT_CONDITION_TEMPERATURE));
    QVERIFY(!target.hasEnvironmentCondition(R_ENVIRONMENT_CONDITION_CUSTOM));
    QCOMPARE(target.getNEnvironmentConditions(),uint(1));
    QVERIFY(target.hasBoundaryCondition(R_BOUNDARY_CONDITION_FORCE));
    QVERIFY(!target.hasBoundaryCondition(R_BOUNDARY_CONDITION_VELOCITY_NORMAL));
    QCOMPARE(target.getNBoundaryConditions(),uint(1));
    QVERIFY(target.hasInitialCondition(R_INITIAL_CONDITION_DISPLACEMENT));
    QVERIFY(!target.hasInitialCondition(R_INITIAL_CONDITION_TEMPERATURE));
    QCOMPARE(target.getNInitialConditions(),uint(1));

    RModel model;
    model.addLine(source);
    const RLine identifiedSource(model.getLine(0));
    const QString currentPath = directory.filePath(binary ? "group-v18.bin" : "group-v18.txt");
    {
        RSaveFile out(currentPath,binary ? RSaveFile::BINARY : RSaveFile::ASCII);
        out.setVersion(RVersion(1,3,18));
        QVERIFY(out.open(QIODevice::WriteOnly));
        if (binary) RFileIO::writeBinary(out,identifiedSource);
        else RFileIO::writeAscii(out,identifiedSource);
        QVERIFY(out.commit());
    }
    RLine currentTarget;
    {
        RFile in(currentPath,binary ? RFile::BINARY : RFile::ASCII);
        in.setVersion(RVersion(1,3,18));
        QVERIFY(in.open(QIODevice::ReadOnly));
        if (binary) RFileIO::readBinary(in,currentTarget);
        else RFileIO::readAscii(in,currentTarget);
    }
    QCOMPARE(currentTarget.getPhysicalId(),identifiedSource.getPhysicalId());

    {
        RFile in(path,binary ? RFile::BINARY : RFile::ASCII);
        in.setVersion(RVersion(1,3,13));
        QVERIFY(in.open(QIODevice::ReadOnly));
        if (binary) RFileIO::readBinary(in,currentTarget);
        else RFileIO::readAscii(in,currentTarget);
    }
    QCOMPARE(currentTarget.getPhysicalId(),0u);
}

QTEST_APPLESS_MAIN(TestElementGroupIO)

#include "tst_element_group_io.moc"
