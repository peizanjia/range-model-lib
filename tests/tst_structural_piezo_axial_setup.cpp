#include <QtTest>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <type_traits>
#include "rml_model.h"
#include "rml_file_io.h"

static_assert(std::is_nothrow_move_assignable<RStructuralPiezoAxialSetup>::value,"Prepared axial source publication must not allocate.");
static_assert(std::is_nothrow_move_assignable<RStructuralPiezoAxialRestart>::value,"Prepared axial restart publication must not allocate.");
namespace {
RModel scene()
{
    RModel m; m.addNode(RNode(0,0,0)); m.addNode(RNode(1,0,0));
    m.setNodePhysicalId(0,101); m.setNodePhysicalId(1,112);
    RElement e(R_ELEMENT_TRUSS1); e.setNodeId(0,0); e.setNodeId(1,1); m.addElement(e,true,0); m.setElementPhysicalId(0,501);
    m.getLine(0).setCrossArea(.01); RMaterial material(RMaterial::Solid);
    RMaterialProperty density(RMaterialProperty::Density); density.add(293.15,7800); material.add(density); m.getLine(0).setMaterial(material);
    m.setProblemTaskTree(RProblemTaskItem::generateDefaultTree(R_PROBLEM_STRESS));
    m.getProblemSetup().setMassMatrixType(RMassMatrixType::Lumped);
    auto &d=m.getProblemSetup().getStructuralDynamicsSetup(); d.setEnabled(true); d.setTimeStep(.01); d.setNSteps(10); m.getTimeSolver().setInputStartTime(.01);
    auto &truss=m.getProblemSetup().getStructuralTrussDynamicsSetup(); truss.enabled=true;
    RStructuralTrussActiveMember active; active.physicalElementId=501; active.portId="drive"; active.referenceLength=1; active.axialStiffness=100; active.coupling=.1; active.capacitance=1e-8;
    truss.activeMembers={active}; m.getProblemSetup().getStructuralPiezoReadoutSetup().ports={{"drive"}};
    truss.bindToModel(m); return m;
}
RStructuralPiezoAxialSetup config()
{
    RStructuralPiezoAxialSetup s; s.enabled=true;
    s.recipeConfiguration="{\"damping\":{\"ratio\":-0},\"family\":\"axialRod\"}";
    return s;
}
template<class T> void writeRecord(RSaveFile &out,const T &value,bool binary)
{ if(binary) RFileIO::writeBinary(out,value); else RFileIO::writeAscii(out,value,false); }
template<class T> void readRecord(RFile &in,T &value,bool binary)
{ if(binary) RFileIO::readBinary(in,value); else RFileIO::readAscii(in,value); }
}
class TestStructuralPiezoAxialSetup:public QObject
{
    Q_OBJECT
private slots:
    void defaults(); void canonicalRecipe_data(); void canonicalRecipe();
    void bindingAndRawIdentity(); void typedSourceMutation_data(); void typedSourceMutation();
    void exclusivity_data(); void exclusivity(); void mixedCableMembers();
    void persistence_data(); void persistence();
    void oldFormat_data(); void oldFormat(); void restartBounds();
    void malformedReader_data(); void malformedReader(); void restartPair();
};
void TestStructuralPiezoAxialSetup::defaults()
{
    RStructuralPiezoAxialSetup s; s.validate(); s.validate(RModel());
    QVERIFY(!s.hasNonDefaultData()); QVERIFY(s.identityBytes().isEmpty()); QVERIFY(s.computeSourceFingerprint(RModel()).isEmpty());
    RStructuralPiezoAxialRestart r; r.validate(); QVERIFY(!r.hasNonDefaultData());
    s.recipeConfiguration="{}"; QVERIFY_EXCEPTION_THROWN(s.validate(),RError);
}
void TestStructuralPiezoAxialSetup::canonicalRecipe_data()
{
    QTest::addColumn<QByteArray>("json"); QTest::addColumn<bool>("valid");
    QTest::newRow("minimum")<<QByteArray("{\"family\":\"axialRod\"}")<<true;
    QTest::newRow("whole-declarations")<<QJsonDocument(QJsonObject{
        {"family","axialRod"},{"units",QJsonObject{{"length","m"}}},{"dt",.01},{"steps",10},{"replaceLegacyConditions",true},
        {"frame",QJsonObject{}},{"template",QJsonObject{}},{"controller",QJsonObject{{"kind","custom"}}},{"excitation",QJsonObject{}},
        {"design",QJsonObject{}},{"materials",QJsonArray{QJsonObject{{"id","host"}}}},
        {"members",QJsonArray{QJsonObject{{"physicalElementId",501}}}},{"supports",QJsonArray{}},
        {"backgroundForces",QJsonArray{}},{"initialDisplacements",QJsonArray{}},{"initialVelocities",QJsonArray{}},{"workpoint",QJsonObject{}},
        {"damping",QJsonObject{{"kind","modal"},{"period",.12345678901234567}}}}).toJson(QJsonDocument::Compact)<<true;
    QTest::newRow("negative-zero")<<config().recipeConfiguration<<true;
    const char *invalid[]={"[]","{}","{\"family\":\"beam\"}","{\"family\":\"axialRod\",\"unknown\":{}}",
        "{ \"family\":\"axialRod\"}","{\"family\":\"axialRod\",\"family\":\"axialRod\"}",
        "{\"family\":\"axialRod\",\"damping\":{}}","{\"controller\":[],\"family\":\"axialRod\"}",
        "{\"family\":\"axialRod\",\"members\":[]}","{\"family\":\"axialRod\",\"supports\":[1]}",
        "{\"damping\":{\"ratio\":1e999},\"family\":\"axialRod\"}","{\"damping\":{\"ratio\":NaN},\"family\":\"axialRod\"}",
        "{\"damping\":{\"ratio\":0.0},\"family\":\"axialRod\"}","{\"damping\":{\"ratio\":01},\"family\":\"axialRod\"}",
        "{\"family\":\"axialRod\"}true","{\"family\":\"axialRod\",\"steps\":1.5}",
        "{\"dt\":0,\"family\":\"axialRod\"}","{\"family\":\"axialRod\",\"replaceLegacyConditions\":0}",
        "{\"controller\":{\"text\":\"\\ud800\"},\"family\":\"axialRod\"}"};
    for(uint i=0;i<sizeof(invalid)/sizeof(*invalid);++i) QTest::newRow(qPrintable(QString("invalid-%1").arg(i)))<<QByteArray(invalid[i])<<false;
    QTest::newRow("depth-bound")<<(QByteArray("{\"controller\":{\"nested\":")+QByteArray(65,'[')+"0"+QByteArray(65,']')+"},\"family\":\"axialRod\"}")<<false;
    QTest::newRow("array-count-bound")<<(QByteArray("{\"controller\":{\"nested\":[")+QByteArray("0,").repeated(100000)+"0]},\"family\":\"axialRod\"}")<<false;
    QTest::newRow("string-bound")<<(QByteArray("{\"controller\":{\"text\":\"")+QByteArray(65537,'a')+"\"},\"family\":\"axialRod\"}")<<false;
    QTest::newRow("blob-bound")<<QByteArray(8*1024*1024+1,'a')<<false;
}
void TestStructuralPiezoAxialSetup::canonicalRecipe()
{
    QFETCH(QByteArray,json); QFETCH(bool,valid); auto s=config(); s.recipeConfiguration=json;
    if(valid) { s.validate(); QVERIFY(!s.identityBytes().isEmpty()); }
    else { QVERIFY_EXCEPTION_THROWN(s.validate(),RError); }
}
void TestStructuralPiezoAxialSetup::bindingAndRawIdentity()
{
    auto m=scene(); const auto trussIdentity=m.getProblemSetup().getStructuralTrussDynamicsSetup().sourceFingerprint;
    auto s=config(); s.bindToModel(m); s.validate(m); const auto identity=s.identityBytes(); const auto fingerprint=s.sourceFingerprint;
    m.getProblemSetup().setStructuralPiezoAxialSetup(s);
    QCOMPARE(m.getProblemSetup().getStructuralTrussDynamicsSetup().computeSourceFingerprint(m),trussIdentity);
    auto changed=s; changed.recipeConfiguration.replace(":-0}",":0}"); QVERIFY(changed.identityBytes()!=identity);
    QVERIFY_EXCEPTION_THROWN(changed.validate(m),RError); changed.bindToModel(m); QVERIFY(changed.sourceFingerprint!=fingerprint);
    auto &restart=m.getProblemSetup().getStructuralPiezoAxialRestart(); restart.available=true; restart.sourceFingerprint=fingerprint; restart.runtimeState=QByteArray("physical\0controller",19);
    QCOMPARE(s.computeSourceFingerprint(m),fingerprint); QCOMPARE(s.identityBytes(),identity);
    m.getProblemSetup().getStructuralTrussDynamicsSetup().sourceFingerprint.clear();
    QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError); QCOMPARE(s.sourceFingerprint,fingerprint); QCOMPARE(s.identityBytes(),identity);
}
void TestStructuralPiezoAxialSetup::typedSourceMutation_data()
{
    QTest::addColumn<int>("mutation"); const char *names[]={"geometry","typed-law","readout","time-grid","own-recipe"};
    for(int i=0;i<5;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralPiezoAxialSetup::typedSourceMutation()
{
    QFETCH(int,mutation); auto m=scene(); auto s=config(); s.bindToModel(m); const auto before=s.sourceFingerprint;
    auto &truss=m.getProblemSetup().getStructuralTrussDynamicsSetup();
    switch(mutation) {
    case 0:m.getNode(1).setX(1.01);break;
    case 1:truss.activeMembers[0].coupling=.2;break;
    case 2:m.getProblemSetup().getStructuralPiezoReadoutSetup().ports[0].readout=RStructuralPiezoReadoutMode::ActuatorCharge;break;
    case 3:m.getProblemSetup().getStructuralDynamicsSetup().setTimeStep(.02);break;
    case 4:s.recipeConfiguration="{\"controller\":{\"kind\":\"pi\"},\"family\":\"axialRod\"}";break;
    }
    QVERIFY_EXCEPTION_THROWN(s.validate(m),RError);
    // Own declaration changes can rebind against an unchanged bound parent.
    if(mutation==4) { s.bindToModel(m); }
    else { QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError); QCOMPARE(s.sourceFingerprint,before); truss.bindToModel(m); s.bindToModel(m); }
    QVERIFY(before!=s.sourceFingerprint); s.validate(m);
}
void TestStructuralPiezoAxialSetup::exclusivity_data()
{
    QTest::addColumn<int>("mutation"); const char *names[]={"unbound-parent","disabled-parent","layered","thermal","hidden-thermal","legacy-control","hidden-control","local-v3","hidden-native-state","legacy-restart","hidden-dynamics-restart","truss-restart","frozen-schedule","no-catalog","no-actuator","consistent-mass"};
    for(int i=0;i<16;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralPiezoAxialSetup::exclusivity()
{
    QFETCH(int,mutation); auto m=scene(); auto s=config(); auto &p=m.getProblemSetup();
    switch(mutation) {
    case 0:p.getStructuralTrussDynamicsSetup().sourceFingerprint.clear();break;
    case 1:p.getStructuralTrussDynamicsSetup().enabled=false;break;
    case 2:p.getStructuralPiezoLayeredSetup().enabled=true;break;
    case 3:p.getStructuralThermalControlSetup().enabled=true;break;
    case 4:p.getStructuralThermalControlSetup().thermalSubstepSeconds=.1;break;
    case 5:p.getStructuralControlSetup().enabled=true;break;
    case 6:p.getStructuralControlSetup().controllerId="old";break;
    case 7:p.getStructuralNativeControlSpec().source=RStructuralNativeControlSource::LocalV3;break;
    case 8:p.getStructuralNativeControlState().sourceFingerprint=QString(64,'a');break;
    case 9:p.getStructuralDynamicsRestartData().available=true;break;
    case 10:p.getStructuralDynamicsRestartData().q={0};break;
    case 11:p.getStructuralTrussDynamicsRestartData().available=true;break;
    case 12:p.getStructuralTrussDynamicsSetup().heldCommands={{1,{{"drive",1}},{},{}}};break;
    case 13:p.getStructuralPiezoReadoutSetup().ports.clear();break;
    case 14:p.getStructuralPiezoReadoutSetup().ports[0].role=RStructuralPiezoPortRole::Sensor;p.getStructuralPiezoReadoutSetup().ports[0].readout=RStructuralPiezoReadoutMode::ShortCircuitCharge;break;
    case 15:p.setMassMatrixType(RMassMatrixType::Consistent);break;
    }
    QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError); QVERIFY(s.sourceFingerprint.isEmpty());
}
void TestStructuralPiezoAxialSetup::mixedCableMembers()
{
    auto m=scene(); RElement cable(R_ELEMENT_TRUSS1); cable.setNodeId(0,0); cable.setNodeId(1,1);
    m.addElement(cable,true,0); m.setElementPhysicalId(1,502);
    RMaterialProperty modulus(RMaterialProperty::ModulusOfElasiticity); modulus.add(293.15,2e11); m.getLine(0).getMaterial().add(modulus);
    RStructuralTrussCableMember member; member.physicalElementId=502; member.initialReferenceLength=1;
    auto &truss=m.getProblemSetup().getStructuralTrussDynamicsSetup(); truss.cableMembers={member}; truss.bindToModel(m);
    auto s=config(); s.bindToModel(m); s.validate(m);
}
void TestStructuralPiezoAxialSetup::persistence_data()
{ QTest::addColumn<bool>("binary"); QTest::newRow("RAM")<<false; QTest::newRow("RBM")<<true; }
void TestStructuralPiezoAxialSetup::persistence()
{
    QFETCH(bool,binary); QTemporaryDir dir; QVERIFY(dir.isValid()); auto m=scene(); auto s=config(); s.bindToModel(m); m.getProblemSetup().setStructuralPiezoAxialSetup(s);
    RStructuralPiezoAxialRestart r; r.available=true; r.sourceFingerprint=s.sourceFingerprint; r.runtimeState=QByteArray("physical\0controller\xff",20); m.getProblemSetup().setStructuralPiezoAxialRestart(r);
    const auto path=dir.filePath("axial."+RModel::getDefaultFileExtension(binary)); m.write(path,false);
    RModel restored; restored.read(path); const auto &actual=restored.getProblemSetup().getStructuralPiezoAxialSetup(); actual.validate(restored);
    QCOMPARE(actual.recipeConfiguration,s.recipeConfiguration); QCOMPARE(actual.identityBytes(),s.identityBytes()); QCOMPARE(actual.sourceFingerprint,s.sourceFingerprint);
    QCOMPARE(restored.getProblemSetup().getStructuralPiezoAxialRestart().runtimeState,r.runtimeState);
    RProblemSetup copied(restored.getProblemSetup()),assigned; assigned=copied; QCOMPARE(assigned.getStructuralPiezoAxialSetup().recipeConfiguration,s.recipeConfiguration);
    RProblemSetup moved(std::move(assigned)); QCOMPARE(moved.getStructuralPiezoAxialRestart().runtimeState,r.runtimeState);
    const auto typed=dir.filePath("typed");
    { RSaveFile out(typed,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,24)); QVERIFY(out.open(QIODevice::WriteOnly)); writeRecord(out,s,binary); QVERIFY(out.commit()); }
    { RFile in(typed,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,24)); QVERIFY(in.open(QIODevice::ReadOnly)); RStructuralPiezoAxialSetup read; readRecord(in,read,binary); QCOMPARE(read.recipeConfiguration,s.recipeConfiguration); }
    // The opaque runtime supports its complete 32 MiB limit, above other native blobs' 16 MiB.
    r.runtimeState=QByteArray(32*1024*1024,'x');
    { RSaveFile out(typed,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,24)); QVERIFY(out.open(QIODevice::WriteOnly)); writeRecord(out,r,binary); QVERIFY(out.commit()); }
    { RFile in(typed,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,24)); QVERIFY(in.open(QIODevice::ReadOnly)); RStructuralPiezoAxialRestart read; readRecord(in,read,binary); QCOMPARE(read.runtimeState,r.runtimeState); }
}
void TestStructuralPiezoAxialSetup::oldFormat_data() { persistence_data(); }
void TestStructuralPiezoAxialSetup::oldFormat()
{
    QFETCH(bool,binary); QTemporaryDir dir; const auto path=dir.filePath("legacy"); RProblemSetup defaults;
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,23)); QVERIFY(out.open(QIODevice::WriteOnly)); writeRecord(out,defaults,binary); QVERIFY(out.commit()); }
    RProblemSetup restored; restored.setStructuralPiezoAxialSetup(config());
    { RFile in(path,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,23)); QVERIFY(in.open(QIODevice::ReadOnly)); readRecord(in,restored,binary); }
    QVERIFY(!restored.getStructuralPiezoAxialSetup().hasNonDefaultData()); QVERIFY(!restored.getStructuralPiezoAxialRestart().hasNonDefaultData());
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); const auto before=file.readAll(); file.close();
    RProblemSetup configured; configured.setStructuralPiezoAxialSetup(config());
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,23)); QVERIFY(out.open(QIODevice::WriteOnly)); QVERIFY_EXCEPTION_THROWN(writeRecord(out,configured,binary),RError); }
    QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(),before); file.close();
    RStructuralPiezoAxialRestart restart; restart.available=true; restart.sourceFingerprint=QString(64,'a'); restart.runtimeState="opaque";
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,23)); QVERIFY(out.open(QIODevice::WriteOnly)); QVERIFY_EXCEPTION_THROWN(writeRecord(out,restart,binary),RError); }
}
void TestStructuralPiezoAxialSetup::restartBounds()
{
    RStructuralPiezoAxialRestart r; r.available=true; r.sourceFingerprint=QString(64,'a'); r.runtimeState="opaque"; r.validate();
    r.sourceFingerprint=QString(64,'A'); QVERIFY_EXCEPTION_THROWN(r.validate(),RError); r.sourceFingerprint=QString(64,'a');
    r.runtimeState.clear(); QVERIFY_EXCEPTION_THROWN(r.validate(),RError); r.runtimeState="opaque";
    r.available=false; QVERIFY_EXCEPTION_THROWN(r.validate(),RError); r.available=true;
    r.runtimeState=QByteArray(32*1024*1024+1,'x'); QVERIFY_EXCEPTION_THROWN(r.validate(),RError);
}
void TestStructuralPiezoAxialSetup::malformedReader_data()
{
    QTest::addColumn<bool>("binary"); QTest::addColumn<int>("mutation");
    for(bool binary:{false,true}) for(int i=0;i<3;++i) QTest::newRow(qPrintable(QString("%1-%2").arg(binary).arg(i)))<<binary<<i;
}
void TestStructuralPiezoAxialSetup::malformedReader()
{
    QFETCH(bool,binary); QFETCH(int,mutation); QTemporaryDir dir; const auto path=dir.filePath("bad");
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,24)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(mutation==0) writeRecord(out,config(),binary);
      else if(binary) { if(mutation==1) { const char invalid=2; QCOMPARE(out.write(&invalid,1),qint64(1)); } else { RFileIO::writeBinary(out,true); RFileIO::writeBinary(out,65u); } }
      else RFileIO::writeAscii(out,QString(mutation==1?"2":"1 \"\" \"gg\"")); QVERIFY(out.commit()); }
    if(mutation==0) { QFile file(path); QVERIFY(file.open(QIODevice::ReadWrite)); QVERIFY(file.resize(file.size()/2)); }
    auto target=config(); const auto before=target.identityBytes();
    RFile in(path,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,24)); QVERIFY(in.open(QIODevice::ReadOnly));
    QVERIFY_EXCEPTION_THROWN(readRecord(in,target,binary),RError); QCOMPARE(target.identityBytes(),before);
}
void TestStructuralPiezoAxialSetup::restartPair()
{
    auto m=scene(); auto s=config(); s.bindToModel(m); RProblemSetup p=m.getProblemSetup(); p.setStructuralPiezoAxialSetup(s);
    RStructuralPiezoAxialRestart r; r.available=true; r.sourceFingerprint=QString(64,'a'); r.runtimeState="sealed"; p.setStructuralPiezoAxialRestart(r);
    QTemporaryDir dir; for(bool binary:{false,true}) {
        const auto path=dir.filePath(binary?"bad-binary":"bad-ascii");
        { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,24)); QVERIFY(out.open(QIODevice::WriteOnly)); QVERIFY_EXCEPTION_THROWN(writeRecord(out,p,binary),RError); }
        auto valid=p; valid.getStructuralPiezoAxialRestart().sourceFingerprint=s.sourceFingerprint;
        { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,24)); QVERIFY(out.open(QIODevice::WriteOnly)); writeRecord(out,valid,binary); QVERIFY(out.commit()); }
        // Tamper only the final source reference, retaining valid record syntax.
        { QFile file(path); QVERIFY(file.open(QIODevice::ReadWrite)); QByteArray bytes=file.readAll();
          const QByteArray needle=binary?s.sourceFingerprint.toLatin1():s.sourceFingerprint.toLatin1().toHex();
          const int start=bytes.lastIndexOf(needle); QVERIFY(start>=0); bytes.replace(start,needle.size(),binary?QByteArray(64,'b'):QByteArray(64,'b').toHex());
          QVERIFY(file.seek(0)); QCOMPARE(file.write(bytes),qint64(bytes.size())); }
        RProblemSetup target; target.setStructuralPiezoAxialSetup(config()); const auto before=target.getStructuralPiezoAxialSetup().recipeConfiguration;
        RFile in(path,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,24)); QVERIFY(in.open(QIODevice::ReadOnly));
        QVERIFY_EXCEPTION_THROWN(readRecord(in,target,binary),RError); QCOMPARE(target.getStructuralPiezoAxialSetup().recipeConfiguration,before);
    }
}
QTEST_APPLESS_MAIN(TestStructuralPiezoAxialSetup)
#include "tst_structural_piezo_axial_setup.moc"
