#include <QJsonArray>

#include "rml_element_descriptor.h"

namespace
{

QList<RDofType> translations()
{
    return {RDofType::UX,RDofType::UY,RDofType::UZ};
}

QList<RDofType> frameDofs()
{
    return {RDofType::UX,RDofType::UY,RDofType::UZ,
            RDofType::RX,RDofType::RY,RDofType::RZ};
}

QStringList elasticMaterial()
{
    return {"elastic-modulus","poisson-ratio","density","thermal-expansion-coefficient"};
}

const QList<RElementDescriptor> &descriptors()
{
    const QStringList structuralLoads{"force","weight","gravity","temperature"};
    const QStringList membraneLoads{"force","surface-traction","pressure","weight","gravity","temperature"};
    const QStringList solidResults{"displacement","force","stress","strain","von-mises-stress"};
    static const QList<RElementDescriptor> values{
        {R_ELEMENT_POINT,RElementTopology::Point,0,1,0,translations(),{RElementFormulation::Auxiliary},0,true,true,
         {"density"},{"force","weight","gravity"},{"displacement","force"}},
        {R_ELEMENT_TRUSS1,RElementTopology::Line,1,2,1,translations(),{RElementFormulation::Truss3D},2,true,true,
         elasticMaterial(),structuralLoads,{"displacement","force","normal-stress","von-mises-stress"}},
        {R_ELEMENT_TRUSS2,RElementTopology::Line,2,3,1,translations(),{RElementFormulation::Truss3D},2,true,true,
         elasticMaterial(),structuralLoads,{"displacement","force","normal-stress","von-mises-stress"}},
        {R_ELEMENT_BEAM1,RElementTopology::Line,1,2,1,frameDofs(),{RElementFormulation::Frame3D},2,true,true,
         elasticMaterial(),{"force","moment","distributed-line-load","gravity","temperature"},{"displacement","rotation","force","moment","stress"}},
        {R_ELEMENT_BEAM2,RElementTopology::Line,2,3,1,frameDofs(),{RElementFormulation::Frame3D},3,true,true,
         elasticMaterial(),{"force","moment","distributed-line-load","gravity","temperature"},{"displacement","rotation","force","moment","stress"}},
        {R_ELEMENT_TRI1,RElementTopology::Triangle,1,3,2,translations(),
         {RElementFormulation::PlaneStress,RElementFormulation::PlaneStrain,RElementFormulation::Axisymmetric},3,true,true,
         elasticMaterial(),membraneLoads,solidResults},
        {R_ELEMENT_TRI2,RElementTopology::Triangle,2,6,2,translations(),
         {RElementFormulation::PlaneStress,RElementFormulation::PlaneStrain,RElementFormulation::Axisymmetric},3,true,true,
         elasticMaterial(),membraneLoads,solidResults},
        {R_ELEMENT_QUAD1,RElementTopology::Quadrilateral,1,4,2,translations(),
         {RElementFormulation::PlaneStress,RElementFormulation::PlaneStrain,RElementFormulation::Axisymmetric},4,true,true,
         elasticMaterial(),membraneLoads,solidResults},
        {R_ELEMENT_QUAD2,RElementTopology::Quadrilateral,2,9,2,translations(),
         {RElementFormulation::PlaneStress,RElementFormulation::PlaneStrain,RElementFormulation::Axisymmetric},9,true,true,
         elasticMaterial(),membraneLoads,solidResults},
        {R_ELEMENT_TETRA1,RElementTopology::Tetrahedron,1,4,3,translations(),{RElementFormulation::Solid3D},4,true,true,
         elasticMaterial(),structuralLoads,solidResults},
        {R_ELEMENT_WEDGE1,RElementTopology::Wedge,1,6,3,translations(),{RElementFormulation::Solid3D},6,true,true,
         elasticMaterial(),structuralLoads,solidResults},
        {R_ELEMENT_HEXA1,RElementTopology::Hexahedron,1,8,3,translations(),{RElementFormulation::Solid3D},8,true,true,
         elasticMaterial(),structuralLoads,solidResults}
    };
    return values;
}

}

RElementDescriptor::RElementDescriptor(RElementType type,
                                       RElementTopology topology,
                                       uint order,
                                       uint nNodes,
                                       uint naturalDimension,
                                       const QList<RDofType> &dofsPerNode,
                                       const QList<RElementFormulation> &formulations,
                                       uint defaultIntegrationPoints,
                                       bool consistentMass,
                                       bool lumpedMass,
                                       const QStringList &requiredMaterialProperties,
                                       const QStringList &supportedLoads,
                                       const QStringList &producedResults)
    : type(type)
    , topology(topology)
    , order(order)
    , nNodes(nNodes)
    , naturalDimension(naturalDimension)
    , dofsPerNode(dofsPerNode)
    , formulations(formulations)
    , defaultIntegrationPoints(defaultIntegrationPoints)
    , consistentMass(consistentMass)
    , lumpedMass(lumpedMass)
    , requiredMaterialProperties(requiredMaterialProperties)
    , supportedLoads(supportedLoads)
    , producedResults(producedResults)
{
}

RElementType RElementDescriptor::getType() const { return this->type; }
RElementTopology RElementDescriptor::getTopology() const { return this->topology; }
uint RElementDescriptor::getOrder() const { return this->order; }
uint RElementDescriptor::getNNodes() const { return this->nNodes; }
uint RElementDescriptor::getNaturalDimension() const { return this->naturalDimension; }
const QList<RDofType> &RElementDescriptor::getDofsPerNode() const { return this->dofsPerNode; }
const QList<RElementFormulation> &RElementDescriptor::getFormulations() const { return this->formulations; }
uint RElementDescriptor::getDefaultIntegrationPoints() const { return this->defaultIntegrationPoints; }
bool RElementDescriptor::hasConsistentMass() const { return this->consistentMass; }
bool RElementDescriptor::hasLumpedMass() const { return this->lumpedMass; }
const QStringList &RElementDescriptor::getRequiredMaterialProperties() const { return this->requiredMaterialProperties; }
const QStringList &RElementDescriptor::getSupportedLoads() const { return this->supportedLoads; }
const QStringList &RElementDescriptor::getProducedResults() const { return this->producedResults; }
bool RElementDescriptor::hasDof(RDofType type) const { return this->dofsPerNode.contains(type); }
bool RElementDescriptor::supports(RElementFormulation formulation) const { return this->formulations.contains(formulation); }

QJsonObject RElementDescriptor::toJson() const
{
    QJsonArray dofs;
    for (RDofType dof : this->dofsPerNode) dofs.append(RDofDescriptor::get(dof).getId());
    QJsonArray formulationsJson;
    for (RElementFormulation formulation : this->formulations)
        formulationsJson.append(RElementDescriptor::formulationToId(formulation));
    QJsonArray material;
    for (const QString &value : this->requiredMaterialProperties) material.append(value);
    QJsonArray loads;
    for (const QString &value : this->supportedLoads) loads.append(value);
    QJsonArray results;
    for (const QString &value : this->producedResults) results.append(value);
    return QJsonObject{
        {"type",RElement::getId(this->type)},
        {"topology",RElementDescriptor::topologyToId(this->topology)},
        {"order",int(this->order)},
        {"nodes",int(this->nNodes)},
        {"naturalDimension",int(this->naturalDimension)},
        {"dofsPerNode",dofs},
        {"formulations",formulationsJson},
        {"defaultIntegrationPoints",int(this->defaultIntegrationPoints)},
        {"consistentMass",this->consistentMass},
        {"lumpedMass",this->lumpedMass},
        {"requiredMaterialProperties",material},
        {"supportedLoads",loads},
        {"producedResults",results}
    };
}

const RElementDescriptor &RElementDescriptor::get(RElementType type)
{
    for (const RElementDescriptor &descriptor : descriptors())
    {
        if (descriptor.getType() == type) return descriptor;
    }
    return descriptors().first();
}

QList<RElementDescriptor> RElementDescriptor::getAll() { return descriptors(); }

QString RElementDescriptor::topologyToId(RElementTopology topology)
{
    switch (topology)
    {
        case RElementTopology::Point: return "point";
        case RElementTopology::Line: return "line";
        case RElementTopology::Triangle: return "triangle";
        case RElementTopology::Quadrilateral: return "quadrilateral";
        case RElementTopology::Tetrahedron: return "tetrahedron";
        case RElementTopology::Wedge: return "wedge";
        case RElementTopology::Hexahedron: return "hexahedron";
    }
    return QString();
}

QString RElementDescriptor::formulationToId(RElementFormulation formulation)
{
    switch (formulation)
    {
        case RElementFormulation::Auxiliary: return "auxiliary";
        case RElementFormulation::Truss3D: return "truss-3d";
        case RElementFormulation::Frame3D: return "frame-3d";
        case RElementFormulation::PlaneStress: return "plane-stress";
        case RElementFormulation::PlaneStrain: return "plane-strain";
        case RElementFormulation::Axisymmetric: return "axisymmetric";
        case RElementFormulation::Solid3D: return "solid-3d";
    }
    return QString();
}
