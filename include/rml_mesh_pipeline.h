#ifndef RML_MESH_PIPELINE_H
#define RML_MESH_PIPELINE_H

#include <QMap>
#include <QString>
#include <QStringList>

#include <vector>

#include "rml_element.h"
#include "rml_node.h"

enum class RMeshBackendType
{
    Automatic,
    TetGen,
    Gmsh
};

enum class RMeshPrimitiveType
{
    Line,
    Curve,
    Rectangle,
    Box
};

struct RMeshLocalSize
{
    RNode center;
    double size = 0.0;
    double radius = 0.0;
};

enum class RMeshAssignmentKind
{
    Material,
    Constraint,
    Sensor,
    Actuator
};

enum class RMeshLineFamily
{
    Truss,
    Beam
};

struct RMeshAssignment
{
    QString stableId;
    QString namedSet;
    RMeshAssignmentKind kind = RMeshAssignmentKind::Material;
};

//! Backend-independent request for the Stage 3 primitive meshing slice.
struct RMeshGenerationRequest
{
    RMeshBackendType backend = RMeshBackendType::Automatic;
    RMeshPrimitiveType primitive = RMeshPrimitiveType::Line;
    RNode origin = RNode(0.0,0.0,0.0);
    RR3Vector size = RR3Vector(1.0,1.0,1.0);
    double targetSize = 0.1;
    uint divisionsX = 0;
    uint divisionsY = 0;
    uint divisionsZ = 0;
    double biasX = 1.0;
    double biasY = 1.0;
    double biasZ = 1.0;
    uint elementOrder = 1;
    RMeshLineFamily lineFamily = RMeshLineFamily::Truss;
    bool mapped = false;
    bool recombine = false;
    QString domainName = "domain";
    //! Interpolation points for a Gmsh spline when primitive is Curve.
    std::vector<RNode> curvePoints;
    //! Local spherical/circular size fields. Outside radius, targetSize applies.
    std::vector<RMeshLocalSize> localSizes;
    std::vector<RMeshAssignment> assignments;
};

struct RMeshNamedSet
{
    QString name;
    uint dimension = 0;
    std::vector<uint> nodeIds;
    std::vector<uint> elementIds;
};

struct RMeshQualitySummary
{
    double minimumJacobian = 0.0;
    double maximumAspectRatio = 0.0;
    double maximumSkewness = 0.0;
    double minimumAngleDegrees = 0.0;
    uint invalidElementCount = 0;
};

struct RMeshGenerationResult
{
    QString backendId;
    std::vector<RNode> nodes;
    std::vector<RElement> elements;
    QMap<QString,RMeshNamedSet> namedSets;
    std::vector<RMeshAssignment> transferredAssignments;
    std::vector<RMeshAssignment> rejectedAssignments;
    RMeshQualitySummary quality;
};

//! Backend-independent mesh checks used by every mesher and importer.
class RMeshQuality
{
    public:
        static RMeshQualitySummary evaluate(const std::vector<RNode> &nodes,
                                            const std::vector<RElement> &elements);
};

#endif /* RML_MESH_PIPELINE_H */
