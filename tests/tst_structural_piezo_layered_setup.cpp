#include <QtTest>
#include <limits>
#include <cmath>
#include <QTemporaryDir>
#include <QFile>
#include "rml_model.h"
#include "rml_file_io.h"
#include "rml_structural_piezo_layered_setup.h"

namespace {
RModel scene(bool plate=false)
{
    RModel m; const uint n=plate?4:2;
    const double xy[4][2]={{0,0},{1,0},{1,1},{0,1}};
    for(uint i=0;i<n;++i) { m.addNode(RNode(xy[i][0],xy[i][1],0)); m.setNodePhysicalId(i,101+11*i); }
    RElement e(plate?R_ELEMENT_QUAD1:R_ELEMENT_BEAM1); for(uint i=0;i<n;++i) e.setNodeId(i,i);
    m.addElement(e,true,0); m.setElementPhysicalId(0,501);
    m.setProblemTaskTree(RProblemTaskItem::generateDefaultTree(R_PROBLEM_STRESS));
    m.getProblemSetup().setMassMatrixType(RMassMatrixType::Consistent);
    auto &d=m.getProblemSetup().getStructuralDynamicsSetup(); d.setEnabled(true); d.setTimeStep(.01); d.setNSteps(10);
    m.getTimeSolver().setInputStartTime(.01); return m;
}
RStructuralPiezoLayeredSetup config(bool plate=false)
{
    RStructuralPiezoLayeredSetup s; s.enabled=true;
    if(plate) s.family=RStructuralPiezoLayeredModelFamily::FlatPlate;
    RStructuralPiezoLayeredMaterial a; a.materialId="host"; a.density=7800;
    for(uint i=0;i<6;++i) a.elasticityOrCompliance[6*i+i]=1e9;
    a.sourceId="example-host-v1"; a.sourceSha256=QString(64,'a');
    auto b=a; b.materialId="pzt"; b.piezoelectric=true; b.sourceId="example-pzt-v1"; b.coupling[0]=.1;
    for(uint i=0;i<3;++i) b.permittivity[3*i+i]=1e-8;
    s.materials={a,b}; s.ports={{{"drive"},12}};
    s.members={{501,{{"host",-.01,.01,plate?1.0:.1,"",1,{{-1,1,-1,1}}},{"pzt",.01,.011,plate?1.0:.1,"drive",1,{{-1,1,-1,1}}}}}};
    s.supports={{101,RStructuralPiezoPhysicalDof::UX,0}};
    return s;
}
RModel readoutScene()
{
    auto m=scene(); m.getElement(0).setType(R_ELEMENT_TRUSS1);
    auto &line=m.getLine(0); line.setCrossArea(.01);
    RMaterial material(RMaterial::Solid);
    RMaterialProperty density(RMaterialProperty::Density); density.add(293.15,7800); material.add(density);
    RMaterialProperty modulus(RMaterialProperty::ModulusOfElasiticity); modulus.add(293.15,2e11); material.add(modulus);
    line.setMaterial(material);
    auto &truss=m.getProblemSetup().getStructuralTrussDynamicsSetup(); truss.enabled=true;
    RStructuralTrussActiveMember member; member.physicalElementId=501; member.portId="drive";
    member.referenceLength=1; member.axialStiffness=100; member.coupling=.1; member.capacitance=1e-8;
    truss.activeMembers={member};
    m.getProblemSetup().getStructuralPiezoReadoutSetup().ports={{"drive"}};
    truss.bindToModel(m); return m;
}
}
class TestStructuralPiezoLayeredSetup:public QObject
{
    Q_OBJECT
private slots:
    void legacyDefault(); void familyAndPhysicalIdentity();
    void invalidSource_data(); void invalidSource();
    void exactSourceIdentity(); void sourceBinding(); void workpoint(); void dampingAuthorities(); void restartIsSeparate();
    void actualSourceInvalidation_data(); void actualSourceInvalidation(); void locatedCoverage();
    void unsupportedActualSource_data(); void unsupportedActualSource();
    void staleModelRead_data(); void staleModelRead();
    void persistence_data(); void persistence();
    void legacyAndDowngrade_data(); void legacyAndDowngrade();
    void malformedReader_data(); void malformedReader();
};
void TestStructuralPiezoLayeredSetup::legacyDefault()
{
    RStructuralPiezoLayeredSetup s; s.validate(); s.validate(RModel());
    QVERIFY(!s.hasNonDefaultData()); QVERIFY(s.identityBytes().isEmpty()); QVERIFY(s.computeSourceFingerprint(RModel()).isEmpty());
}
void TestStructuralPiezoLayeredSetup::familyAndPhysicalIdentity()
{
    auto s=config(); auto m=scene(); s.validate(m); s.bindToModel(m); s.validate(m);
    auto plate=config(true); auto pm=scene(true); plate.validate(pm);
    QVERIFY_EXCEPTION_THROWN(plate.validate(m),RError);
    s.sourceFingerprint.clear(); s.members[0].physicalElementId=0; QVERIFY_EXCEPTION_THROWN(s.validate(m),RError);
}
void TestStructuralPiezoLayeredSetup::invalidSource_data()
{
    QTest::addColumn<int>("mutation"); const char *names[]={"disabled-hidden","unknown-family","unknown-form","unknown-provenance","missing-provenance","bad-sha","duplicate-material","unknown-material","duplicate-host","missing-host","unknown-port","duplicate-port","unused-port","mechanical-electrode","overlap","width","coverage","polarity","left-frame","nonfinite","duplicate-support","unknown-dof","duplicate-field","hidden-workpoint","irrelevant-damping","short-sensor-voltage","unsupported-tri","disabled-dynamics","lumped-mass","wrong-task","unknown-field-node","duplicate-node","truss-conflict","legacy-piezo-conflict","legacy-damping"};
    for(int i=0;i<35;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralPiezoLayeredSetup::invalidSource()
{
    QFETCH(int,mutation); auto s=config(); auto m=scene(); auto &l=s.members[0].layers[1];
    switch(mutation) {
    case 0:s.enabled=false;break;
    case 1:s.family=static_cast<RStructuralPiezoLayeredModelFamily>(99);break;
    case 2:s.materials[0].form=static_cast<RStructuralPiezoMaterialForm>(99);break;
    case 3:s.materials[0].sourceKind=static_cast<RStructuralPiezoMaterialSourceKind>(99);break;
    case 4:s.materials[0].sourceId.clear();break;
    case 5:s.materials[0].sourceSha256="wrong";break;
    case 6:s.materials.push_back(s.materials[0]);break;
    case 7:l.materialId="missing";break;
    case 8:s.members.push_back(s.members[0]);break;
    case 9:s.members.clear();break;
    case 10:l.portId="missing";break;
    case 11:s.ports.push_back(s.ports[0]);break;
    case 12:s.ports.push_back({{"unused"},0});break;
    case 13:s.members[0].layers[0].portId="drive";break;
    case 14:l.lowerZ=0;break;
    case 15:l.width=0;break;
    case 16:l.coverage[1]=1.1;break;
    case 17:l.polarity=0;break;
    case 18:s.frame.ez[2]=-1;break;
    case 19:s.materials[0].elasticityOrCompliance[0]=std::numeric_limits<double>::quiet_NaN();break;
    case 20:s.supports.push_back(s.supports[0]);break;
    case 21:s.supports[0].dof=static_cast<RStructuralPiezoPhysicalDof>(6);break;
    case 22:s.initialVelocities={{101,{}},{101,{}}};break;
    case 23:s.workpoint.nodes={{101,{},{}}};break;
    case 24:s.damping.rayleighAlpha=1;break;
    case 25:s.ports[0].readout.role=RStructuralPiezoPortRole::Sensor;s.ports[0].readout.readout=RStructuralPiezoReadoutMode::ShortCircuitCharge;break;
    case 26:m.getElement(0).setType(R_ELEMENT_TRI1);break;
    case 27:m.getProblemSetup().getStructuralDynamicsSetup().setEnabled(false);break;
    case 28:m.getProblemSetup().setMassMatrixType(RMassMatrixType::Lumped);break;
    case 29:m.setProblemTaskTree(RProblemTaskItem());break;
    case 30:s.backgroundForces={{777,{}}};break;
    case 31:
        // Duplicate identities are rejected before a layered source can be constructed.
        QVERIFY_EXCEPTION_THROWN(m.setNodePhysicalId(1,101),RError);
        QCOMPARE(m.getNodePhysicalId(0),101u);
        QCOMPARE(m.getNodePhysicalId(1),112u);
        s.validate(m);
        return;
    case 32:m.getProblemSetup().getStructuralTrussDynamicsSetup().enabled=true;break;
    case 33:m.getProblemSetup().getStructuralPiezoSetup().enabled=true;break;
    case 34:m.getProblemSetup().getStructuralDynamicsSetup().setRayleighAlpha(.1);break;
    }
    QVERIFY_EXCEPTION_THROWN(s.validate(m),RError);
}
void TestStructuralPiezoLayeredSetup::exactSourceIdentity()
{
    const auto s=config(); auto other=s; const auto b=s.identityBytes();
    other.supports[0].value=-0.0; QVERIFY(b!=other.identityBytes());
    other=s; other.materials[1].coupling[1]=-0.0; QVERIFY(b!=other.identityBytes());
    other=s; other.materials[0].notes="new explicit provenance"; QVERIFY(b!=other.identityBytes());
    other=s; other.materials[0].density=std::nextafter(other.materials[0].density,8000.0); QVERIFY(b!=other.identityBytes());
    other=s; other.sourceFingerprint=QString(64,'b'); QCOMPARE(b,other.identityBytes());
    other=s; other.controllerConfiguration=QByteArray("source\0identity",15); QVERIFY(b!=other.identityBytes());
}
void TestStructuralPiezoLayeredSetup::sourceBinding()
{
    auto s=config(); auto m=scene(); s.bindToModel(m); const auto fingerprint=s.sourceFingerprint; const auto identity=s.identityBytes();
    m.getLine(0).setName("display rename"); QCOMPARE(s.computeSourceFingerprint(m),fingerprint);
    m.getNode(1).setX(1.1); QVERIFY(s.computeSourceFingerprint(m)!=fingerprint); QVERIFY_EXCEPTION_THROWN(s.validate(m),RError);
    s.bindToModel(m); s.validate(m); QCOMPARE(s.identityBytes(),identity);
    const auto rebound=s.sourceFingerprint; m.getProblemSetup().setMassMatrixType(RMassMatrixType::Lumped);
    QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError); QCOMPARE(s.sourceFingerprint,rebound);
}
void TestStructuralPiezoLayeredSetup::actualSourceInvalidation_data()
{
    QTest::addColumn<int>("mutation"); const char *names[]={"node-geometry","physical-node-id","physical-element-id","connectivity","group-section","material-density","dynamics-grid","initial-clock","opaque-declaration","provenance","electrode-bias","physical-support"};
    for(int i=0;i<12;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralPiezoLayeredSetup::actualSourceInvalidation()
{
    QFETCH(int,mutation); auto s=config(); auto m=scene(); const auto before=s.computeSourceFingerprint(m);
    switch(mutation) {
    case 0:m.getNode(1).setX(1.01);break;
    case 1:m.setNodePhysicalId(1,113);break;
    case 2:m.setElementPhysicalId(0,502);s.members[0].physicalElementId=502;break;
    case 3:m.getElement(0).setNodeId(0,1);m.getElement(0).setNodeId(1,0);break;
    case 4:m.getLine(0).setCrossArea(.2);break;
    case 5:{ RMaterial a(RMaterial::Solid); RMaterialProperty density(RMaterialProperty::Density); density.add(293.15,7850); a.add(density); m.getLine(0).setMaterial(a); break; }
    case 6:m.getProblemSetup().getStructuralDynamicsSetup().setTimeStep(.02);break;
    case 7:m.getTimeSolver().setInputStartTime(.02);break;
    case 8:s.excitationConfiguration="named force source";break;
    case 9:s.materials[0].sourceSha256=QString(64,'b');break;
    case 10:s.ports[0].initialValue=13;break;
    case 11:s.supports[0].value=.01;break;
    }
    QVERIFY(before!=s.computeSourceFingerprint(m));
}
void TestStructuralPiezoLayeredSetup::locatedCoverage()
{
    auto s=config(true); s.members[0].layers[1].coverage={{-.8,.2,-.7,.3}}; s.validate(scene(true));
    const auto before=s.identityBytes(); s.members[0].layers[1].coverage={{-.2,.8,-.3,.7}};
    // Equal-area rectangles at different positions have different source identities.
    QVERIFY(before!=s.identityBytes());
    auto left=s.members[0].layers[1]; left.coverage={{-1,0,-1,1}};
    auto right=left; right.coverage={{0,1,-1,1}};
    s.members[0].layers={s.members[0].layers[0],left,right}; s.validate();
    s.members[0].layers[2].coverage[0]=-.1; QVERIFY_EXCEPTION_THROWN(s.validate(),RError);
}
void TestStructuralPiezoLayeredSetup::unsupportedActualSource_data()
{
    QTest::addColumn<int>("mutation");
    const char *names[]={"mpc","coupling","prescribed-motion","sensor","actuator","native-control","native-control-hidden-state","legacy-control","legacy-control-hidden","thermal-enabled","thermal-hidden","dynamics-restart","hidden-dynamics-restart","truss-restart","hidden-truss-restart","generalized-alpha","central-difference","newmark-beta","newmark-gamma","unused-rho","point-volume","point-density-volume","legacy-piezo-hidden","legacy-cable-hidden","readout-hidden"};
    for(int i=0;i<25;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralPiezoLayeredSetup::unsupportedActualSource()
{
    QFETCH(int,mutation); auto s=config(); auto m=scene(); s.bindToModel(m); const auto accepted=s.sourceFingerprint;
    auto &p=m.getProblemSetup(); auto &interaction=p.getStructuralInteractionSetup(); auto &d=p.getStructuralDynamicsSetup();
    switch(mutation) {
    case 0:{ RLinearConstraint c; c.id="actual-mpc"; c.terms={{{0,RDofType::UX},1},{{1,RDofType::UX},-1}}; interaction.getLinearConstraints().push_back(c); break; }
    case 1:interaction.getCouplings().push_back({});break;
    case 2:{ RStructuralPrescribedMotion motion; motion.id="support-motion"; motion.target={0,RDofType::UX}; motion.knots={{0,0,0,0},{.1,.01,0,0}}; interaction.getPrescribedMotions().push_back(motion); break; }
    case 3:interaction.getSensors().push_back({});break;
    case 4:interaction.getActuators().push_back({});break;
    case 5:p.getStructuralNativeControlSpec().providerId="uncompiled-controller";break;
    case 6:p.getStructuralNativeControlState().controllerState="hidden-state";break;
    case 7:p.getStructuralControlSetup().enabled=true;break;
    case 8:p.getStructuralControlSetup().deterministicSeed=42;break;
    case 9:p.getStructuralThermalControlSetup().enabled=true;break;
    case 10:p.getStructuralThermalControlSetup().thermalSubstepsPerMechanicalMacrostep=2;break;
    case 11:p.getStructuralDynamicsRestartData().available=true;break;
    case 12:p.getStructuralDynamicsRestartData().q={0};break;
    case 13:p.getStructuralTrussDynamicsRestartData().available=true;break;
    case 14:p.getStructuralTrussDynamicsRestartData().q={0};break;
    case 15:d.setMethod(RStructuralTimeIntegrationMethod::GeneralizedAlpha);break;
    case 16:d.setMethod(RStructuralTimeIntegrationMethod::CentralDifference);break;
    case 17:d.setNewmarkBeta(.3);break;
    case 18:d.setNewmarkGamma(.6);break;
    case 19:d.setRhoInfinity(.9);break;
    case 20:case 21:{ RElement point(R_ELEMENT_POINT); point.setNodeId(0,0); m.addElement(point,true,0); m.setElementPhysicalId(1,777); m.getPoint(0).setVolume(.01);
        if(mutation==21) { RMaterial material(RMaterial::Solid); RMaterialProperty density(RMaterialProperty::Density); density.add(293.15,7800); material.add(density); m.getPoint(0).setMaterial(material); } break; }
    case 22:p.getStructuralPiezoSetup().ports.push_back({});break;
    case 23:p.getStructuralCableSetup().maxIterations=49;break;
    case 24:p.getStructuralPiezoReadoutSetup().ports={{"hidden"}};break;
    }
    QVERIFY_EXCEPTION_THROWN(s.validate(m),RError);
    QVERIFY_EXCEPTION_THROWN(s.computeSourceFingerprint(m),RError); // Unsupported inputs cannot receive the old source stamp.
    QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError); QCOMPARE(s.sourceFingerprint,accepted);
}
void TestStructuralPiezoLayeredSetup::staleModelRead_data()
{
    QTest::addColumn<bool>("binary"); QTest::addColumn<bool>("readout"); QTest::addColumn<bool>("physicalId");
    for(bool binary:{false,true}) for(bool readout:{false,true}) for(bool physicalId:{false,true})
        QTest::newRow(qPrintable(QString("%1-%2-%3").arg(binary?"rbm":"ram").arg(readout?"readout":"layered").arg(physicalId?"physical-id":"geometry")))<<binary<<readout<<physicalId;
}
void TestStructuralPiezoLayeredSetup::staleModelRead()
{
    QFETCH(bool,binary); QFETCH(bool,readout); QFETCH(bool,physicalId); QTemporaryDir dir; QVERIFY(dir.isValid());
    auto stale=readout?readoutScene():scene();
    if(!readout) { auto source=config(); source.bindToModel(stale); stale.getProblemSetup().setStructuralPiezoLayeredSetup(source); }
    if(physicalId) stale.setNodePhysicalId(1,999); else stale.getNode(1).setX(1.125);
    const auto path=dir.filePath("stale."+RModel::getDefaultFileExtension(binary)); stale.write(path,false);
    // The input envelope is valid, but its source fingerprint describes the earlier actual mesh.
    auto target=scene(true); target.setName("sentinel-target"); target.getNode(2).setX(3);
    auto targetSource=config(true); targetSource.bindToModel(target); target.getProblemSetup().setStructuralPiezoLayeredSetup(targetSource);
    const auto targetIdentity=targetSource.identityBytes(); const auto targetFingerprint=targetSource.sourceFingerprint;
    QVERIFY_EXCEPTION_THROWN(target.read(path),RError);
    QCOMPARE(target.getName(),QString("sentinel-target")); QCOMPARE(target.getNNodes(),4u); QCOMPARE(target.getNElements(),1u);
    QCOMPARE(target.getNode(2).getX(),3.0); QCOMPARE(target.getNodePhysicalId(1),112u); QCOMPARE(target.getElementPhysicalId(0),501u);
    QCOMPARE(target.getElement(0).getType(),R_ELEMENT_QUAD1);
    QCOMPARE(target.getProblemSetup().getStructuralPiezoLayeredSetup().identityBytes(),targetIdentity);
    QCOMPARE(target.getProblemSetup().getStructuralPiezoLayeredSetup().sourceFingerprint,targetFingerprint);
}
void TestStructuralPiezoLayeredSetup::workpoint()
{
    auto s=config(); auto m=scene(); s.workpoint.available=true;
    s.workpoint.nodes={{101,{{.01,0,0,0,0,0}},{{1,0,0,0,0,0}}},{112,{},{}}};
    s.workpoint.ports={{"drive",12,3e-9}}; s.validate(m);
    const auto bytes=s.identityBytes(); s.workpoint.nodes[1].q0[5]=.2; QVERIFY(bytes!=s.identityBytes());
    // Model accepts source values without claiming force/electrical equilibrium.
    s.validate(m); s.workpoint.nodes.pop_back(); QVERIFY_EXCEPTION_THROWN(s.validate(m),RError);
}
void TestStructuralPiezoLayeredSetup::dampingAuthorities()
{
    auto s=config(); s.damping.kind=RStructuralPiezoDampingKind::Rayleigh; s.damping.rayleighAlpha=.2; s.validate();
    s.damping={}; s.damping.kind=RStructuralPiezoDampingKind::UniformModal; s.damping.uniformRatio=.03; s.validate();
    s.damping={}; s.damping.kind=RStructuralPiezoDampingKind::PerMode; s.damping.ratios={.01,.02}; s.validate();
    s.damping.kind=RStructuralPiezoDampingKind::PerCluster; s.damping.clusterEnds={2,4}; s.validate();
    s.damping.clusterEnds={2,2}; QVERIFY_EXCEPTION_THROWN(s.validate(),RError);
    s.damping={}; s.damping.kind=RStructuralPiezoDampingKind::Imported; s.damping.importedDimension=2;
    s.damping.importedMatrix={1,0,0,1}; s.damping.sourceId="calibration-C"; s.damping.sourceSha256=QString(64,'c'); s.validate();
    s.damping.importedMatrix.pop_back(); QVERIFY_EXCEPTION_THROWN(s.validate(),RError);
}
void TestStructuralPiezoLayeredSetup::restartIsSeparate()
{
    const auto s=config(); const auto identity=s.identityBytes(); RStructuralPiezoLayeredRestart r; r.validate();
    r.available=true; r.sourceFingerprint=QString(64,'a'); r.stateBytes="opaque accepted state"; r.validate(); QCOMPARE(s.identityBytes(),identity);
    r.available=false; QVERIFY_EXCEPTION_THROWN(r.validate(),RError);
    r.available=true; r.stateBytes=QByteArray(16*1024*1024+1,'x'); QVERIFY_EXCEPTION_THROWN(r.validate(),RError);
}
void TestStructuralPiezoLayeredSetup::persistence_data()
{ QTest::addColumn<bool>("binary"); QTest::newRow("ascii")<<false; QTest::newRow("binary")<<true; }
void TestStructuralPiezoLayeredSetup::persistence()
{
    QFETCH(bool,binary); QTemporaryDir dir; QVERIFY(dir.isValid()); auto s=config(); auto m=scene();
    s.supports[0].value=-0.0; s.members[0].layers[1].coverage[0]=-.8;
    s.materials[0].notes=QString::fromUtf8("Explicit example\nquoted \"source\" \\ provenance 压电");
    s.excitationConfiguration=QByteArray("time\0source",11); s.bindToModel(m);
    m.getProblemSetup().setStructuralPiezoLayeredSetup(s);
    RStructuralPiezoLayeredRestart restart; restart.available=true; restart.sourceFingerprint=s.sourceFingerprint; restart.stateBytes=QByteArray("state\0bytes",11);
    m.getProblemSetup().setStructuralPiezoLayeredRestart(restart);
    const auto path=dir.filePath("layered."+RModel::getDefaultFileExtension(binary)); m.write(path,false);
    RModel restored; restored.read(path); const auto &actual=restored.getProblemSetup().getStructuralPiezoLayeredSetup();
    actual.validate(restored); QCOMPARE(actual.identityBytes(),s.identityBytes()); QCOMPARE(actual.sourceFingerprint,s.sourceFingerprint);
    QVERIFY(std::signbit(actual.supports[0].value)); QCOMPARE(restored.getProblemSetup().getStructuralPiezoLayeredRestart().stateBytes,restart.stateBytes);
    RProblemSetup copied=restored.getProblemSetup(); RProblemSetup assigned; assigned=copied;
    QCOMPARE(assigned.getStructuralPiezoLayeredSetup().identityBytes(),s.identityBytes());
    const auto typed=dir.filePath("typed");
    { RSaveFile out(typed,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,23)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(binary) RFileIO::writeBinary(out,s); else RFileIO::writeAscii(out,s,false); QVERIFY(out.commit()); }
    RFile in(typed,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,23)); QVERIFY(in.open(QIODevice::ReadOnly));
    RStructuralPiezoLayeredSetup read; if(binary) RFileIO::readBinary(in,read); else RFileIO::readAscii(in,read);
    QCOMPARE(read.identityBytes(),s.identityBytes());
}
void TestStructuralPiezoLayeredSetup::legacyAndDowngrade_data() { persistence_data(); }
void TestStructuralPiezoLayeredSetup::legacyAndDowngrade()
{
    QFETCH(bool,binary); QTemporaryDir dir; const auto path=dir.filePath("legacy"); RProblemSetup defaults;
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,22)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(binary) RFileIO::writeBinary(out,defaults); else RFileIO::writeAscii(out,defaults); QVERIFY(out.commit()); }
    RProblemSetup restored; restored.setStructuralPiezoLayeredSetup(config());
    { RFile in(path,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,22)); QVERIFY(in.open(QIODevice::ReadOnly));
      if(binary) RFileIO::readBinary(in,restored); else RFileIO::readAscii(in,restored); }
    QVERIFY(!restored.getStructuralPiezoLayeredSetup().hasNonDefaultData()); QVERIFY(!restored.getStructuralPiezoLayeredRestart().available);
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); const auto old=file.readAll(); file.close();
    RProblemSetup configured; configured.setStructuralPiezoLayeredSetup(config());
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,22)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(binary) { QVERIFY_EXCEPTION_THROWN(RFileIO::writeBinary(out,configured),RError); }
      else { QVERIFY_EXCEPTION_THROWN(RFileIO::writeAscii(out,configured),RError); } }
    QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(),old);
}
void TestStructuralPiezoLayeredSetup::malformedReader_data()
{
    QTest::addColumn<bool>("binary"); QTest::addColumn<int>("mutation");
    for(bool binary:{false,true}) for(int i=0;i<3;++i) QTest::newRow(qPrintable(QString("%1-%2").arg(binary?"binary":"ascii").arg(i)))<<binary<<i;
}
void TestStructuralPiezoLayeredSetup::malformedReader()
{
    QFETCH(bool,binary); QFETCH(int,mutation); QTemporaryDir dir; const auto path=dir.filePath("bad");
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,23)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(mutation==0) { auto s=config(); if(binary) RFileIO::writeBinary(out,s); else RFileIO::writeAscii(out,s); }
      else if(binary) { if(mutation==1) RFileIO::writeBinary(out,true); else { const char invalid=2; QCOMPARE(out.write(&invalid,1),qint64(1)); }
          if(mutation==1) RFileIO::writeBinary(out,99u); }
      else RFileIO::writeAscii(out,QString(mutation==1?"1 99":"2"));
      QVERIFY(out.commit()); }
    if(mutation==0) { QFile f(path); QVERIFY(f.open(QIODevice::ReadWrite)); QVERIFY(f.resize(f.size()/2)); }
    auto target=config(); const auto before=target.identityBytes(); RFile in(path,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,23)); QVERIFY(in.open(QIODevice::ReadOnly));
    if(binary) { QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,target),RError); }
    else { QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(in,target),RError); }
    QCOMPARE(target.identityBytes(),before);
}
QTEST_APPLESS_MAIN(TestStructuralPiezoLayeredSetup)
#include "tst_structural_piezo_layered_setup.moc"
