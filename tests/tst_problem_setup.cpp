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
    void structuralCatalogAndCouplings();
    void structuralValidation();
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
