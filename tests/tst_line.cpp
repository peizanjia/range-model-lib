#include <QtTest>
#include <QTemporaryDir>

#include <cmath>

#include "rml_model.h"

class TestLine : public QObject
{
    Q_OBJECT

    private slots:

        void defaultsAndCopy();
        void rejectsInvalidSection();
        void modelRoundTrip_data();
        void modelRoundTrip();
};

void TestLine::defaultsAndCopy()
{
    RLine line;
    QCOMPARE(line.getCrossArea(),0.0);
    QCOMPARE(line.getSecondMomentY(),0.0);
    QCOMPARE(line.getSecondMomentZ(),0.0);
    QCOMPARE(line.getTorsionalConstant(),0.0);
    QCOMPARE(line.getSectionModulusY(),0.0);
    QCOMPARE(line.getSectionModulusZ(),0.0);
    QCOMPARE(line.getTorsionalSectionModulus(),0.0);
    QCOMPARE(line.getLocalYDirection()[0],0.0);
    QCOMPARE(line.getLocalYDirection()[1],1.0);
    QCOMPARE(line.getLocalYDirection()[2],0.0);

    line.setCrossArea(0.02);
    line.setSecondMomentY(2.0e-5);
    line.setSecondMomentZ(3.0e-5);
    line.setTorsionalConstant(4.0e-5);
    line.setSectionModulusY(5.0e-4);
    line.setSectionModulusZ(6.0e-4);
    line.setTorsionalSectionModulus(7.0e-4);
    line.setLocalYDirection(RR3Vector(0.0,2.0,2.0));
    const RLine copy(line);
    QCOMPARE(copy.getCrossArea(),0.02);
    QCOMPARE(copy.getSecondMomentY(),2.0e-5);
    QCOMPARE(copy.getSecondMomentZ(),3.0e-5);
    QCOMPARE(copy.getTorsionalConstant(),4.0e-5);
    QCOMPARE(copy.getSectionModulusY(),5.0e-4);
    QCOMPARE(copy.getSectionModulusZ(),6.0e-4);
    QCOMPARE(copy.getTorsionalSectionModulus(),7.0e-4);
    QVERIFY(std::fabs(copy.getLocalYDirection().length()-1.0) < 1.0e-14);
}

void TestLine::rejectsInvalidSection()
{
    RLine line;
    QVERIFY_EXCEPTION_THROWN(line.setSecondMomentY(-1.0),RError);
    QVERIFY_EXCEPTION_THROWN(line.setSecondMomentZ(-1.0),RError);
    QVERIFY_EXCEPTION_THROWN(line.setTorsionalConstant(-1.0),RError);
    QVERIFY_EXCEPTION_THROWN(line.setSectionModulusY(-1.0),RError);
    QVERIFY_EXCEPTION_THROWN(line.setSectionModulusZ(-1.0),RError);
    QVERIFY_EXCEPTION_THROWN(line.setTorsionalSectionModulus(-1.0),RError);
    QVERIFY_EXCEPTION_THROWN(line.setLocalYDirection(RR3Vector(0.0,0.0,0.0)),RError);
}

void TestLine::modelRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::newRow("ascii") << false;
    QTest::newRow("binary") << true;
}

void TestLine::modelRoundTrip()
{
    QFETCH(bool,binary);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    RModel source;
    source.setNLines(1);
    RLine &line = source.getLine(0);
    line.setName("Frame section");
    line.setCrossArea(0.012);
    line.setSecondMomentY(1.2e-5);
    line.setSecondMomentZ(2.3e-5);
    line.setTorsionalConstant(3.4e-5);
    line.setSectionModulusY(4.5e-4);
    line.setSectionModulusZ(5.6e-4);
    line.setTorsionalSectionModulus(6.7e-4);
    line.setLocalYDirection(RR3Vector(0.0,1.0,1.0));

    const QString fileName = directory.filePath("line." + RModel::getDefaultFileExtension(binary));
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

    QCOMPARE(restored.getNLines(),1u);
    const RLine &copy = restored.getLine(0);
    QCOMPARE(copy.getName(),QString("Frame section"));
    const double scalarTolerance = binary ? 1.0e-15 : 1.0e-10;
    QVERIFY(std::fabs(copy.getCrossArea()-0.012) < scalarTolerance);
    QVERIFY(std::fabs(copy.getSecondMomentY()-1.2e-5) < scalarTolerance);
    QVERIFY(std::fabs(copy.getSecondMomentZ()-2.3e-5) < scalarTolerance);
    QVERIFY(std::fabs(copy.getTorsionalConstant()-3.4e-5) < scalarTolerance);
    QVERIFY(std::fabs(copy.getSectionModulusY()-4.5e-4) < scalarTolerance);
    QVERIFY(std::fabs(copy.getSectionModulusZ()-5.6e-4) < scalarTolerance);
    QVERIFY(std::fabs(copy.getTorsionalSectionModulus()-6.7e-4) < scalarTolerance);
    const double directionTolerance = binary ? 1.0e-14 : 1.0e-6;
    QVERIFY(std::fabs(copy.getLocalYDirection()[1]-std::sqrt(0.5)) < directionTolerance);
    QVERIFY(std::fabs(copy.getLocalYDirection()[2]-std::sqrt(0.5)) < directionTolerance);
}

QTEST_APPLESS_MAIN(TestLine)

#include "tst_line.moc"
