#include <QtTest>
#include <QFile>
#include <QTemporaryDir>
#include <stdexcept>
#include "rml_model.h"
#include "rml_file_io.h"

namespace {
RModel fixture(bool plate = false)
{
    RModel m;
    const double xy[4][2]{{0,0},{1,0},{1,1},{0,1}};
    for (uint n = 0; n < (plate ? 4u : 2u); ++n) m.addNode(RNode(xy[n][0],xy[n][1],0));
    RElement e(plate ? R_ELEMENT_QUAD1 : R_ELEMENT_BEAM1);
    for (uint n = 0; n < e.size(); ++n) e.setNodeId(n,n);
    m.addElement(e,true,0);
    m.setProblemTaskTree(RProblemTaskItem::generateDefaultTree(R_PROBLEM_STRESS));
    auto &d = m.getProblemSetup().getStructuralDynamicsSetup();
    d.setEnabled(true); d.setTimeStep(.02); d.setNSteps(7);
    m.getTimeSolver().setInputStartTime(.02);
    RStructuralPiezoLayeredSetup setup;
    setup.enabled = true;
    if (plate) setup.family = RStructuralPiezoLayeredModelFamily::FlatPlate;
    RStructuralPiezoLayeredMaterial material;
    material.materialId = "host"; material.density = 7800;
    material.sourceId = "publication-fixture"; material.sourceSha256 = QString(64,'a');
    for (uint i = 0; i < 6; ++i) material.elasticityOrCompliance[6*i+i] = 1e9;
    setup.materials = {material};
    setup.members = {{m.getElementPhysicalId(0),{{"host",-.01,.01,plate ? 1.0 : .1,"",1,{{-1,1,-1,1}}}}}};
    setup.supports = {{m.getNodePhysicalId(0),RStructuralPiezoPhysicalDof::UX,0}};
    setup.excitationConfiguration = "whole-excitation";
    setup.controllerConfiguration = "whole-controller";
    setup.designConfiguration = "whole-design";
    setup.bindToModel(m);
    m.getProblemSetup().setStructuralPiezoLayeredSetup(setup);
    return m;
}
template<class T> QByteArray bytes(const T &value)
{
    QTemporaryDir directory;
    RSaveFile output(directory.filePath("value.bin"),RSaveFile::BINARY);
    output.setVersion(RModel::version);
    if (!output.open(QIODevice::WriteOnly)) throw std::runtime_error("open");
    RFileIO::writeBinary(output,value);
    if (!output.commit()) throw std::runtime_error("commit");
    QFile input(directory.filePath("value.bin"));
    if (!input.open(QIODevice::ReadOnly)) throw std::runtime_error("read");
    return input.readAll();
}
class LiveModel : public RModel {
public:
    int documentIdentity = 73;
    void exhaustIds() { nextNodePhysicalId = nextElementPhysicalId = nextElementGroupPhysicalId = RConstants::eod; }
};
}

class TestPiezoExperimentPublication : public QObject {
    Q_OBJECT
private slots:
    void completePreparedBody();
    void rejectsBeforePublication();
    void geometryBoundary_data();
    void geometryBoundary();
    void allocatorHighWater();
};

void TestPiezoExperimentPublication::completePreparedBody()
{
    LiveModel live;
    live.setName("Live document");
    const auto candidate = fixture();
    QString error;
    QVERIFY2(live.applyPiezoExperimentCandidate(candidate,[](const RModel &) { return true; },&error),qPrintable(error));
    live.getLine(0).setName("Retained display name");
    live.getLine(0).getData().setVisible(false);
    RModel changed(live);
    changed.setName("Attempted document replacement");
    changed.getLine(0).setName("Attempted group replacement");
    auto &setup = changed.getProblemSetup();
    setup.getStructuralDynamicsSetup().setTimeStep(.03);
    changed.getTimeSolver().setInputStartTime(.03);
    changed.getTimeSolver().setInputNTimeSteps(9);
    changed.getTimeSolver().setInputTimeStepSize(.03);
    auto &layered = setup.getStructuralPiezoLayeredSetup();
    layered.controllerConfiguration = "updated-controller";
    layered.excitationConfiguration = "updated-excitation";
    layered.bindToModel(changed);
    setup.getStructuralPiezoLayeredRestart() = {true,layered.sourceFingerprint,"accepted-state"};
    const auto expectedSetup = bytes(setup), expectedTask = bytes(changed.getProblemTaskTree()), expectedTime = bytes(changed.getTimeSolver());
    QVERIFY2(live.applyPiezoExperimentCandidate(changed,[&](const RModel &prepared) {
        return bytes(prepared.getProblemSetup()) == expectedSetup
            && bytes(prepared.getProblemTaskTree()) == expectedTask
            && bytes(prepared.getTimeSolver()) == expectedTime
            && prepared.getLine(0).getName() == "Retained display name"
            && !prepared.getLine(0).getData().getVisible();
    },&error),qPrintable(error));
    QCOMPARE(live.documentIdentity,73);
    QCOMPARE(live.getName(),QString("Live document"));
    QCOMPARE(bytes(live.getProblemSetup()),expectedSetup);
    QCOMPARE(bytes(live.getTimeSolver()),expectedTime);
    QCOMPARE(bytes(live.getProblemTaskTree()),expectedTask);
    QCOMPARE(live.getProblemSetup().getStructuralPiezoLayeredRestart().stateBytes,QByteArray("accepted-state"));
}

void TestPiezoExperimentPublication::rejectsBeforePublication()
{
    auto live = fixture();
    const auto setupBefore = bytes(live.getProblemSetup()), timeBefore = bytes(live.getTimeSolver()), groupBefore = bytes(live.getLine(0));
    auto candidate = live;
    candidate.getProblemSetup().getStructuralPiezoLayeredSetup().controllerConfiguration = "changed";
    candidate.getProblemSetup().getStructuralPiezoLayeredSetup().bindToModel(candidate);
    QString error;
    QVERIFY(!live.applyPiezoExperimentCandidate(candidate,[](const RModel &) { return false; },&error));
    QVERIFY(!live.applyPiezoExperimentCandidate(candidate,[](const RModel &) -> bool { throw std::bad_alloc(); },&error));
    auto unsupported = candidate;
    unsupported.setProblemTaskTree(RProblemTaskItem::generateDefaultTree(R_PROBLEM_HEAT));
    QVERIFY(!live.applyPiezoExperimentCandidate(unsupported,[](const RModel &) { return true; },&error));
    QCOMPARE(bytes(live.getProblemSetup()),setupBefore);
    QCOMPARE(bytes(live.getTimeSolver()),timeBefore);
    QCOMPARE(bytes(live.getLine(0)),groupBefore);
    QCOMPARE(live.getNNodes(),2u);
    QCOMPARE(live.getNElements(),1u);
}

void TestPiezoExperimentPublication::geometryBoundary_data()
{
    QTest::addColumn<bool>("plate");
    QTest::newRow("beam") << false;
    QTest::newRow("plate") << true;
}
void TestPiezoExperimentPublication::geometryBoundary()
{
    QFETCH(bool,plate);
    RModel live, baseline(live);
    const auto generated = fixture(plate);
    QString error;
    QVERIFY2(live.applyPiezoExperimentCandidate(generated,[](const RModel &) { return true; },&error),qPrintable(error));
    auto geometryChange = live;
    geometryChange.getNode(1).setX(1.1);
    geometryChange.getProblemSetup().getStructuralPiezoLayeredSetup().bindToModel(geometryChange);
    QVERIFY(!live.applyPiezoExperimentCandidate(geometryChange,[](const RModel &) { return true; },&error));
    QVERIFY2(live.applyPiezoExperimentCandidate(baseline,[](const RModel &) { return true; },&error),qPrintable(error));
    QVERIFY2(live.applyPiezoExperimentCandidate(generated,[](const RModel &) { return true; },&error),qPrintable(error));
    QCOMPARE(live.getNodePhysicalId(1),generated.getNodePhysicalId(1));
    QCOMPARE(live.getElementPhysicalId(0),generated.getElementPhysicalId(0));
}

void TestPiezoExperimentPublication::allocatorHighWater()
{
    LiveModel live;
    RModel baseline(live);
    const auto generated = fixture();
    QString error;
    QVERIFY(live.applyPiezoExperimentCandidate(generated,[](const RModel &) { return true; },&error));
    QVERIFY(live.applyPiezoExperimentCandidate(baseline,[](const RModel &) { return true; },&error));
    QVERIFY(live.applyPiezoExperimentCandidate(generated,[](const RModel &) { return true; },&error));
    const uint n = live.getNNodes(); live.addNode(RNode(2,0,0));
    QVERIFY(live.getNodePhysicalId(n) > generated.getNodePhysicalId(1));
    RElement e(R_ELEMENT_BEAM1); e.setNodeId(0,1); e.setNodeId(1,n);
    const uint element = live.getNElements(); live.addElement(e,true,1);
    QVERIFY(live.getElementPhysicalId(element) > generated.getElementPhysicalId(0));
    QVERIFY(live.getLine(1).getPhysicalId() > generated.getLine(0).getPhysicalId());
    LiveModel exhausted;
    QVERIFY(exhausted.applyPiezoExperimentCandidate(generated,[](const RModel &) { return true; },&error));
    exhausted.exhaustIds();
    QVERIFY(exhausted.applyPiezoExperimentCandidate(baseline,[](const RModel &) { return true; },&error));
    QVERIFY_EXCEPTION_THROWN(exhausted.addNode(RNode()),RError);
    QVERIFY_EXCEPTION_THROWN(exhausted.addLine(RLine()),RError);
}

QTEST_APPLESS_MAIN(TestPiezoExperimentPublication)
#include "tst_piezo_experiment_publication.moc"
