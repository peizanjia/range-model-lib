#include <QtTest>
#include <QTemporaryDir>

#include "rml_model.h"

class TestProblemSetup : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndCopy();
    void rejectsInvalidMassMatrixType();
    void modelRoundTrip_data();
    void modelRoundTrip();
};

void TestProblemSetup::defaultsAndCopy()
{
    RProblemSetup setup;
    QCOMPARE(setup.getMassMatrixType(),RMassMatrixType::Consistent);

    setup.setMassMatrixType(RMassMatrixType::Lumped);
    const RProblemSetup copy(setup);
    QCOMPARE(copy.getMassMatrixType(),RMassMatrixType::Lumped);
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
}

QTEST_APPLESS_MAIN(TestProblemSetup)

#include "tst_problem_setup.moc"
