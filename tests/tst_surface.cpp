#include <QtTest>
#include <QTemporaryDir>

#include "rml_model.h"
#include "rml_surface.h"

class TestSurface : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndCopy();
    void rejectsNonSurfaceFormulations();
    void modelRoundTrip_data();
    void modelRoundTrip();
};

void TestSurface::defaultsAndCopy()
{
    RSurface surface;
    QCOMPARE(surface.getStructuralFormulation(),RElementFormulation::PlaneStrain);

    surface.setThickness(0.25);
    surface.setStructuralFormulation(RElementFormulation::PlaneStress);
    const RSurface copy(surface);
    QCOMPARE(copy.getThickness(),0.25);
    QCOMPARE(copy.getStructuralFormulation(),RElementFormulation::PlaneStress);
}

void TestSurface::rejectsNonSurfaceFormulations()
{
    RSurface surface;
    QVERIFY_EXCEPTION_THROWN(surface.setStructuralFormulation(RElementFormulation::Solid3D),RError);
    QCOMPARE(surface.getStructuralFormulation(),RElementFormulation::PlaneStrain);
}

void TestSurface::modelRoundTrip_data()
{
    QTest::addColumn<bool>("binary");
    QTest::addColumn<int>("formulation");
    QTest::newRow("ascii-plane-stress") << false << int(RElementFormulation::PlaneStress);
    QTest::newRow("ascii-plane-strain") << false << int(RElementFormulation::PlaneStrain);
    QTest::newRow("binary-plane-stress") << true << int(RElementFormulation::PlaneStress);
    QTest::newRow("binary-axisymmetric-contract") << true << int(RElementFormulation::Axisymmetric);
}

void TestSurface::modelRoundTrip()
{
    QFETCH(bool,binary);
    QFETCH(int,formulation);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    RModel source;
    source.setNSurfaces(1);
    source.getSurface(0).setName("Structural surface");
    source.getSurface(0).setThickness(0.125);
    source.getSurface(0).getData().setVisible(true);
    source.getSurface(0).getData().setDrawArrowFrom(false);
    source.getSurface(0).setStructuralFormulation(static_cast<RElementFormulation>(formulation));

    const QString extension = RModel::getDefaultFileExtension(binary);
    const QString fileName = directory.filePath("surface." + extension);
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
    QCOMPARE(restored.getNSurfaces(),1u);
    QCOMPARE(restored.getSurface(0).getName(),QString("Structural surface"));
    QCOMPARE(restored.getSurface(0).getThickness(),0.125);
    QVERIFY(restored.getSurface(0).getData().getVisible());
    QVERIFY(!restored.getSurface(0).getData().getDrawArrowFrom());
    QCOMPARE(restored.getSurface(0).getStructuralFormulation(),
             static_cast<RElementFormulation>(formulation));
}

QTEST_APPLESS_MAIN(TestSurface)

#include "tst_surface.moc"
