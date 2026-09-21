#include <QtTest>
#include <QTemporaryDir>

#include <rbl_error.h>

#include "rml_material.h"

class TestMaterial : public QObject
{
    Q_OBJECT

private slots:

    void defaultFileExtension();
    void jsonRoundTrip();
    void fileRoundTrip();
    void fileRoundTrip_data();
    void evaluationContext();
};

void TestMaterial::defaultFileExtension()
{
    QCOMPARE(RMaterial::getDefaultFileExtension(), RMaterial::File::Json::extension);
}

void TestMaterial::jsonRoundTrip()
{
    RMaterial material = RMaterial::generateDefault();
    material.setName("Test material");
    material.setState(RMaterial::Solid);

    RMaterialProperty property(RMaterialProperty::Density);
    // The constructor seeds the table with an initial value - drop it so that
    // the table content is known exactly.
    property.clear();
    property.add(300.0, 1000.0);
    property.add(400.0, 900.0);
    material.add(property);

    RMaterial readMaterial = RMaterial::fromJson(material.toJson());

    QCOMPARE(readMaterial.getID(), material.getID());
    QCOMPARE(readMaterial.getName(), material.getName());
    QCOMPARE(readMaterial.getState(), material.getState());
    QCOMPARE(readMaterial.size(), material.size());
    QVERIFY(readMaterial.propertiesEqual(material));

    // Property types must survive the round trip - not silently fall back to None.
    for (uint i = 0; i < material.size(); i++)
    {
        QCOMPARE(readMaterial.get(i).getType(), material.get(i).getType());
        QCOMPARE(readMaterial.get(i).size(), material.get(i).size());
    }

    uint position = readMaterial.findPosition(RMaterialProperty::Density);
    QVERIFY(position < readMaterial.size());
    QCOMPARE(readMaterial.get(position).size(), 2u);
    QCOMPARE(readMaterial.get(position).getKey(0), 300.0);
    QCOMPARE(readMaterial.get(position).getValue(0), 1000.0);
    QCOMPARE(readMaterial.get(position).getKey(1), 400.0);
    QCOMPARE(readMaterial.get(position).getValue(1), 900.0);
}

void TestMaterial::fileRoundTrip_data()
{
    QTest::addColumn<QString>("extension");

    QTest::newRow("json") << RMaterial::File::Json::extension;
    QTest::newRow("ascii") << RMaterial::File::Ascii::extension;
    QTest::newRow("binary") << RMaterial::File::Binary::extension;
}

void TestMaterial::fileRoundTrip()
{
    QFETCH(QString, extension);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    RMaterial material = RMaterial::generateDefault();
    material.setName("Test material");
    material.setState(RMaterial::Liquid);

    QString fileName = dir.filePath("material." + extension);

    material.write(fileName);
    QVERIFY(QFile::exists(fileName));

    RMaterial readMaterial;
    readMaterial.read(fileName);

    QCOMPARE(readMaterial.getID(), material.getID());
    QCOMPARE(readMaterial.getName(), material.getName());
    QCOMPARE(readMaterial.getState(), material.getState());
    QCOMPARE(readMaterial.size(), material.size());
    QVERIFY(readMaterial.propertiesEqual(material));
}

void TestMaterial::evaluationContext()
{
    RMaterial material;
    RMaterialProperty density(RMaterialProperty::Density);
    density.clear();
    density.add(300.0,1000.0);
    density.add(400.0,900.0);
    material.add(density);

    RMaterialPointState committed;
    committed.set("plastic-strain",0.125);

    RMaterialEvaluationContext context;
    context.setTime(2.0);
    context.setTimeStep(0.01);
    context.setField(R_VARIABLE_TEMPERATURE,350.0);
    context.setStrain({0.01,0.0,0.0});
    context.setStrainIncrement({0.001,0.0,0.0});
    context.setCommittedState(committed);

    const RMaterialEvaluationResult result = material.evaluateProperty(RMaterialProperty::Density,context);
    QCOMPARE(result.getValue(),950.0);
    QCOMPARE(result.getBehaviorId(),QString("builtin.table.v1"));
    QCOMPARE(result.getTrialState().get("plastic-strain"),0.125);
    QCOMPARE(context.getTime(),2.0);
    QCOMPARE(context.getTimeStep(),0.01);
    QCOMPARE(context.getStrain().size(),3);
    QCOMPARE(context.getStrainIncrement().size(),3);

    RMaterialEvaluationContext missingField;
    QVERIFY_EXCEPTION_THROWN(material.evaluateProperty(RMaterialProperty::Density,missingField),RError);
    QVERIFY_EXCEPTION_THROWN(material.evaluateProperty(RMaterialProperty::PoissonRatio,context),RError);
}

QTEST_APPLESS_MAIN(TestMaterial)

#include "tst_material.moc"
