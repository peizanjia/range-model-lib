#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <set>

#include <rbl_error.h>

#include "rml_mesh_generator.h"

#ifdef RANGE_HAVE_GMSH
#ifdef RANGE_GMSH_C_WRAPPER
#include <gmsh.h_cwrap>
#else
#include <gmsh.h>
#endif
#endif

namespace
{
#ifdef RANGE_HAVE_GMSH
std::mutex gmshMutex;

class GmshSession
{
    public:
        GmshSession()
        {
            gmsh::initialize(0,nullptr,false,false);
            gmsh::option::setNumber("General.Terminal",0.0);
        }
        ~GmshSession()
        {
            if (gmsh::isInitialized()) gmsh::finalize();
        }
};

RElementType mapElementType(int gmshType)
{
    switch (gmshType)
    {
        case 1: return R_ELEMENT_TRUSS1;
        case 8: return R_ELEMENT_TRUSS2;
        case 2: return R_ELEMENT_TRI1;
        case 9: return R_ELEMENT_TRI2;
        case 3: return R_ELEMENT_QUAD1;
        case 10: return R_ELEMENT_QUAD2;
        case 4: return R_ELEMENT_TETRA1;
        case 5: return R_ELEMENT_HEXA1;
        case 6: return R_ELEMENT_WEDGE1;
        default: return R_ELEMENT_NONE;
    }
}

uint requestDimension(const RMeshGenerationRequest &request)
{
    if (request.primitive == RMeshPrimitiveType::Line || request.primitive == RMeshPrimitiveType::Curve) return 1;
    if (request.primitive == RMeshPrimitiveType::Rectangle) return 2;
    return 3;
}

uint divisions(double length, double targetSize, uint requested);
void addPhysical(int dimension, const std::vector<int> &entities, const QString &name);

void buildCurve(const RMeshGenerationRequest &request)
{
    if (request.curvePoints.size() < 2)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Curve meshing requires at least two interpolation points.");
    }
    std::vector<int> pointTags;
    for (const RNode &point : request.curvePoints)
        pointTags.push_back(gmsh::model::geo::addPoint(point.getX(),point.getY(),point.getZ(),request.targetSize));
    const int curve=gmsh::model::geo::addSpline(pointTags);
    gmsh::model::geo::synchronize();
    double length=0.0;
    for (uint i=1;i<request.curvePoints.size();i++) length += request.curvePoints[i-1].getDistance(request.curvePoints[i]);
    gmsh::model::mesh::setTransfiniteCurve(curve,int(divisions(length,request.targetSize,request.divisionsX)+1),"Progression",request.biasX);
    addPhysical(1,{curve},request.domainName);
    addPhysical(0,{pointTags.front()},"curve-start");
    addPhysical(0,{pointTags.back()},"curve-end");
}

uint divisions(double length, double targetSize, uint requested)
{
    if (requested > 0) return requested;
    return std::max(1u,uint(std::ceil(length/targetSize)));
}

void addPhysical(int dimension, const std::vector<int> &entities, const QString &name)
{
    gmsh::model::addPhysicalGroup(dimension,entities,-1,name.toStdString());
}

void buildLine(const RMeshGenerationRequest &request)
{
    const int p0 = gmsh::model::geo::addPoint(request.origin.getX(),request.origin.getY(),request.origin.getZ(),request.targetSize);
    const int p1 = gmsh::model::geo::addPoint(request.origin.getX()+request.size[0],request.origin.getY(),request.origin.getZ(),request.targetSize);
    const int curve = gmsh::model::geo::addLine(p0,p1);
    gmsh::model::geo::synchronize();
    gmsh::model::mesh::setTransfiniteCurve(curve,int(divisions(std::fabs(request.size[0]),request.targetSize,request.divisionsX)+1),"Progression",request.biasX);
    addPhysical(1,{curve},request.domainName);
    addPhysical(0,{p0},"x-min");
    addPhysical(0,{p1},"x-max");
}

void buildRectangle(const RMeshGenerationRequest &request)
{
    const double x=request.origin.getX(), y=request.origin.getY(), z=request.origin.getZ();
    const double dx=request.size[0], dy=request.size[1];
    const int p0=gmsh::model::geo::addPoint(x,y,z,request.targetSize);
    const int p1=gmsh::model::geo::addPoint(x+dx,y,z,request.targetSize);
    const int p2=gmsh::model::geo::addPoint(x+dx,y+dy,z,request.targetSize);
    const int p3=gmsh::model::geo::addPoint(x,y+dy,z,request.targetSize);
    const int bottom=gmsh::model::geo::addLine(p0,p1);
    const int right=gmsh::model::geo::addLine(p1,p2);
    const int top=gmsh::model::geo::addLine(p2,p3);
    const int left=gmsh::model::geo::addLine(p3,p0);
    const int loop=gmsh::model::geo::addCurveLoop({bottom,right,top,left});
    const int surface=gmsh::model::geo::addPlaneSurface({loop});
    gmsh::model::geo::synchronize();
    if (request.mapped)
    {
        const int nx=int(divisions(std::fabs(dx),request.targetSize,request.divisionsX)+1);
        const int ny=int(divisions(std::fabs(dy),request.targetSize,request.divisionsY)+1);
        gmsh::model::mesh::setTransfiniteCurve(bottom,nx,"Progression",request.biasX);
        gmsh::model::mesh::setTransfiniteCurve(top,nx,"Progression",request.biasX);
        gmsh::model::mesh::setTransfiniteCurve(right,ny,"Progression",request.biasY);
        gmsh::model::mesh::setTransfiniteCurve(left,ny,"Progression",request.biasY);
        gmsh::model::mesh::setTransfiniteSurface(surface,"Left",{p0,p1,p2,p3});
    }
    if (request.recombine) gmsh::model::mesh::setRecombine(2,surface);
    addPhysical(2,{surface},request.domainName);
    addPhysical(1,{left},"x-min");
    addPhysical(1,{right},"x-max");
    addPhysical(1,{bottom},"y-min");
    addPhysical(1,{top},"y-max");
}

void buildBox(const RMeshGenerationRequest &request)
{
    const int volume=gmsh::model::occ::addBox(request.origin.getX(),request.origin.getY(),request.origin.getZ(),
                                              request.size[0],request.size[1],request.size[2]);
    gmsh::model::occ::synchronize();
    gmsh::vectorpair points;
    gmsh::model::getEntities(points,0);
    gmsh::model::mesh::setSize(points,request.targetSize);
    gmsh::vectorpair boundary;
    gmsh::model::getBoundary({{3,volume}},boundary,true,false,false);
    std::vector<int> surfaces;
    for (const auto &entry : boundary) if (entry.first == 2) surfaces.push_back(entry.second);
    addPhysical(3,{volume},request.domainName);
    addPhysical(2,surfaces,"boundary");
}

void applyLocalSizeFields(const RMeshGenerationRequest &request)
{
    std::vector<double> fields;
    for (const RMeshLocalSize &local : request.localSizes)
    {
        if (local.size <= 0.0 || local.radius <= 0.0)
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Local mesh sizes and radii must be positive.");
        const int field=gmsh::model::mesh::field::add("Ball");
        gmsh::model::mesh::field::setNumber(field,"XCenter",local.center.getX());
        gmsh::model::mesh::field::setNumber(field,"YCenter",local.center.getY());
        gmsh::model::mesh::field::setNumber(field,"ZCenter",local.center.getZ());
        gmsh::model::mesh::field::setNumber(field,"Radius",local.radius);
        gmsh::model::mesh::field::setNumber(field,"VIn",local.size);
        gmsh::model::mesh::field::setNumber(field,"VOut",request.targetSize);
        fields.push_back(double(field));
    }
    if (fields.empty()) return;
    if (fields.size() == 1)
    {
        gmsh::model::mesh::field::setAsBackgroundMesh(int(fields.front()));
        return;
    }
    const int minimum=gmsh::model::mesh::field::add("Min");
    gmsh::model::mesh::field::setNumbers(minimum,"FieldsList",fields);
    gmsh::model::mesh::field::setAsBackgroundMesh(minimum);
}

RMeshGenerationResult generateGmsh(const RMeshGenerationRequest &request)
{
    if (request.targetSize <= 0.0 || request.elementOrder < 1 || request.elementOrder > 2)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Invalid Gmsh size or element order.");
    }
    if (request.primitive != RMeshPrimitiveType::Curve && (request.size[0] <= 0.0 ||
        (request.primitive != RMeshPrimitiveType::Line && request.size[1] <= 0.0) ||
        (request.primitive == RMeshPrimitiveType::Box && request.size[2] <= 0.0))
       )
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Mesh primitive dimensions must be positive.");
    }
    if (request.primitive == RMeshPrimitiveType::Box && request.elementOrder != 1)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Quadratic volume elements are not in the current Range element registry.");
    }

    std::lock_guard<std::mutex> lock(gmshMutex);
    GmshSession session;
    gmsh::model::add("range-stage3");
    if (request.primitive == RMeshPrimitiveType::Line) buildLine(request);
    else if (request.primitive == RMeshPrimitiveType::Curve) buildCurve(request);
    else if (request.primitive == RMeshPrimitiveType::Rectangle) buildRectangle(request);
    else buildBox(request);

    applyLocalSizeFields(request);

    const uint dimension=requestDimension(request);
    gmsh::model::mesh::generate(int(dimension));
    if (request.elementOrder == 2) gmsh::model::mesh::setOrder(2);

    RMeshGenerationResult result;
    result.backendId="gmsh-4";
    std::vector<std::size_t> nodeTags;
    std::vector<double> coordinates,parametric;
    gmsh::model::mesh::getNodes(nodeTags,coordinates,parametric,-1,-1,true,false);
    std::map<std::size_t,uint> nodeMap;
    for (uint i=0;i<nodeTags.size();i++)
    {
        nodeMap[nodeTags[i]]=i;
        result.nodes.emplace_back(coordinates[3*i],coordinates[3*i+1],coordinates[3*i+2]);
    }

    std::vector<int> elementTypes;
    std::vector<std::vector<std::size_t>> elementTags,elementNodeTags;
    gmsh::model::mesh::getElements(elementTypes,elementTags,elementNodeTags,int(dimension),-1);
    std::map<std::size_t,uint> elementMap;
    for (uint block=0;block<elementTypes.size();block++)
    {
        RElementType type=mapElementType(elementTypes[block]);
        if (dimension == 1 && request.lineFamily == RMeshLineFamily::Beam)
            type = request.elementOrder == 2 ? R_ELEMENT_BEAM2 : R_ELEMENT_BEAM1;
        if (type == R_ELEMENT_NONE) continue;
        const uint nNodes=RElement::getNNodes(type);
        for (uint e=0;e<elementTags[block].size();e++)
        {
            RElement element(type);
            for (uint n=0;n<nNodes;n++) element.setNodeId(n,nodeMap.at(elementNodeTags[block][e*nNodes+n]));
            elementMap[elementTags[block][e]]=uint(result.elements.size());
            result.elements.push_back(element);
        }
    }

    gmsh::vectorpair physicalGroups;
    gmsh::model::getPhysicalGroups(physicalGroups);
    for (const auto &group : physicalGroups)
    {
        std::string rawName;
        gmsh::model::getPhysicalName(group.first,group.second,rawName);
        RMeshNamedSet namedSet;
        namedSet.name=QString::fromStdString(rawName);
        namedSet.dimension=uint(group.first);
        std::vector<std::size_t> groupNodes;
        std::vector<double> groupCoordinates;
        gmsh::model::mesh::getNodesForPhysicalGroup(group.first,group.second,groupNodes,groupCoordinates);
        for (const std::size_t tag : groupNodes)
        {
            const auto found=nodeMap.find(tag);
            if (found != nodeMap.end()) namedSet.nodeIds.push_back(found->second);
        }
        std::vector<int> entities;
        gmsh::model::getEntitiesForPhysicalGroup(group.first,group.second,entities);
        std::set<uint> uniqueElements;
        for (const int entity : entities)
        {
            std::vector<int> types;
            std::vector<std::vector<std::size_t>> tags,nodesByElement;
            gmsh::model::mesh::getElements(types,tags,nodesByElement,group.first,entity);
            for (const auto &tagBlock : tags)
                for (const std::size_t tag : tagBlock)
                {
                    const auto found=elementMap.find(tag);
                    if (found != elementMap.end()) uniqueElements.insert(found->second);
                }
        }
        namedSet.elementIds.assign(uniqueElements.begin(),uniqueElements.end());
        result.namedSets.insert(namedSet.name,namedSet);
    }

    for (const RMeshAssignment &assignment : request.assignments)
    {
        if (result.namedSets.contains(assignment.namedSet)) result.transferredAssignments.push_back(assignment);
        else result.rejectedAssignments.push_back(assignment);
    }
    result.quality=RMeshQuality::evaluate(result.nodes,result.elements);
    return result;
}
#endif
}

QStringList RMeshGenerator::getAvailableBackends()
{
    QStringList backends{"tetgen"};
#ifdef RANGE_HAVE_GMSH
    backends.push_back("gmsh-4");
#endif
    return backends;
}

RMeshGenerationResult RMeshGenerator::generate(const RMeshGenerationRequest &request)
{
    RMeshBackendType backend=request.backend;
    if (backend == RMeshBackendType::Automatic) backend=RMeshBackendType::Gmsh;
#ifdef RANGE_HAVE_GMSH
    if (backend == RMeshBackendType::Gmsh) return generateGmsh(request);
#endif
    if (backend == RMeshBackendType::TetGen)
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                     "TetGen remains available for triangulated RModel volume meshing; it does not implement primitive 1D/2D generation.");
    }
    throw RError(RError::Type::Application,R_ERROR_REF,
                 "The Gmsh backend is not available in this build. Configure RANGE_GMSH_ROOT.");
}

void RMeshGenerator::apply(const RMeshGenerationResult &result, RModel &model)
{
    model=RModel();
    model.setNNodes(uint(result.nodes.size()));
    for (uint i=0;i<result.nodes.size();i++) model.setNode(i,result.nodes[i]);
    model.setNElements(uint(result.elements.size()));
    for (uint i=0;i<result.elements.size();i++) model.setElement(i,result.elements[i],true);
}
