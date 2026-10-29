#include "rml_structural_piezo_layered_setup.h"
#include "rml_model.h"
#include <rbl_error.h>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <tuple>

namespace {
constexpr size_t maximumCount = 1000000;
constexpr int maximumBlob = 16*1024*1024;
void require(bool ok,const char *message) { if(!ok) throw RError(RError::Type::InvalidInput,R_ERROR_REF,message); }
void count(size_t n) { require(n<=maximumCount,"Layered piezo source count exceeds its bound."); }
void finite(double x) { require(std::isfinite(x),"Non-finite layered piezo source value."); }
template<size_t N> void finite(const std::array<double,N> &a) { for(double x:a) finite(x); }
void text(const QString &s,bool identifier=false)
{
    require(s.toUtf8().size()<=65536,"Layered piezo source text exceeds its bound.");
    if(identifier) require(!s.isEmpty() && s.trimmed()==s && s.toUtf8().size()<=4096,"Invalid layered piezo identity.");
    for(int i=0;i<s.size();++i) {
        const QChar c=s[i];
        require(c.unicode()!=0 && (!identifier || (c.unicode()>=32 && c.unicode()!=127 && c!='"' && c!='\\')),
                "Invalid layered piezo source text.");
        if(c.isHighSurrogate()) { require(i+1<s.size() && s[i+1].isLowSurrogate(),"Invalid layered piezo UTF-16."); ++i; }
        else require(!c.isLowSurrogate(),"Invalid layered piezo UTF-16.");
    }
}
void sha(const QString &s) { require(QRegularExpression(QStringLiteral("^[0-9a-f]{64}$")).match(s).hasMatch(),"Invalid layered piezo SHA-256 provenance."); }
void physical(uint n) { require(n!=0 && n!=RConstants::eod,"Invalid layered piezo physical identity."); }
bool exact(double a,double b) { return std::memcmp(&a,&b,sizeof(double))==0; }
template<size_t N> bool exact(const std::array<double,N> &a,const std::array<double,N> &b)
{ for(size_t i=0;i<N;++i) if(!exact(a[i],b[i])) return false; return true; }
struct Writer {
    QByteArray bytes;
    void integer(quint64 x) { for(uint i=0;i<8;++i) bytes.append(char((x>>(8*i))&255)); }
    void real(double x) { finite(x); static_assert(sizeof(double)==8 && std::numeric_limits<double>::is_iec559,"IEEE binary64 required"); quint64 n; std::memcpy(&n,&x,8); integer(n); }
    void blob(const QByteArray &b) { integer(quint64(b.size())); bytes.append(b); }
    void text(const QString &s) { ::text(s); blob(s.toUtf8()); }
    template<size_t N> void array(const std::array<double,N> &a) { for(double x:a) real(x); }
    void entries(size_t n) { count(n); integer(n); }
};
void writeTable(Writer &w,const RValueTable &t)
{
    w.text(t.getKeyUnits()); w.text(t.getValueUnits()); w.entries(t.size());
    for(uint i=0;i<t.size();++i) { w.real(t.getKey(i)); w.real(t.getValue(i)); }
}
void writeCondition(Writer &w,const RCondition &c)
{
    // Display names are deliberately excluded. Every actual condition component is retained.
    w.integer(c.getApplyOnPoint()); w.integer(c.getApplyOnLine()); w.integer(c.getApplyOnSurface()); w.integer(c.getApplyOnVolume());
    w.integer(c.getProblemTypeMask()); w.entries(c.size());
    for(uint i=0;i<c.size();++i) { const auto &v=c.getComponent(i); w.integer(v.getType()); w.integer(v.getKeyType()); w.integer(v.getEnabled()); writeTable(w,v); }
}
void writeGroup(Writer &w,const RElementGroup &g,const RModel &m)
{
    physical(g.getPhysicalId()); w.integer(g.getPhysicalId()); w.entries(g.size());
    for(uint i=0;i<g.size();++i) { require(g.get(i)<m.getNElements(),"Layered source group has an unknown element."); w.integer(m.getElementPhysicalId(g.get(i))); }
    const auto &mat=g.getMaterial(); w.text(mat.getID().toString(QUuid::WithoutBraces)); w.integer(mat.getState()); w.entries(mat.size());
    for(uint i=0;i<mat.size();++i) { const auto &p=mat.get(i); w.integer(p.getType()); w.integer(p.getKeyType()); writeTable(w,p); }
    w.entries(g.getNBoundaryConditions());
    for(uint i=0;i<g.getNBoundaryConditions();++i) {
        const auto &c=g.getBoundaryCondition(i); w.integer(c.getType()); w.integer(c.getOptional()); w.integer(c.getExplicit());
        w.integer(c.getHasLocalDirection()); w.integer(c.getExplicitLocalDirection());
        const auto &d=c.getLocalDirection(); require(d.size()==3,"Invalid layered boundary direction."); for(uint j=0;j<3;++j) w.real(d[j]); writeCondition(w,c);
    }
    w.entries(g.getNInitialConditions()); for(uint i=0;i<g.getNInitialConditions();++i) { const auto &c=g.getInitialCondition(i); w.integer(c.getType()); writeCondition(w,c); }
    w.entries(g.getNEnvironmentConditions()); for(uint i=0;i<g.getNEnvironmentConditions();++i) { const auto &c=g.getEnvironmentCondition(i); w.integer(c.getType()); writeCondition(w,c); }
}
void writeTask(Writer &w,const RProblemTaskItem &t,uint depth=0)
{
    require(depth<=256,"Layered source problem tree exceeds its depth bound."); w.integer(t.getProblemType()); w.integer(t.getNIterations()); w.entries(t.getNChildren());
    for(uint i=0;i<t.getNChildren();++i) writeTask(w,t.getChild(i),depth+1);
}
bool defaultLegacyRestart(const RStructuralDynamicsRestartData &s)
{
    const RStructuralDynamicsRestartData defaults;
    const auto fields=[](const RStructuralDynamicsRestartData &v) {
        return std::tie(v.available,v.completedSteps,v.time,v.fixedTimeStep,v.meshNodeCount,v.meshElementCount,
            v.physicalDofCount,v.meshFingerprint,v.dofFingerprint,v.systemFingerprint,v.q,v.v,v.a,v.externalForce,
            v.cumulativeExternalWork,v.cumulativeDampingDissipation,v.initialMechanicalEnergy,v.supportResponseAvailable,
            v.supportFingerprint,v.supportReactions,v.supportPowers,v.cumulativeSupportWork,v.controlAvailable,
            v.controlProtocolVersion,v.controlFingerprint,v.controlChannelIds,v.controlLastCommands,v.controlPowers,
            v.cumulativeControlWork,v.controlControllerState,v.controlDeterministicSeed);
    };
    return fields(s)==fields(defaults) && exact(s.time,defaults.time) && exact(s.fixedTimeStep,defaults.fixedTimeStep) &&
        exact(s.cumulativeExternalWork,defaults.cumulativeExternalWork) && exact(s.cumulativeDampingDissipation,defaults.cumulativeDampingDissipation) &&
        exact(s.initialMechanicalEnergy,defaults.initialMechanicalEnergy);
}
bool defaultTrussRestart(const RStructuralTrussDynamicsRestartData &s)
{
    return !s.hasNonDefaultData() && exact(s.initialTime,0.0) && exact(s.time,0.0) && exact(s.fixedTimeStep,0.0) &&
        exact(s.storedEnergy,0.0) && exact(s.kineticEnergy,0.0) && exact(s.initialTotalEnergy,0.0) &&
        exact(s.cumulativeExternalWork,0.0) && exact(s.cumulativeSupportWork,0.0) &&
        exact(s.cumulativeElectricalWork,0.0) && exact(s.cumulativeRestLengthWork,0.0);
}
void topology(const RStructuralPiezoLayeredSetup &s,const RModel &m)
{
    if(!s.hasNonDefaultData()) return;
    const auto &p=m.getProblemSetup(); const auto &d=p.getStructuralDynamicsSetup();
    require((m.getProblemTaskTree().getProblemTypeMask() & R_PROBLEM_STRESS)!=0,"Layered native source requires STRESS.");
    d.validate(); require(d.getEnabled() && p.getMassMatrixType()==RMassMatrixType::Consistent,"Layered native dynamics requires enabled dynamics and consistent mass.");
    require(d.getMethod()==RStructuralTimeIntegrationMethod::Newmark && exact(d.getNewmarkBeta(),.25) &&
            exact(d.getNewmarkGamma(),.5) && exact(d.getRhoInfinity(),1.0),
            "Layered native driver requires average-acceleration Newmark with default unused rhoInfinity.");
    const auto &interaction=p.getStructuralInteractionSetup();
    require(interaction.getLinearConstraints().empty() && interaction.getCouplings().empty() && interaction.getPrescribedMotions().empty() &&
            interaction.getSensors().empty() && interaction.getActuators().empty(),
            "Layered native driver cannot compile structural interactions yet.");
    require(!p.getStructuralTrussDynamicsSetup().hasNonDefaultData(),"Layered source conflicts with native truss source data.");
    require(!p.getStructuralPiezoReadoutSetup().hasNonDefaultData(),"Layered source must use its own electrical port/workpoint catalog.");
    require(!p.getStructuralNativeControlSpec().hasNonDefaultData() && !p.getStructuralNativeControlState().hasNonDefaultData(),
            "Layered native driver cannot consume legacy native control source/state.");
    const auto &control=p.getStructuralControlSetup(); const RStructuralControlSetup defaultControl;
    require(!control.enabled && control.controlProtocolVersion==defaultControl.controlProtocolVersion && control.controllerId.isEmpty() &&
            control.deterministicSeed==defaultControl.deterministicSeed && control.channelConditioning.empty(),
            "Layered native driver cannot consume legacy control source data.");
    const auto &thermal=p.getStructuralThermalControlSetup(); const RStructuralThermalControlSetup defaultThermal;
    require(!thermal.enabled && exact(thermal.thermalSubstepSeconds,defaultThermal.thermalSubstepSeconds) &&
            thermal.thermalSubstepsPerMechanicalMacrostep==defaultThermal.thermalSubstepsPerMechanicalMacrostep &&
            thermal.mechanicalMacrostepsPerControllerSample==defaultThermal.mechanicalMacrostepsPerControllerSample &&
            thermal.channels.empty() && thermal.sparseObservationNodeIds.empty(),
            "Layered native driver cannot consume thermal control source data.");
    const auto &piezo=p.getStructuralPiezoSetup(); const RStructuralPiezoSetup defaultPiezo;
    require(!piezo.enabled && piezo.meshFingerprint.isEmpty() && piezo.ports.empty() && piezo.passiveElementIds.empty() &&
            piezo.nodalBoundaryConditions.empty() && piezo.electricalMode==defaultPiezo.electricalMode,
            "Layered source conflicts with legacy piezo source data.");
    const auto &cable=p.getStructuralCableSetup(); const RStructuralCableSetup defaultCable;
    require(!cable.enabled && cable.meshFingerprint.isEmpty() && cable.members.empty() &&
            cable.maxIterations==defaultCable.maxIterations && cable.maxLineSearchIterations==defaultCable.maxLineSearchIterations &&
            exact(cable.absoluteForceTolerance,defaultCable.absoluteForceTolerance) && exact(cable.relativeForceTolerance,defaultCable.relativeForceTolerance),
            "Layered source conflicts with legacy cable source data.");
    require(defaultLegacyRestart(p.getStructuralDynamicsRestartData()) && defaultTrussRestart(p.getStructuralTrussDynamicsRestartData()),
            "Layered native driver cannot consume legacy dynamics/truss restart state.");
    require(exact(d.getRayleighAlpha(),0.0) && exact(d.getRayleighBeta(),0.0) && d.getModalDampingRatios().empty(),"Layered user damping is the exclusive damping authority.");
    finite(m.getTimeSolver().getInputStartTime()); finite(m.getTimeSolver().getInputStartTime()-d.getTimeStep());
    finite(m.getTimeSolver().getInputStartTime()+double(d.getNSteps())*d.getTimeStep());
    count(m.getNNodes()); count(m.getNElements()); std::set<uint> nodes,hostNodes,hosts,elements;
    count(m.getNPoints());
    for(uint i=0;i<m.getNPoints();++i) {
        finite(m.getPoint(i).getVolume());
        require(m.getPoint(i).getVolume()==0,"Layered POINT helpers cannot carry artificial volume or point mass.");
    }
    for(uint i=0;i<m.getNNodes();++i) { physical(m.getNodePhysicalId(i)); require(nodes.insert(m.getNodePhysicalId(i)).second,"Duplicate model physical node."); const auto &n=m.getNode(i); finite(n.getX()); finite(n.getY()); finite(n.getZ()); }
    const auto hostType=s.family==RStructuralPiezoLayeredModelFamily::PlanarBeam?R_ELEMENT_BEAM1:R_ELEMENT_QUAD1;
    const bool solid=s.family==RStructuralPiezoLayeredModelFamily::Solid3D;
    const auto isHost=[&](RElementType type){return solid?(type==R_ELEMENT_TETRA1||type==R_ELEMENT_HEXA1):type==hostType;};
    for(uint i=0;i<m.getNElements();++i) {
        physical(m.getElementPhysicalId(i)); require(elements.insert(m.getElementPhysicalId(i)).second,"Duplicate model physical element.");
        const auto &e=m.getElement(i); require(isHost(e.getType()) || e.getType()==R_ELEMENT_POINT,"Unsupported actual element in layered native route.");
        require(e.size()==RElement::getNNodes(e.getType()),"Invalid physical host connectivity shape.");
        std::set<uint> ends;
        for(uint j=0;j<e.size();++j) { const uint pos=e.getNodeId(j); require(pos<m.getNNodes() && ends.insert(pos).second,"Invalid or repeated layered host node."); if(isHost(e.getType())) hostNodes.insert(m.getNodePhysicalId(pos)); }
        if(isHost(e.getType())) hosts.insert(m.getElementPhysicalId(i));
    }
    require(!hosts.empty() && hosts.size()==s.members.size(),"Layered declarations must cover every actual physical host.");
    for(const auto &v:s.members) require(hosts.count(v.physicalElementId)==1,"Layered member references an unknown physical host.");
    require(nodes==hostNodes,"Layered model contains nodes unused by its physical hosts.");
    for(uint i=0;i<m.getNElements();++i) if(m.getElement(i).getType()==R_ELEMENT_POINT)
        require(hostNodes.count(m.getNodePhysicalId(m.getElement(i).getNodeId(0)))==1,"POINT helper must reference a physical host node.");
    std::map<uint,uint> owner;
    const auto own=[&](const RElementGroup &g) { count(g.size()); for(uint j=0;j<g.size();++j) { require(g.get(j)<m.getNElements(),"Invalid layered host group index."); if(isHost(m.getElement(g.get(j)).getType())) require(owner.emplace(g.get(j),g.getPhysicalId()).second,"Layered host has multiple group owners."); } };
    if(solid) { count(m.getNVolumes()); for(uint i=0;i<m.getNVolumes();++i) own(m.getVolume(i)); }
    else if(hostType==R_ELEMENT_BEAM1) { count(m.getNLines()); for(uint i=0;i<m.getNLines();++i) own(m.getLine(i)); }
    else { count(m.getNSurfaces()); for(uint i=0;i<m.getNSurfaces();++i) own(m.getSurface(i)); }
    require(owner.size()==hosts.size(),"Native piezo host lacks an actual line/surface/volume group owner.");
    if(s.damping.kind==RStructuralPiezoDampingKind::Imported) require(s.damping.importedDimension==quint64(nodes.size())*((solid||hostType==R_ELEMENT_BEAM1)?3u:5u),"Imported damping must use the complete supported generalized DOF order.");
    for(const auto &v:s.supports) require(nodes.count(v.physicalNodeId)==1,"Unknown layered support physical node.");
    for(const auto *fields:{&s.backgroundForces,&s.initialDisplacements,&s.initialVelocities}) for(const auto &v:*fields) require(nodes.count(v.physicalNodeId)==1,"Unknown layered field physical node.");
    if(s.workpoint.available) { require(s.workpoint.nodes.size()==nodes.size() && s.workpoint.ports.size()==s.ports.size(),"Layered workpoint must cover all physical nodes and ports."); for(const auto &n:s.workpoint.nodes) require(nodes.count(n.physicalNodeId)==1,"Unknown layered workpoint node."); }
}
}

bool RStructuralPiezoLayeredSetup::hasNonDefaultData() const
{
    const RStructuralPiezoLayeredFrame f;
    return enabled || family!=RStructuralPiezoLayeredModelFamily::PlanarBeam || !exact(frame.origin,f.origin) || !exact(frame.ex,f.ex) || !exact(frame.ey,f.ey) || !exact(frame.ez,f.ez) ||
        !sourceFingerprint.isEmpty() || !materials.empty() || !members.empty() || !ports.empty() || !supports.empty() || !backgroundForces.empty() || !initialDisplacements.empty() || !initialVelocities.empty() ||
        workpoint.available || !workpoint.nodes.empty() || !workpoint.ports.empty() || damping.kind!=RStructuralPiezoDampingKind::None || !exact(damping.rayleighAlpha,0.0) || !exact(damping.rayleighBeta,0.0) || !exact(damping.uniformRatio,0.0) ||
        !damping.ratios.empty() || !damping.clusterEnds.empty() || damping.importedDimension!=0 || !damping.importedMatrix.empty() || !damping.sourceId.isEmpty() || !damping.sourceSha256.isEmpty() ||
        !excitationConfiguration.isEmpty() || !controllerConfiguration.isEmpty() || !designConfiguration.isEmpty();
}
void RStructuralPiezoLayeredSetup::validate() const
{
    require(enabled || !hasNonDefaultData(),"Disabled layered setup contains hidden source data.");
    if(!hasNonDefaultData()) return;
    require(family==RStructuralPiezoLayeredModelFamily::PlanarBeam || family==RStructuralPiezoLayeredModelFamily::FlatPlate || family==RStructuralPiezoLayeredModelFamily::Solid3D,"Invalid native piezo family.");
    finite(frame.origin); finite(frame.ex); finite(frame.ey); finite(frame.ez);
    const auto dot=[](const std::array<double,3> &a,const std::array<double,3> &b) { double v=0; for(uint i=0;i<3;++i) v+=a[i]*b[i]; return v; };
    for(const auto *a:{&frame.ex,&frame.ey,&frame.ez}) require(std::abs(dot(*a,*a)-1)<1e-10,"Layered frame axis is not normalized.");
    require(std::abs(dot(frame.ex,frame.ey))<1e-10 && std::abs(dot(frame.ex,frame.ez))<1e-10 && std::abs(dot(frame.ey,frame.ez))<1e-10,"Layered frame axes are not orthogonal.");
    std::array<double,3> cross{{frame.ex[1]*frame.ey[2]-frame.ex[2]*frame.ey[1],frame.ex[2]*frame.ey[0]-frame.ex[0]*frame.ey[2],frame.ex[0]*frame.ey[1]-frame.ex[1]*frame.ey[0]}};
    require(dot(cross,frame.ez)>1-1e-10,"Layered frame is not right handed.");
    if(!sourceFingerprint.isEmpty()) sha(sourceFingerprint);
    count(materials.size()); count(members.size()); count(ports.size()); count(supports.size());
    require(!materials.empty() && !members.empty(),"Enabled layered source requires materials and physical hosts.");
    std::map<QString,const RStructuralPiezoLayeredMaterial*> mats;
    for(const auto &v:materials) {
        text(v.materialId,true); require(mats.emplace(v.materialId,&v).second,"Duplicate layered material identity.");
        require(v.form==RStructuralPiezoMaterialForm::StressCharge || v.form==RStructuralPiezoMaterialForm::StrainCharge,"Unknown layered material form.");
        finite(v.elasticityOrCompliance); finite(v.coupling); finite(v.permittivity); finite(v.density); require(v.density>0,"Layered density must be positive.");
        require(v.sourceKind==RStructuralPiezoMaterialSourceKind::Example || v.sourceKind==RStructuralPiezoMaterialSourceKind::Vendor || v.sourceKind==RStructuralPiezoMaterialSourceKind::Calibration,"Unknown layered material provenance kind.");
        text(v.sourceId,true); sha(v.sourceSha256); text(v.notes);
        if(!v.piezoelectric) { for(double x:v.coupling) require(x==0,"Mechanical material contains hidden piezo coupling."); for(double x:v.permittivity) require(x==0,"Mechanical material contains hidden permittivity."); }
    }
    std::set<QString> portIds,usedPorts; RStructuralPiezoReadoutSetup catalog;
    for(const auto &v:ports) {
        require(portIds.insert(v.readout.portId).second,"Duplicate layered port."); catalog.ports.push_back(v.readout); finite(v.initialValue);
        if(v.readout.role==RStructuralPiezoPortRole::Sensor && v.readout.readout!=RStructuralPiezoReadoutMode::OpenCircuitVoltage) require(v.initialValue==0,"Short/amplifier sensor must hold zero voltage.");
    }
    catalog.validate(); std::set<uint> hostIds;
    for(const auto &v:members) {
        physical(v.physicalElementId); require(hostIds.insert(v.physicalElementId).second,"Duplicate layered physical host."); count(v.layers.size()); require(!v.layers.empty(),"Layered host has no layers.");
        if(family==RStructuralPiezoLayeredModelFamily::Solid3D) require(v.layers.size()==1,"Solid host requires one whole-volume material declaration.");

        for(const auto &l:v.layers) {
            text(l.materialId,true); const auto found=mats.find(l.materialId); require(found!=mats.end(),"Unknown layered material reference.");
            finite(l.lowerZ); finite(l.upperZ); finite(l.width); finite(l.coverage);
            require(l.upperZ>l.lowerZ,"Layer has non-positive thickness.");
            if(family==RStructuralPiezoLayeredModelFamily::Solid3D) {
                require(l.width==1 && l.coverage==std::array<double,4>{{-1,1,-1,1}},"Solid declaration requires neutral width and full volume coverage.");
                if(!found->second->piezoelectric) require(l.lowerZ==0 && l.upperZ==1,"Passive solid requires neutral electrode planes 0..1.");
            }
            require(l.width>0 && (family!=RStructuralPiezoLayeredModelFamily::FlatPlate || l.width==1),"Invalid layer width."); require(l.coverage[0]>=-1 && l.coverage[1]<=1 && l.coverage[0]<l.coverage[1] && l.coverage[2]>=-1 && l.coverage[3]<=1 && l.coverage[2]<l.coverage[3],"Invalid parent-coordinate layer coverage rectangle."); require(l.polarity==1 || l.polarity==-1,"Invalid layered polarization.");
            if(found->second->piezoelectric) { text(l.portId,true); require(portIds.count(l.portId)==1,"Unknown layered electrode port."); usedPorts.insert(l.portId); }
            else require(l.portId.isEmpty() && l.polarity==1,"Mechanical layer contains hidden electrode data.");
        }
    }
    for(const auto &m:members) for(size_t i=0;i<m.layers.size();++i) for(size_t j=i+1;j<m.layers.size();++j) {
        const auto &a=m.layers[i]; const auto &b=m.layers[j];
        const bool overlapZ=std::max(a.lowerZ,b.lowerZ)<std::min(a.upperZ,b.upperZ);
        const bool overlapXi=std::max(a.coverage[0],b.coverage[0])<std::min(a.coverage[1],b.coverage[1]);
        const bool overlapEta=std::max(a.coverage[2],b.coverage[2])<std::min(a.coverage[3],b.coverage[3]);
        require(!(overlapZ && overlapXi && overlapEta),"Layer source volumes overlap.");
    }
    require(usedPorts==portIds,"Every layered port must cover a piezoelectric layer.");
    std::set<std::pair<uint,uint>> supportIds;
    for(const auto &v:supports) { physical(v.physicalNodeId); require(uint(v.dof)<=5,"Unknown layered physical DOF."); finite(v.value); require(supportIds.emplace(v.physicalNodeId,uint(v.dof)).second,"Duplicate layered physical support."); }
    for(const auto *fields:{&backgroundForces,&initialDisplacements,&initialVelocities}) { count(fields->size()); std::set<uint> ids; for(const auto &v:*fields) { physical(v.physicalNodeId); require(ids.insert(v.physicalNodeId).second,"Duplicate layered physical field node."); finite(v.values); } }
    count(workpoint.nodes.size()); count(workpoint.ports.size()); require(workpoint.available || (workpoint.nodes.empty() && workpoint.ports.empty()),"Unavailable layered workpoint contains hidden data.");
    std::set<uint> wpNodes; std::set<QString> wpPorts;
    for(const auto &v:workpoint.nodes) { physical(v.physicalNodeId); require(wpNodes.insert(v.physicalNodeId).second,"Duplicate layered workpoint node."); finite(v.q0); finite(v.f0); }
    for(const auto &v:workpoint.ports) { text(v.portId,true); require(wpPorts.insert(v.portId).second && portIds.count(v.portId)==1,"Unknown or duplicate layered workpoint port."); finite(v.V0); finite(v.Q0); }
    const auto &d=damping; require(uint(d.kind)<=uint(RStructuralPiezoDampingKind::Imported),"Unknown layered damping authority.");
    finite(d.rayleighAlpha); finite(d.rayleighBeta); finite(d.uniformRatio); require(d.rayleighAlpha>=0 && d.rayleighBeta>=0 && d.uniformRatio>=0,"Negative layered damping value.");
    count(d.ratios.size()); count(d.clusterEnds.size()); count(d.importedMatrix.size()); for(double x:d.ratios) { finite(x); require(x>=0,"Negative layered modal damping."); } for(double x:d.importedMatrix) finite(x);
    require(d.kind==RStructuralPiezoDampingKind::Rayleigh || (d.rayleighAlpha==0 && d.rayleighBeta==0),"Irrelevant Rayleigh damping fields.");
    require(d.kind==RStructuralPiezoDampingKind::UniformModal || d.uniformRatio==0,"Irrelevant uniform damping field.");
    const bool modal=d.kind==RStructuralPiezoDampingKind::PerMode || d.kind==RStructuralPiezoDampingKind::PerCluster;
    require(modal?!d.ratios.empty():d.ratios.empty(),"Missing or irrelevant per-mode damping fields.");
    if(d.kind==RStructuralPiezoDampingKind::PerCluster) { require(d.clusterEnds.size()==d.ratios.size(),"Damping cluster shape mismatch."); uint last=0; for(uint end:d.clusterEnds) { require(end>last && end<=maximumCount,"Invalid damping cluster layout."); last=end; } }
    else require(d.clusterEnds.empty(),"Irrelevant damping clusters.");
    if(d.kind==RStructuralPiezoDampingKind::Imported) { require(d.importedDimension>0 && quint64(d.importedDimension)*d.importedDimension==d.importedMatrix.size(),"Imported damping matrix shape mismatch."); text(d.sourceId,true); sha(d.sourceSha256); }
    else require(d.importedDimension==0 && d.importedMatrix.empty() && d.sourceId.isEmpty() && d.sourceSha256.isEmpty(),"Irrelevant imported damping source fields.");
    for(const auto *b:{&excitationConfiguration,&controllerConfiguration,&designConfiguration}) require(b->size()<=maximumBlob,"Layered opaque declaration exceeds its bound.");
}

void RStructuralPiezoLayeredSetup::validate(const RModel &model) const
{ validate(); topology(*this,model); if(!sourceFingerprint.isEmpty()) require(sourceFingerprint==computeSourceFingerprint(model),"Layered setup is bound to a changed source model."); }

QByteArray RStructuralPiezoLayeredSetup::identityBytes() const
{
    validate(); if(!hasNonDefaultData()) return {};
    Writer w; w.text(QStringLiteral("range-fea/piezo-layered/source/v1/binary64")); w.integer(enabled); w.integer(uint(family));
    w.array(frame.origin); w.array(frame.ex); w.array(frame.ey); w.array(frame.ez);
    w.entries(materials.size()); for(const auto &m:materials) { w.text(m.materialId); w.integer(uint(m.form)); w.array(m.elasticityOrCompliance); w.array(m.coupling); w.array(m.permittivity); w.real(m.density); w.integer(m.piezoelectric); w.integer(uint(m.sourceKind)); w.text(m.sourceId); w.text(m.sourceSha256); w.text(m.notes); }
    w.entries(members.size()); for(const auto &m:members) { w.integer(m.physicalElementId); w.entries(m.layers.size()); for(const auto &l:m.layers) { w.text(l.materialId); w.real(l.lowerZ); w.real(l.upperZ); w.real(l.width); w.text(l.portId); w.integer(quint64(qint64(l.polarity))); w.array(l.coverage); } }
    RStructuralPiezoReadoutSetup catalog; for(const auto &p:ports) catalog.ports.push_back(p.readout); w.blob(catalog.identityBytes()); w.entries(ports.size()); for(const auto &p:ports) w.real(p.initialValue);
    w.entries(supports.size()); for(const auto &s:supports) { w.integer(s.physicalNodeId); w.integer(uint(s.dof)); w.real(s.value); }
    for(const auto *fields:{&backgroundForces,&initialDisplacements,&initialVelocities}) { w.entries(fields->size()); for(const auto &f:*fields) { w.integer(f.physicalNodeId); w.array(f.values); } }
    w.integer(workpoint.available); w.entries(workpoint.nodes.size()); for(const auto &n:workpoint.nodes) { w.integer(n.physicalNodeId); w.array(n.q0); w.array(n.f0); }
    w.entries(workpoint.ports.size()); for(const auto &p:workpoint.ports) { w.text(p.portId); w.real(p.V0); w.real(p.Q0); }
    const auto &d=damping; w.integer(uint(d.kind)); w.real(d.rayleighAlpha); w.real(d.rayleighBeta); w.real(d.uniformRatio); w.entries(d.ratios.size()); for(double x:d.ratios) w.real(x);
    w.entries(d.clusterEnds.size()); for(uint x:d.clusterEnds) w.integer(x); w.integer(d.importedDimension); w.entries(d.importedMatrix.size()); for(double x:d.importedMatrix) w.real(x); w.text(d.sourceId); w.text(d.sourceSha256);
    w.blob(excitationConfiguration); w.blob(controllerConfiguration); w.blob(designConfiguration); return w.bytes;
}
QString RStructuralPiezoLayeredSetup::computeSourceFingerprint(const RModel &model) const
{
    validate(); topology(*this,model); if(!hasNonDefaultData()) return {};
    Writer w; w.blob(identityBytes()); w.entries(model.getNNodes());
    for(uint i=0;i<model.getNNodes();++i) { w.integer(model.getNodePhysicalId(i)); const auto &n=model.getNode(i); w.real(n.getX()); w.real(n.getY()); w.real(n.getZ()); }
    w.entries(model.getNElements()); for(uint i=0;i<model.getNElements();++i) { w.integer(model.getElementPhysicalId(i)); const auto &e=model.getElement(i); w.integer(e.getType()); w.entries(e.size()); for(uint j=0;j<e.size();++j) w.integer(model.getNodePhysicalId(e.getNodeId(j))); }
    std::set<uint> groups; const auto group=[&](const RElementGroup &g) { require(groups.insert(g.getPhysicalId()).second,"Duplicate layered physical group."); writeGroup(w,g,model); };
    w.entries(model.getNPoints()); for(uint i=0;i<model.getNPoints();++i) { group(model.getPoint(i)); w.real(model.getPoint(i).getVolume()); }
    w.entries(model.getNLines()); for(uint i=0;i<model.getNLines();++i) { const auto &l=model.getLine(i); group(l); w.real(l.getCrossArea()); w.real(l.getSecondMomentY()); w.real(l.getSecondMomentZ()); w.real(l.getTorsionalConstant()); w.real(l.getSectionModulusY()); w.real(l.getSectionModulusZ()); w.real(l.getTorsionalSectionModulus()); const auto &v=l.getLocalYDirection(); require(v.size()==3,"Invalid source line direction."); for(uint j=0;j<3;++j) w.real(v[j]); }
    w.entries(model.getNSurfaces()); for(uint i=0;i<model.getNSurfaces();++i) { const auto &s=model.getSurface(i); group(s); w.real(s.getThickness()); w.integer(uint(s.getStructuralFormulation())); }
    w.entries(model.getNVolumes()); for(uint i=0;i<model.getNVolumes();++i) group(model.getVolume(i));
    const auto &p=model.getProblemSetup(); const auto &d=p.getStructuralDynamicsSetup(); w.integer(uint(p.getMassMatrixType())); w.integer(d.getEnabled()); w.integer(uint(d.getMethod())); w.real(d.getTimeStep()); w.integer(d.getNSteps()); w.integer(d.getOutputFrequency()); w.real(d.getRhoInfinity()); w.real(d.getNewmarkGamma()); w.real(d.getNewmarkBeta());
    w.real(model.getTimeSolver().getInputStartTime()); writeTask(w,model.getProblemTaskTree());
    return QString::fromLatin1(QCryptographicHash::hash(w.bytes,QCryptographicHash::Sha256).toHex());
}
void RStructuralPiezoLayeredSetup::bindToModel(const RModel &model)
{
    auto candidate=*this; candidate.sourceFingerprint.clear(); candidate.validate(); topology(candidate,model);
    candidate.sourceFingerprint=candidate.computeSourceFingerprint(model); candidate.validate(); *this=std::move(candidate);
}
void RStructuralPiezoLayeredRestart::validate() const
{
    require(stateBytes.size()<=maximumBlob,"Layered restart exceeds its bounded state size.");
    if(available) { sha(sourceFingerprint); require(!stateBytes.isEmpty(),"Available layered restart requires state bytes."); }
    else require(sourceFingerprint.isEmpty() && stateBytes.isEmpty(),"Unavailable layered restart contains hidden state.");
}
