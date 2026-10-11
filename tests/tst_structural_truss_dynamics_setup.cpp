#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>
#include "rml_model.h"
#include "rml_file_io.h"

namespace {
RModel scene()
{
    RModel m;
    for (uint i=0;i<4;++i) {
        m.addNode(RNode(double(i)*1.2345678901234567,0.0,0.0));
        m.setNodePhysicalId(i,101+11*i);
    }
    for (uint i=0;i<3;++i) {
        RElement e(R_ELEMENT_TRUSS1); e.setNodeId(0,i); e.setNodeId(1,i+1);
        m.addElement(e,true,0); m.setElementPhysicalId(i,501+13*i);
    }
    RMaterial mat(RMaterial::Solid);
    for (auto entry : {std::pair{RMaterialProperty::Density,7890.1234567890123},
                       std::pair{RMaterialProperty::ModulusOfElasiticity,2.1234567890123456e11}}) {
        RMaterialProperty p(entry.first); p.add(293.15,entry.second); p.add(400.1234567890123,entry.second*0.875);
        mat.add(p);
    }
    m.getLine(0).setCrossArea(0.012345678901234567); m.getLine(0).setMaterial(mat);
    auto &d=m.getProblemSetup().getStructuralDynamicsSetup();
    d.setEnabled(true); d.setTimeStep(0.012345678901234567); d.setNSteps(20); d.setOutputFrequency(2);
    m.getTimeSolver().setInputStartTime(d.getTimeStep());
    m.getTimeSolver().setTimes({0.01,0.02,0.03,0.04,0.05});
    return m;
}
RStructuralTrussDynamicsSetup config()
{
    RStructuralTrussDynamicsSetup s; s.enabled=true;
    s.passiveMembers={{501,1.2345678901234567}};
    RStructuralTrussActiveMember a; a.physicalElementId=514; a.portId="isolated electrode";
    a.referenceLength=1.2345678901234567; a.axialStiffness=12345.678901234567;
    a.coupling=-0.12345678901234567; a.capacitance=1.2345678901234567e-8;
    a.initialValue=-12.345678901234567; s.activeMembers={a};
    s.cableMembers={{527,1.1234567890123457,2.345678901234567}};
    s.heldCommands={{1,{{a.portId,13.234567890123457}},{{527,1.1111111111111112}},
                     {{134,{{1.2345678901234567,-2.345678901234567,3.456789012345678}}}}},
                    {7,{{a.portId,-3.456789012345678}},{},{}}};
    return s;
}
RStructuralTrussDynamicsRestartData restart()
{
    RStructuralTrussDynamicsRestartData r; r.available=true; r.completedSteps=7; r.runTargetSteps=20;
    r.initialTime=0.125; r.fixedTimeStep=0.012345678901234567; r.time=r.initialTime+7*r.fixedTimeStep;
    r.nextCommandIndex=2; r.orderedPhysicalNodeIds={101,112}; r.orderedMemberPhysicalIds={501,514,527,540};
    r.orderedPortIds={"voltage electrode","charge electrode"}; r.sourceFingerprint=QString(64,'a');
    r.portVoltages={-1.2345678901234567,0.23456789012345678};
    r.portCharges={2.345678901234567e-8,-3.456789012345678e-9};
    r.portEnthalpies={-4.567890123456789,5.67890123456789};
    r.systemIdentity=QByteArray(32,'b');
    r.q={1.2345678901234567,2,3,4,5,6}; r.v={-1.2345678901234567,2,3,4,5,6};
    r.a={0.23456789012345678,2,3,4,5,6}; r.reactions={9.8765432109876543,2,3,4,5,6};
    r.externalForce={7.6543210987654321,2,3,4,5,6}; r.memberInputKinds={0,1,3,2};
    r.memberInputValues={0,-12.345678901234567,1.2345678901234567,2.345678901234567e-6};
    r.storedEnergy=1.2345678901234567; r.kineticEnergy=2.345678901234567; r.initialTotalEnergy=3.456789012345678;
    r.cumulativeExternalWork=-4.567890123456789; r.cumulativeSupportWork=5.67890123456789;
    r.cumulativeElectricalWork=-6.789012345678901; r.cumulativeRestLengthWork=7.890123456789012;
    return r;
}
void equalRestart(const RStructuralTrussDynamicsRestartData &a,const RStructuralTrussDynamicsRestartData &b)
{
#define FIELD(f) QVERIFY(a.f==b.f)
    FIELD(available); FIELD(completedSteps); FIELD(runTargetSteps); FIELD(initialTime); FIELD(time); FIELD(fixedTimeStep);
    FIELD(nextCommandIndex); FIELD(orderedPhysicalNodeIds); FIELD(orderedMemberPhysicalIds); FIELD(orderedPortIds);
    FIELD(portVoltages); FIELD(portCharges); FIELD(portEnthalpies);
    FIELD(sourceFingerprint); FIELD(systemIdentity); FIELD(q); FIELD(v); FIELD(a); FIELD(reactions); FIELD(externalForce);
    FIELD(memberInputKinds); FIELD(memberInputValues); FIELD(storedEnergy); FIELD(kineticEnergy); FIELD(initialTotalEnergy);
    FIELD(cumulativeExternalWork); FIELD(cumulativeSupportWork); FIELD(cumulativeElectricalWork); FIELD(cumulativeRestLengthWork);
#undef FIELD
    // Equality alone does not distinguish the IEEE signed-zero encodings.
#define BITS(f) QVERIFY(std::memcmp(&a.f,&b.f,sizeof(double))==0)
    BITS(initialTime);BITS(time);BITS(fixedTimeStep);BITS(storedEnergy);BITS(kineticEnergy);BITS(initialTotalEnergy);
    BITS(cumulativeExternalWork);BITS(cumulativeSupportWork);BITS(cumulativeElectricalWork);BITS(cumulativeRestLengthWork);
#undef BITS
    const auto sameBits=[](const std::vector<double> &x,const std::vector<double> &y) {
        if(x.size()!=y.size())return false;
        for(size_t i=0;i<x.size();++i)if(std::memcmp(&x[i],&y[i],sizeof(double))!=0)return false;
        return true;
    };
#define VECTOR_BITS(f) QVERIFY(sameBits(a.f,b.f))
    VECTOR_BITS(q);VECTOR_BITS(v);VECTOR_BITS(a);VECTOR_BITS(reactions);VECTOR_BITS(externalForce);
    VECTOR_BITS(memberInputValues);VECTOR_BITS(portVoltages);VECTOR_BITS(portCharges);VECTOR_BITS(portEnthalpies);
#undef VECTOR_BITS
}
}
class TestStructuralTrussDynamicsSetup : public QObject {
    Q_OBJECT
private slots:
    void classificationAndTransactionalBinding();
    void unsafeMetadataBinding_data(); void unsafeMetadataBinding();
    void invalidValues_data(); void invalidValues();
    void exactSourceAndRuntimeIndependence();
    void frozenSources_data(); void frozenSources();
    void persistence_data(); void persistence();
    void signedZeroSourceRoundTrip_data(); void signedZeroSourceRoundTrip();
    void legacyAndDowngrade_data(); void legacyAndDowngrade();
    void malformedReader_data(); void malformedReader();
    void malformedOuterReader_data(); void malformedOuterReader();
    void restartValidation_data(); void restartValidation();
};
void TestStructuralTrussDynamicsSetup::classificationAndTransactionalBinding()
{
    auto m=scene(); auto s=config(); s.bindToModel(m); s.validate(m);
    QCOMPARE(s.passiveMembers[0].physicalElementId,501u); QCOMPARE(s.activeMembers[0].portId,QString("isolated electrode"));
    auto before=s.sourceFingerprint; m.getElement(0).setNodeId(1,0);
    QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError); QCOMPARE(s.sourceFingerprint,before);
    m=scene(); s.cableMembers.clear(); QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError);
    s=config(); s.activeMembers[0].massSource=RStructuralTrussMassSource::FixedMemberMass;
    s.activeMembers[0].fixedMass=0.12345678901234567; s.bindToModel(m); s.validate(m);
}
void TestStructuralTrussDynamicsSetup::unsafeMetadataBinding_data()
{
    QTest::addColumn<QString>("name");
    QTest::newRow("embedded-nul")<<QString::fromLatin1("front\0rear",10);
    QTest::newRow("leading-whitespace")<<QString(" front");
    QTest::newRow("embedded-ascii-quote")<<QString("front\"rear");
}
void TestStructuralTrussDynamicsSetup::unsafeMetadataBinding()
{
    QFETCH(QString,name);auto m=scene();auto s=config();s.bindToModel(m);
    const auto original=s.sourceFingerprint;
    m.getLine(0).setName(name);
    QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError);
    QCOMPARE(s.sourceFingerprint,original);
    QCOMPARE(s.activeMembers[0].portId,QString("isolated electrode"));
}
void TestStructuralTrussDynamicsSetup::invalidValues_data()
{
    QTest::addColumn<int>("mutation");
    const char *names[]={"zero-id","positional-id","overlap","duplicate-port","density-and-fixed","zero-fixed-mass","invalid-kind","nan","zero-length","command-order","unknown-port","duplicate-target","unsupported-element","zero-area","zero-iterations"};
    for(int i=0;i<15;++i) QTest::newRow(names[i])<<i;
}
void TestStructuralTrussDynamicsSetup::invalidValues()
{
    QFETCH(int,mutation); auto m=scene(); auto s=config();
    switch(mutation) {
    case 0:s.passiveMembers[0].physicalElementId=0;break;
    case 1:s.passiveMembers[0].physicalElementId=1;break;
    case 2:s.cableMembers[0].physicalElementId=501;break;
    case 3:s.activeMembers.push_back(s.activeMembers[0]);s.activeMembers.back().physicalElementId=527;s.cableMembers.clear();break;
    case 4:s.activeMembers[0].fixedMass=1;break;
    case 5:s.activeMembers[0].massSource=RStructuralTrussMassSource::FixedMemberMass;break;
    case 6:s.activeMembers[0].kind=static_cast<RStructuralTrussElectricalKind>(99);break;
    case 7:s.activeMembers[0].coupling=std::numeric_limits<double>::quiet_NaN();break;
    case 8:s.passiveMembers[0].referenceLength=0;break;
    case 9:s.heldCommands[1].stepIndex=1;break;
    case 10:s.heldCommands[0].electrical[0].portId="unknown";break;
    case 11:s.heldCommands[0].nodalForces.push_back(s.heldCommands[0].nodalForces[0]);break;
    case 12:m.getElement(0).setType(R_ELEMENT_TRUSS2);break;
    case 13:m.getLine(0).setCrossArea(0);break;
    case 14:s.maxIterations=0;break;
    }
    QVERIFY_EXCEPTION_THROWN(s.bindToModel(m),RError); QVERIFY(s.sourceFingerprint.isEmpty());
}
void TestStructuralTrussDynamicsSetup::exactSourceAndRuntimeIndependence()
{
    auto m=scene(); auto s=config(); s.bindToModel(m); const auto hash=s.sourceFingerprint;
    auto changed=m; changed.getNode(1).setX(std::nextafter(m.getNode(1).getX(),2.0));
    QVERIFY(s.computeSourceFingerprint(changed)!=hash);
    changed=m; changed.getLine(0).setCrossArea(std::nextafter(m.getLine(0).getCrossArea(),1.0));
    QVERIFY(s.computeSourceFingerprint(changed)!=hash);
    changed=m; changed.getProblemSetup().getStructuralDynamicsSetup().setTimeStep(0.02);
    QVERIFY(s.computeSourceFingerprint(changed)!=hash);
    changed=m; changed.getNode(0).setY(-0.0); QVERIFY(s.computeSourceFingerprint(changed)!=hash);
    auto inputs=s; inputs.heldCommands[0].electrical[0].value+=1; QVERIFY(inputs.computeSourceFingerprint(m)!=hash);
    m.getTimeSolver().setCurrentTimeStep(3); m.getTimeSolver().setComputedTime(0.04);
    m.getProblemSetup().getStructuralTrussDynamicsRestartData()=restart();
    const uint pos=m.addVariable(RVariable(R_VARIABLE_DISPLACEMENT)); m.getVariable(pos).resize(1,m.getNNodes());
    m.getVariable(pos).setValue(0,0,123); QCOMPARE(s.computeSourceFingerprint(m),hash);
}
void TestStructuralTrussDynamicsSetup::frozenSources_data()
{
    QTest::addColumn<int>("mutation");const char *names[]={"material-tail","boundary","initial","motion","parallel","initial-time","physical-node-order","reference-length","mass-source"};
    for(int i=0;i<9;++i)QTest::newRow(names[i])<<i;
}
void TestStructuralTrussDynamicsSetup::frozenSources()
{
    QFETCH(int,mutation);auto m=scene();auto s=config();s.bindToModel(m);const auto original=s.sourceFingerprint;
    switch(mutation) {
    case 0: {
        auto &mat=m.getLine(0).getMaterial();
        mat.get(mat.findPosition(RMaterialProperty::Density)).add(500.1234567890123,8123.456789012345);
        break;
    }
    case 1: {
        RBoundaryCondition bc(R_BOUNDARY_CONDITION_DISPLACEMENT);
        bc.getComponent(bc.findComponentPosition(R_VARIABLE_DISPLACEMENT_X)).add(0,0.12345678901234567);
        m.getLine(0).addBoundaryCondition(bc);break;
    }
    case 2: {
        RInitialCondition ic(R_INITIAL_CONDITION_DISPLACEMENT);
        ic.getComponent(ic.findComponentPosition(R_VARIABLE_DISPLACEMENT_X)).add(0,0.23456789012345678);
        m.getLine(0).addInitialCondition(ic);break;
    }
    case 3: m.getProblemSetup().getStructuralInteractionSetup().getPrescribedMotions().push_back(
        {"anchor-motion",{0,RDofType::UX},{{0,0,0,0},{1,0.12345678901234567,0,0}}});break;
    case 4:m.getProblemSetup().getParallelExecutionConfig().setMode(RParallelExecutionMode::SerialDeterministic);break;
    case 5:m.getTimeSolver().setInputStartTime(0.125);break;
    case 6:m.setNodePhysicalId(0,999);break;
    case 7:s.passiveMembers[0].referenceLength=1.2;break;
    case 8:s.activeMembers[0].massSource=RStructuralTrussMassSource::FixedMemberMass;s.activeMembers[0].fixedMass=0.125;break;
    }
    QVERIFY(s.computeSourceFingerprint(m)!=original);
}
void TestStructuralTrussDynamicsSetup::persistence_data()
{ QTest::addColumn<bool>("binary"); QTest::newRow("ascii")<<false; QTest::newRow("binary")<<true; }
void TestStructuralTrussDynamicsSetup::persistence()
{
    QFETCH(bool,binary); QTemporaryDir dir; QVERIFY(dir.isValid()); auto m=scene(); auto s=config(); s.bindToModel(m);
    m.getProblemSetup().setStructuralTrussDynamicsSetup(s); const auto r=restart(); m.getProblemSetup().setStructuralTrussDynamicsRestartData(r);
    const auto path=dir.filePath("complete."+RModel::getDefaultFileExtension(binary)); m.write(path,false);
    RModel restored; restored.read(path); const auto &actual=restored.getProblemSetup().getStructuralTrussDynamicsSetup();
    actual.validate(restored); QCOMPARE(actual.sourceFingerprint,s.sourceFingerprint);
    QCOMPARE(restored.getNode(1).getX(),1.2345678901234567);
    QCOMPARE(restored.getLine(0).getCrossArea(),0.012345678901234567);
    QCOMPARE(restored.getLine(0).getMaterial().get(restored.getLine(0).getMaterial().findPosition(RMaterialProperty::Density)).getValue(0),7890.1234567890123);
    QCOMPARE(actual.passiveMembers[0].referenceLength,1.2345678901234567);
    QCOMPARE(actual.activeMembers[0].physicalElementId,514u);QCOMPARE(actual.activeMembers[0].portId,QString("isolated electrode"));
    QCOMPARE(actual.activeMembers[0].referenceLength,1.2345678901234567);
    QCOMPARE(actual.activeMembers[0].axialStiffness,12345.678901234567);
    QCOMPARE(actual.activeMembers[0].coupling,-0.12345678901234567);
    QCOMPARE(actual.activeMembers[0].capacitance,1.2345678901234567e-8);
    QCOMPARE(int(actual.activeMembers[0].kind),0);QCOMPARE(actual.activeMembers[0].initialValue,-12.345678901234567);
    QCOMPARE(int(actual.activeMembers[0].massSource),0);QCOMPARE(actual.activeMembers[0].fixedMass,0.0);
    QCOMPARE(actual.cableMembers[0].physicalElementId,527u);
    QCOMPARE(actual.cableMembers[0].initialReferenceLength,1.1234567890123457);
    QCOMPARE(actual.cableMembers[0].initialTension,2.345678901234567);
    QCOMPARE(actual.heldCommands[0].stepIndex,1u);QCOMPARE(actual.heldCommands[1].stepIndex,7u);
    QCOMPARE(actual.heldCommands[0].electrical[0].value,13.234567890123457);
    QCOMPARE(actual.heldCommands[0].cables[0].referenceLength,1.1111111111111112);
    QCOMPARE(actual.heldCommands[0].nodalForces[0].force[1],-2.345678901234567);
    equalRestart(restored.getProblemSetup().getStructuralTrussDynamicsRestartData(),r);
    // Exercise the typed readers separately from whole-model persistence.
    const auto typed=dir.filePath("typed");
    { RSaveFile out(typed,binary?RSaveFile::BINARY:RSaveFile::ASCII);out.setVersion(RVersion(1,3,20));QVERIFY(out.open(QIODevice::WriteOnly));
      if(binary){RFileIO::writeBinary(out,s);RFileIO::writeBinary(out,r);}else{RFileIO::writeAscii(out,s);RFileIO::writeAscii(out,r);}QVERIFY(out.commit()); }
    RFile in(typed,binary?RFile::BINARY:RFile::ASCII);in.setVersion(RVersion(1,3,20));QVERIFY(in.open(QIODevice::ReadOnly));
    RStructuralTrussDynamicsSetup ss; RStructuralTrussDynamicsRestartData rr;
    if(binary){RFileIO::readBinary(in,ss);RFileIO::readBinary(in,rr);}else{RFileIO::readAscii(in,ss);RFileIO::readAscii(in,rr);}
    QCOMPARE(ss.activeMembers[0].axialStiffness,12345.678901234567); equalRestart(rr,r);
}
void TestStructuralTrussDynamicsSetup::legacyAndDowngrade_data() { persistence_data(); }
void TestStructuralTrussDynamicsSetup::signedZeroSourceRoundTrip_data() { persistence_data(); }
void TestStructuralTrussDynamicsSetup::signedZeroSourceRoundTrip()
{
    QFETCH(bool,binary);QTemporaryDir dir;auto m=scene();m.getNode(0).setY(-0.0);
    auto s=config();s.bindToModel(m);m.getProblemSetup().setStructuralTrussDynamicsSetup(s);
    const auto path=dir.filePath("signed-zero."+RModel::getDefaultFileExtension(binary));m.write(path,false);
    RModel restored;restored.read(path);QVERIFY(std::signbit(restored.getNode(0).getY()));
    QCOMPARE(s.computeSourceFingerprint(restored),s.sourceFingerprint);
    restored.getProblemSetup().getStructuralTrussDynamicsSetup().validate(restored);
}
void TestStructuralTrussDynamicsSetup::legacyAndDowngrade()
{
    QFETCH(bool,binary); QTemporaryDir dir; const auto path=dir.filePath("legacy"); RProblemSetup defaults;
    {RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII);out.setVersion(RVersion(1,3,19));QVERIFY(out.open(QIODevice::WriteOnly));
     if(binary)RFileIO::writeBinary(out,defaults);else RFileIO::writeAscii(out,defaults);QVERIFY(out.commit());}
    RProblemSetup target; target.getStructuralTrussDynamicsSetup().maxIterations=81;
    target.getStructuralTrussDynamicsRestartData()=restart();
    {RFile in(path,binary?RFile::BINARY:RFile::ASCII);in.setVersion(RVersion(1,3,19));QVERIFY(in.open(QIODevice::ReadOnly));
     if(binary)RFileIO::readBinary(in,target);else RFileIO::readAscii(in,target);}
    QVERIFY(!target.getStructuralTrussDynamicsSetup().hasNonDefaultData()); QVERIFY(!target.getStructuralTrussDynamicsRestartData().hasNonDefaultData());
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly));const auto old=file.readAll();file.close();
    for(int mutation=0;mutation<7;++mutation) {
        RProblemSetup hidden;
        auto &s=hidden.getStructuralTrussDynamicsSetup();
        switch(mutation){
        case 0:s.maxIterations=81;break;
        case 1:s.maxLineSearchIterations=25;break;
        case 2:s.absoluteForceTolerance=2e-9;break;
        case 3:s.linearRelativeTolerance=2e-10;break;
        case 4:s.sourceFingerprint=QString(64,'a');break;
        case 5:s.passiveMembers={{501,1.0}};break;
        case 6:hidden.getStructuralTrussDynamicsRestartData().cumulativeSupportWork=1.0;break;
        }
        // Even disabled/unavailable hidden fields must never be silently dropped.
        {RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII);out.setVersion(RVersion(1,3,19));QVERIFY(out.open(QIODevice::WriteOnly));
         if(binary){QVERIFY_EXCEPTION_THROWN(RFileIO::writeBinary(out,hidden),RError);}else{QVERIFY_EXCEPTION_THROWN(RFileIO::writeAscii(out,hidden),RError);}}
        QVERIFY(file.open(QIODevice::ReadOnly));QCOMPARE(file.readAll(),old);file.close();
    }
}
void TestStructuralTrussDynamicsSetup::malformedReader_data()
{ QTest::addColumn<int>("mutation");QTest::newRow("invalid-bool")<<0;QTest::newRow("truncated")<<1;QTest::newRow("oversized-count")<<2;QTest::newRow("invalid-electrical-enum")<<3; }
void TestStructuralTrussDynamicsSetup::malformedReader()
{
    QFETCH(int,mutation);QTemporaryDir dir;const auto path=dir.filePath("bad.bin");
    {RSaveFile f(path,RSaveFile::BINARY);f.setVersion(RVersion(1,3,20));QVERIFY(f.open(QIODevice::WriteOnly));
     if(mutation<2){const char raw=mutation==0?char(2):char(1);QCOMPARE(f.write(&raw,1),qint64(1));}
     else {
         RFileIO::writeBinary(f,true);RFileIO::writeBinary(f,QString(64,'a'));
         RFileIO::writeBinary(f,mutation==2?1000001u:0u);
         if(mutation==3){RFileIO::writeBinary(f,1u);RFileIO::writeBinary(f,514u);RFileIO::writeBinary(f,QString("port"));
             for(double v:{1.0,100.0,0.1,0.01})RFileIO::writeBinary(f,v);
             RFileIO::writeBinary(f,99);}
     }QVERIFY(f.commit());}
    auto target=config();target.sourceFingerprint=QString(64,'b');
    RFile in(path,RFile::BINARY);in.setVersion(RVersion(1,3,20));QVERIFY(in.open(QIODevice::ReadOnly));
    QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,target),RError);
    QCOMPARE(target.sourceFingerprint,QString(64,'b'));QCOMPARE(target.activeMembers[0].portId,QString("isolated electrode"));
}
void TestStructuralTrussDynamicsSetup::malformedOuterReader_data() {persistence_data();}
void TestStructuralTrussDynamicsSetup::malformedOuterReader()
{
    QFETCH(bool,binary);QTemporaryDir dir;const auto path=dir.filePath("outer");RProblemSetup original;
    {RSaveFile out(path,binary?RSaveFile::BINARY:RSaveFile::ASCII);out.setVersion(RVersion(1,3,20));QVERIFY(out.open(QIODevice::WriteOnly));
     if(binary)RFileIO::writeBinary(out,original);else RFileIO::writeAscii(out,original);QVERIFY(out.commit());}
    {QFile f(path);QVERIFY(f.open(QIODevice::ReadWrite));QVERIFY(f.resize(f.size()/2));}
    RProblemSetup kept;kept.getStructuralTrussDynamicsSetup().maxIterations=81;
    kept.getStructuralTrussDynamicsRestartData()=restart();
    RFile in(path,binary?RFile::BINARY:RFile::ASCII);in.setVersion(RVersion(1,3,20));QVERIFY(in.open(QIODevice::ReadOnly));
    if(binary){QVERIFY_EXCEPTION_THROWN(RFileIO::readBinary(in,kept),RError);}
    else{QVERIFY_EXCEPTION_THROWN(RFileIO::readAscii(in,kept),RError);}
    QCOMPARE(kept.getStructuralTrussDynamicsSetup().maxIterations,81u);
    equalRestart(kept.getStructuralTrussDynamicsRestartData(),restart());
}
void TestStructuralTrussDynamicsSetup::restartValidation_data()
{
    QTest::addColumn<int>("mutation");const char *names[]={"hidden-unavailable","bad-clock","duplicate-id","bad-kind","missing-coordinate","port-count","negative-energy","nonfinite-work","missing-port-voltage","extra-port-charge","nonfinite-port-enthalpy"};
    for(int i=0;i<11;++i)QTest::newRow(names[i])<<i;
}
void TestStructuralTrussDynamicsSetup::restartValidation()
{
    QFETCH(int,mutation);auto r=restart();r.validate();
    switch(mutation){case 0:r.available=false;break;case 1:r.time+=0.01;break;case 2:r.orderedPhysicalNodeIds[1]=101;break;
    case 3:r.memberInputKinds[0]=4;break;case 4:r.q.pop_back();break;case 5:r.orderedPortIds.pop_back();break;
    case 6:r.storedEnergy=-1;break;case 7:r.cumulativeRestLengthWork=std::numeric_limits<double>::infinity();break;
    case 8:r.portVoltages.pop_back();break;case 9:r.portCharges.push_back(1);break;
    case 10:r.portEnthalpies[0]=std::numeric_limits<double>::quiet_NaN();break;}
    QVERIFY_EXCEPTION_THROWN(r.validate(),RError);r.clear();r.validate();QVERIFY(!r.hasNonDefaultData());
}
QTEST_APPLESS_MAIN(TestStructuralTrussDynamicsSetup)
#include "tst_structural_truss_dynamics_setup.moc"
