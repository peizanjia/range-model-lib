#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <cmath>
#include <limits>
#include "rml_model.h"
#include "rml_file_io.h"
#include "rml_structural_piezo_readout_setup.h"

namespace {
RModel scene()
{
    RModel model;
    for(uint i=0;i<3;++i) { model.addNode(RNode(double(i),0,0)); model.setNodePhysicalId(i,101+11*i); }
    auto &truss=model.getProblemSetup().getStructuralTrussDynamicsSetup(); truss.enabled=true;
    for(uint i=0;i<2;++i) {
        RElement e(R_ELEMENT_TRUSS1); e.setNodeId(0,i); e.setNodeId(1,i+1);
        model.addElement(e,true,0); model.setElementPhysicalId(i,501+13*i);
        RStructuralTrussActiveMember member; member.physicalElementId=501+13*i;
        member.portId=i==0?"drive":"sense"; member.referenceLength=1; member.axialStiffness=100;
        member.coupling=0.25; member.capacitance=2e-8;
        if(i==1) { member.kind=RStructuralTrussElectricalKind::ConductorCharge; member.initialValue=3e-9; }
        truss.activeMembers.push_back(member);
    }
    RMaterial material(RMaterial::Solid);
    RMaterialProperty density(RMaterialProperty::Density); density.add(293.15,7800); material.add(density);
    model.getLine(0).setMaterial(material); model.getLine(0).setCrossArea(0.001);
    auto &dynamics=model.getProblemSetup().getStructuralDynamicsSetup();
    dynamics.setEnabled(true); dynamics.setTimeStep(0.01); dynamics.setNSteps(10); dynamics.setOutputFrequency(1);
    model.getTimeSolver().setInputStartTime(0.01);
    std::vector<double> times;
    for(uint i=1;i<=dynamics.getNSteps();++i) times.push_back(double(i)*dynamics.getTimeStep());
    model.getTimeSolver().setTimes(times);
    return model;
}
RStructuralPiezoReadoutSetup config()
{
    RStructuralPiezoReadoutSetup s;
    RStructuralPiezoReadoutPort drive; drive.portId="drive";
    RStructuralPiezoReadoutPort sense; sense.portId="sense";
    sense.role=RStructuralPiezoPortRole::Sensor; sense.readout=RStructuralPiezoReadoutMode::OpenCircuitVoltage;
    s.ports={drive,sense}; return s;
}
RStructuralPiezoReadoutSetup workpoint()
{
    auto s=config(); s.workpoint.available=true;
    for(uint i=0;i<3;++i) s.workpoint.nodes.push_back({101+11*i,{{double(i)*0.01,0,0}},{{double(i),0,0}}});
    s.workpoint.ports={{"drive",12,1.2345678901234567e-8},{"sense",2,3e-9}};
    return s;
}
}

class TestStructuralPiezoReadoutSetup : public QObject
{
    Q_OBJECT
private slots:
    void legacyDefault();
    void supportedModes();
    void invalidCatalog_data(); void invalidCatalog();
    void fixedSensorInputs_data(); void fixedSensorInputs();
    void explicitWorkpoint_data(); void explicitWorkpoint();
    void sourceIdentity();
    void outputUnits();
    void persistence_data(); void persistence();
    void legacyAndDowngrade_data(); void legacyAndDowngrade();
    void malformedReader_data(); void malformedReader();
    void malformedOuterReader_data(); void malformedOuterReader();
    void boundSourceFingerprint();
};

void TestStructuralPiezoReadoutSetup::legacyDefault()
{
    RStructuralPiezoReadoutSetup s; s.validate(); s.validate(RModel());
    QVERIFY(!s.hasNonDefaultData()); QVERIFY(s.identityBytes().isEmpty());
    auto model=scene(); auto &truss=model.getProblemSetup().getStructuralTrussDynamicsSetup();
    truss.heldCommands={{1,{{"sense",8}},{},{}}};
    s.validate(model); // Legacy charge actuators retain their pre-catalog semantics.
}

void TestStructuralPiezoReadoutSetup::supportedModes()
{
    auto model=scene(); auto s=config(); s.validate(model);
    QCOMPARE(model.getProblemSetup().getStructuralTrussDynamicsSetup().activeMembers[1].initialValue,3e-9);
    auto &drive=s.ports[0]; drive.readout=RStructuralPiezoReadoutMode::ActuatorCharge; s.validate(model);
    drive.role=RStructuralPiezoPortRole::ActuatorSensor; s.validate(model);
    auto &sense=s.ports[1]; auto &member=model.getProblemSetup().getStructuralTrussDynamicsSetup().activeMembers[1];
    member.kind=RStructuralTrussElectricalKind::Voltage; member.initialValue=0;
    sense.readout=RStructuralPiezoReadoutMode::ShortCircuitCharge; s.validate(model);
    sense.readout=RStructuralPiezoReadoutMode::IdealChargeAmplifier; sense.Cf=2e-9;
    sense.Qref=3e-9; sense.VoutBias=0.25; s.validate(model);
    QCOMPARE(sense.amplifierPolarity,-1); sense.amplifierPolarity=1; s.validate(model);
}

void TestStructuralPiezoReadoutSetup::invalidCatalog_data()
{
    QTest::addColumn<int>("mutation");
    const char *names[]={"unknown","duplicate","missing","extra","empty-id","padded-id","quote-id","nul-id",
        "bad-role","bad-mode","sensor-none","actuator-open","combined-none","zero-cf","negative-cf","nan-cf",
        "nan-qref","infinite-bias","bad-polarity","irrelevant-cf","irrelevant-qref","irrelevant-bias",
        "irrelevant-polarity","charge-actuator","disabled-truss","duplicate-active","invalid-active-kind","nonfinite-charge"};
    for(int i=0;i<28;++i) QTest::newRow(names[i])<<i;
}

void TestStructuralPiezoReadoutSetup::invalidCatalog()
{
    QFETCH(int,mutation); auto model=scene(); auto s=config(); auto &truss=model.getProblemSetup().getStructuralTrussDynamicsSetup();
    auto &p=s.ports[1];
    switch(mutation) {
    case 0:p.portId="unknown";break;
    case 1:p.portId="drive";break;
    case 2:s.ports.pop_back();break;
    case 3:s.ports.push_back({"extra"});break;
    case 4:p.portId.clear();break;
    case 5:p.portId=" sense";break;
    case 6:p.portId="sen\"se";break;
    case 7:p.portId=QString::fromLatin1("sen\0se",6);break;
    case 8:p.role=static_cast<RStructuralPiezoPortRole>(99);break;
    case 9:p.readout=static_cast<RStructuralPiezoReadoutMode>(99);break;
    case 10:p.readout=RStructuralPiezoReadoutMode::None;break;
    case 11:p.role=RStructuralPiezoPortRole::Actuator;break;
    case 12:s.ports[0].role=RStructuralPiezoPortRole::ActuatorSensor;break;
    case 13:p.readout=RStructuralPiezoReadoutMode::IdealChargeAmplifier;break;
    case 14:p.readout=RStructuralPiezoReadoutMode::IdealChargeAmplifier;p.Cf=-1;break;
    case 15:p.Cf=std::numeric_limits<double>::quiet_NaN();break;
    case 16:p.Qref=std::numeric_limits<double>::quiet_NaN();break;
    case 17:p.VoutBias=std::numeric_limits<double>::infinity();break;
    case 18:p.amplifierPolarity=0;break;
    case 19:p.Cf=1;break;
    case 20:p.Qref=1;break;
    case 21:p.VoutBias=1;break;
    case 22:p.amplifierPolarity=1;break;
    case 23:truss.activeMembers[0].kind=RStructuralTrussElectricalKind::ConductorCharge;break;
    case 24:truss.enabled=false;break;
    case 25:truss.activeMembers[1].portId="drive";break;
    case 26:truss.activeMembers[1].kind=static_cast<RStructuralTrussElectricalKind>(99);break;
    case 27:truss.activeMembers[1].initialValue=std::numeric_limits<double>::infinity();break;
    }
    QVERIFY_EXCEPTION_THROWN(s.validate(model),RError);
}

void TestStructuralPiezoReadoutSetup::fixedSensorInputs_data()
{
    QTest::addColumn<int>("mode"); QTest::addColumn<int>("mutation");
    for(int mode=1;mode<=3;++mode) for(int mutation=0;mutation<5;++mutation)
        QTest::newRow(qPrintable(QString("mode-%1-invalid-%2").arg(mode).arg(mutation)))<<mode<<mutation;
}

void TestStructuralPiezoReadoutSetup::fixedSensorInputs()
{
    QFETCH(int,mode); QFETCH(int,mutation); auto model=scene(); auto s=config();
    auto &truss=model.getProblemSetup().getStructuralTrussDynamicsSetup(); auto &member=truss.activeMembers[1];
    s.ports[1].readout=static_cast<RStructuralPiezoReadoutMode>(mode);
    if(mode!=1) { member.kind=RStructuralTrussElectricalKind::Voltage; member.initialValue=0; }
    if(mode==3) s.ports[1].Cf=1e-9;
    s.validate(model);
    switch(mutation) {
    case 0:member.kind=mode==1?RStructuralTrussElectricalKind::Voltage:RStructuralTrussElectricalKind::ConductorCharge;break;
    case 1:truss.heldCommands={{1,{{"sense",member.initialValue}},{},{}}};break;
    case 2: {
        RStructuralNativeControlAction action; action.actionId="action"; action.portId="sense";
        action.quantity=mode==1?RStructuralNativeActionQuantity::Charge:RStructuralNativeActionQuantity::Voltage;
        model.getProblemSetup().getStructuralNativeControlSpec().actions={action}; break;
    }
    case 3:if(mode==1) member.initialValue=std::numeric_limits<double>::quiet_NaN();else member.initialValue=1e-9;break;
    case 4:truss.heldCommands={{1,{}, {}, {}},{2,{{"sense",0}}, {}, {}}};break;
    }
    QVERIFY_EXCEPTION_THROWN(s.validate(model),RError);
}

void TestStructuralPiezoReadoutSetup::explicitWorkpoint_data()
{
    QTest::addColumn<int>("mutation");
    const char *names[]={"valid-unbalanced-source","missing-node","unknown-node","duplicate-node","reserved-node",
        "nonfinite-q","nonfinite-force","missing-port","extra-port","unknown-port","duplicate-port","nan-v",
        "infinite-q","hidden-unavailable","changed-model-node","workpoint-only"};
    for(int i=0;i<16;++i) QTest::newRow(names[i])<<i;
}

void TestStructuralPiezoReadoutSetup::explicitWorkpoint()
{
    QFETCH(int,mutation); auto model=scene(); auto s=workpoint();
    switch(mutation) {
    case 0:break; // Deliberately unbalanced: the model checks no solver equilibrium residual.
    case 1:s.workpoint.nodes.pop_back();break;
    case 2:s.workpoint.nodes[0].physicalNodeId=999;break;
    case 3:s.workpoint.nodes[0].physicalNodeId=112;break;
    case 4:s.workpoint.nodes[0].physicalNodeId=RConstants::eod;break;
    case 5:s.workpoint.nodes[0].q0[1]=std::numeric_limits<double>::quiet_NaN();break;
    case 6:s.workpoint.nodes[0].f0[2]=std::numeric_limits<double>::infinity();break;
    case 7:s.workpoint.ports.pop_back();break;
    case 8:s.workpoint.ports.push_back({"extra",0,0});break;
    case 9:s.workpoint.ports[0].portId="unknown";break;
    case 10:s.workpoint.ports[0].portId="sense";break;
    case 11:s.workpoint.ports[0].V0=std::numeric_limits<double>::quiet_NaN();break;
    case 12:s.workpoint.ports[0].Q0=std::numeric_limits<double>::infinity();break;
    case 13:s.workpoint.available=false;break;
    case 14:model.setNodePhysicalId(0,999);break;
    case 15:s.ports.clear();break;
    }
    if(mutation==0 || mutation==15) { s.validate(model); QVERIFY(s.hasNonDefaultData()); }
    else { QVERIFY_EXCEPTION_THROWN(s.validate(model),RError); }
}

void TestStructuralPiezoReadoutSetup::sourceIdentity()
{
    auto model=scene(); auto s=workpoint(); const auto bytes=s.identityBytes();
    auto copy=s; QCOMPARE(copy.identityBytes(),bytes);
    copy.workpoint.nodes[0].q0[0]=-0.0; QVERIFY(copy.identityBytes()!=bytes);
    copy=s; copy.workpoint.ports[0].Q0=std::nextafter(copy.workpoint.ports[0].Q0,1.0); QVERIFY(copy.identityBytes()!=bytes);
    copy=s; std::swap(copy.ports[0],copy.ports[1]); QVERIFY(copy.identityBytes()!=bytes);
    copy=s; copy.ports[0].readout=RStructuralPiezoReadoutMode::ActuatorCharge; QVERIFY(copy.identityBytes()!=bytes);
    copy=s; copy.ports[1].Cf=-0.0; QVERIFY(copy.identityBytes()!=bytes);
    model.getTimeSolver().setCurrentTimeStep(2); model.getTimeSolver().setComputedTime(0.5);
    const uint position=model.addVariable(RVariable(R_VARIABLE_DISPLACEMENT)); model.getVariable(position).resize(1,model.getNNodes());
    model.getVariable(position).setValue(0,0,99); s.validate(model); QCOMPARE(s.identityBytes(),bytes);
    // A stale fingerprint cannot recurse through validate(model).
    model.getProblemSetup().getStructuralTrussDynamicsSetup().sourceFingerprint="deliberately-stale"; s.validate(model);
}

void TestStructuralPiezoReadoutSetup::outputUnits()
{
    QCOMPARE(rStructuralPiezoReadoutUnit(RStructuralPiezoReadoutMode::None),QString());
    QCOMPARE(rStructuralPiezoReadoutUnit(RStructuralPiezoReadoutMode::OpenCircuitVoltage),QString("V"));
    QCOMPARE(rStructuralPiezoReadoutUnit(RStructuralPiezoReadoutMode::IdealChargeAmplifier),QString("V"));
    QCOMPARE(rStructuralPiezoReadoutUnit(RStructuralPiezoReadoutMode::ShortCircuitCharge),QString("C"));
    QCOMPARE(rStructuralPiezoReadoutUnit(RStructuralPiezoReadoutMode::ActuatorCharge),QString("C"));
    QVERIFY_EXCEPTION_THROWN(rStructuralPiezoReadoutUnit(static_cast<RStructuralPiezoReadoutMode>(99)),RError);
}

void TestStructuralPiezoReadoutSetup::persistence_data()
{
    QTest::addColumn<bool>("binary"); QTest::newRow("ascii")<<false; QTest::newRow("binary")<<true;
}

void TestStructuralPiezoReadoutSetup::persistence()
{
    QFETCH(bool,binary); QTemporaryDir directory; QVERIFY(directory.isValid());
    auto model=scene(); auto s=workpoint(); s.workpoint.nodes[0].q0[0]=-0.0;
    s.workpoint.ports[1].V0=-0.0; s.ports[0].Qref=-0.0;
    model.getProblemSetup().setStructuralPiezoReadoutSetup(s);
    model.getProblemSetup().getStructuralTrussDynamicsSetup().bindToModel(model);
    const auto path=directory.filePath("readout."+RModel::getDefaultFileExtension(binary)); model.write(path,false);
    RModel restored; restored.read(path);
    const auto &actual=restored.getProblemSetup().getStructuralPiezoReadoutSetup(); actual.validate(restored);
    QCOMPARE(actual.identityBytes(),s.identityBytes()); QVERIFY(std::signbit(actual.workpoint.nodes[0].q0[0]));
    restored.getProblemSetup().getStructuralTrussDynamicsSetup().validate(restored);
    RProblemSetup copied=restored.getProblemSetup(); QCOMPARE(copied.getStructuralPiezoReadoutSetup().identityBytes(),s.identityBytes());
    RProblemSetup assigned; assigned=copied; QCOMPARE(assigned.getStructuralPiezoReadoutSetup().identityBytes(),s.identityBytes());
    const auto typed=directory.filePath("typed");
    { RSaveFile out(typed,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,22)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(binary) RFileIO::writeBinary(out,s); else RFileIO::writeAscii(out,s,false); QVERIFY(out.commit()); }
    RFile in(typed,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,22)); QVERIFY(in.open(QIODevice::ReadOnly));
    RStructuralPiezoReadoutSetup read;
    if(binary) RFileIO::readBinary(in,read); else RFileIO::readAscii(in,read);
    QCOMPARE(read.identityBytes(),s.identityBytes());
}

void TestStructuralPiezoReadoutSetup::legacyAndDowngrade_data() { persistence_data(); }
void TestStructuralPiezoReadoutSetup::legacyAndDowngrade()
{
    QFETCH(bool,binary); QTemporaryDir directory; const auto path=directory.filePath("legacy"); RProblemSetup defaults;
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,21)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(binary) RFileIO::writeBinary(out,defaults); else RFileIO::writeAscii(out,defaults); QVERIFY(out.commit()); }
    RProblemSetup restored; restored.setStructuralPiezoReadoutSetup(workpoint());
    { RFile in(path,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,21)); QVERIFY(in.open(QIODevice::ReadOnly));
      if(binary) RFileIO::readBinary(in,restored); else RFileIO::readAscii(in,restored); }
    QVERIFY(!restored.getStructuralPiezoReadoutSetup().hasNonDefaultData());
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); const auto old=file.readAll(); file.close();
    for(int mutation=0;mutation<3;++mutation) {
        RProblemSetup hidden; auto &readout=hidden.getStructuralPiezoReadoutSetup();
        if(mutation==0) readout=config();
        if(mutation==1) readout.workpoint.available=true;
        if(mutation==2) readout.workpoint.ports={{"hidden",0,0}};
        { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,21)); QVERIFY(out.open(QIODevice::WriteOnly));
          if(binary) { QVERIFY_EXCEPTION_THROWN(RFileIO::writeBinary(out,hidden),RError); }
          else { QVERIFY_EXCEPTION_THROWN(RFileIO::writeAscii(out,hidden),RError); } }
        QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(),old); file.close();
    }
}

void TestStructuralPiezoReadoutSetup::malformedReader_data()
{
    QTest::addColumn<bool>("binary"); QTest::addColumn<int>("mutation");
    for(bool binary:{false,true}) for(int mutation=0;mutation<6;++mutation)
        QTest::newRow(qPrintable(QString("%1-invalid-%2").arg(binary?"binary":"ascii").arg(mutation)))<<binary<<mutation;
}

void TestStructuralPiezoReadoutSetup::malformedReader()
{
    QFETCH(bool,binary); QFETCH(int,mutation); QTemporaryDir directory; const auto path=directory.filePath("bad");
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,22)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(mutation==0) { const auto s=workpoint(); if(binary) RFileIO::writeBinary(out,s); else RFileIO::writeAscii(out,s); }
      else if(binary) {
          RFileIO::writeBinary(out,mutation==1?1000001u:(mutation==5?0u:1u));
          if(mutation==5) { const char invalid=2; QCOMPARE(out.write(&invalid,1),qint64(1)); }
          else if(mutation>1) {
              RFileIO::writeBinary(out,QString("drive"));
              RFileIO::writeBinary(out,mutation==2?99u:0u); RFileIO::writeBinary(out,mutation==3?99u:0u);
              if(mutation==4) { RFileIO::writeBinary(out,0.0); RFileIO::writeBinary(out,0); RFileIO::writeBinary(out,0.0); RFileIO::writeBinary(out,0.0);
                  RFileIO::writeBinary(out,false); RFileIO::writeBinary(out,0u); RFileIO::writeBinary(out,0u); }
          }
      } else {
          const char *records[]={"", "1000001", "1 \"drive\" 99 0", "1 \"drive\" 0 99", "1 \"drive\" 0 0 0 0 0 0 0 0 0", "0 2 0 0"};
          RFileIO::writeAscii(out,QString::fromLatin1(records[mutation]));
      }
      QVERIFY(out.commit()); }
    if(mutation==0) { QFile file(path); QVERIFY(file.open(QIODevice::ReadWrite)); QVERIFY(file.resize(file.size()/2)); }
    auto target=config(); const auto before=target.identityBytes();
    RFile in(path,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,22)); QVERIFY(in.open(QIODevice::ReadOnly));
    if(binary) { QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,target),RError); }
    else { QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(in,target),RError); }
    QCOMPARE(target.identityBytes(),before);
}

void TestStructuralPiezoReadoutSetup::malformedOuterReader_data() { persistence_data(); }
void TestStructuralPiezoReadoutSetup::malformedOuterReader()
{
    QFETCH(bool,binary); QTemporaryDir directory; const auto path=directory.filePath("outer");
    RProblemSetup original; original.setStructuralPiezoReadoutSetup(workpoint());
    { RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII); out.setVersion(RVersion(1,3,22)); QVERIFY(out.open(QIODevice::WriteOnly));
      if(binary) RFileIO::writeBinary(out,original); else RFileIO::writeAscii(out,original); QVERIFY(out.commit()); }
    { QFile file(path); QVERIFY(file.open(QIODevice::ReadWrite));
      if(binary) { QVERIFY(file.resize(file.size()-5)); }
      else { const auto bytes=file.readAll(); const auto lastLine=bytes.lastIndexOf('\n',bytes.size()-2);
          QVERIFY(lastLine>=0); QVERIFY(file.resize(lastLine+1)); } }
    RProblemSetup target; target.setStructuralPiezoReadoutSetup(config()); const auto before=target.getStructuralPiezoReadoutSetup().identityBytes();
    RFile in(path,binary?RFile::BINARY:RFile::ASCII); in.setVersion(RVersion(1,3,22)); QVERIFY(in.open(QIODevice::ReadOnly));
    if(binary) { QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,target),RError); }
    else { QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(in,target),RError); }
    QCOMPARE(target.getStructuralPiezoReadoutSetup().identityBytes(),before);
}

void TestStructuralPiezoReadoutSetup::boundSourceFingerprint()
{
    auto model=scene(); auto &truss=model.getProblemSetup().getStructuralTrussDynamicsSetup(); truss.bindToModel(model);
    const auto legacy=truss.sourceFingerprint;
    model.getProblemSetup().setStructuralPiezoReadoutSetup({}); QCOMPARE(truss.computeSourceFingerprint(model),legacy);
    model.getProblemSetup().setStructuralPiezoReadoutSetup(config()); QVERIFY(truss.computeSourceFingerprint(model)!=legacy);
    QVERIFY_EXCEPTION_THROWN(truss.validate(model),RError); truss.bindToModel(model); const auto readoutHash=truss.sourceFingerprint;
    auto s=workpoint(); model.getProblemSetup().setStructuralPiezoReadoutSetup(s); QVERIFY(truss.computeSourceFingerprint(model)!=readoutHash);
    truss.bindToModel(model); const auto workpointHash=truss.sourceFingerprint;
    s.workpoint.nodes[0].q0[0]=-0.0; model.getProblemSetup().setStructuralPiezoReadoutSetup(s);
    QVERIFY(truss.computeSourceFingerprint(model)!=workpointHash);
    const auto before=truss.sourceFingerprint; model.getProblemSetup().getStructuralPiezoReadoutSetup().ports[1].portId="unknown";
    QVERIFY_EXCEPTION_THROWN(truss.bindToModel(model),RError); QCOMPARE(truss.sourceFingerprint,before);
}

QTEST_APPLESS_MAIN(TestStructuralPiezoReadoutSetup)
#include "tst_structural_piezo_readout_setup.moc"
