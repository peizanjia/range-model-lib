#include <algorithm>
#include <cmath>
#include <limits>

#include "rml_mesh_pipeline.h"

namespace
{
double distance(const RNode &a, const RNode &b)
{
    const double dx = a.getX()-b.getX();
    const double dy = a.getY()-b.getY();
    const double dz = a.getZ()-b.getZ();
    return std::sqrt(dx*dx+dy*dy+dz*dz);
}

double angleDegrees(const RNode &a, const RNode &b, const RNode &c)
{
    const double ux = a.getX()-b.getX();
    const double uy = a.getY()-b.getY();
    const double uz = a.getZ()-b.getZ();
    const double vx = c.getX()-b.getX();
    const double vy = c.getY()-b.getY();
    const double vz = c.getZ()-b.getZ();
    const double un = std::sqrt(ux*ux+uy*uy+uz*uz);
    const double vn = std::sqrt(vx*vx+vy*vy+vz*vz);
    if (un <= 0.0 || vn <= 0.0) return 0.0;
    const double cosine = std::clamp((ux*vx+uy*vy+uz*vz)/(un*vn),-1.0,1.0);
    return std::acos(cosine)*180.0/std::acos(-1.0);
}

std::vector<std::pair<uint,uint>> edgePositions(RElementType type)
{
    switch (type)
    {
        case R_ELEMENT_TRUSS1:
        case R_ELEMENT_TRUSS2:
        case R_ELEMENT_BEAM1:
        case R_ELEMENT_BEAM2: return {{0,1}};
        case R_ELEMENT_TRI1:
        case R_ELEMENT_TRI2: return {{0,1},{1,2},{2,0}};
        case R_ELEMENT_QUAD1:
        case R_ELEMENT_QUAD2: return {{0,1},{1,2},{2,3},{3,0}};
        case R_ELEMENT_TETRA1: return {{0,1},{1,2},{2,0},{0,3},{1,3},{2,3}};
        case R_ELEMENT_WEDGE1: return {{0,1},{1,2},{2,0},{3,4},{4,5},{5,3},{0,3},{1,4},{2,5}};
        case R_ELEMENT_HEXA1: return {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
        default: return {};
    }
}
}

RMeshQualitySummary RMeshQuality::evaluate(const std::vector<RNode> &nodes,
                                           const std::vector<RElement> &elements)
{
    RMeshQualitySummary summary;
    if (elements.empty()) return summary;

    summary.minimumJacobian = std::numeric_limits<double>::max();
    summary.minimumAngleDegrees = 180.0;
    for (const RElement &element : elements)
    {
        double minimumElementJacobian = std::numeric_limits<double>::max();
        for (uint point=0;point<RElement::getNIntegrationPoints(element.getType());point++)
        {
            RRMatrix jacobian,rotation;
            minimumElementJacobian = std::min(minimumElementJacobian,
                                               element.findJacobian(nodes,point,jacobian,rotation));
        }
        summary.minimumJacobian = std::min(summary.minimumJacobian,minimumElementJacobian);
        if (minimumElementJacobian <= 0.0) summary.invalidElementCount++;

        const std::vector<std::pair<uint,uint>> edges = edgePositions(element.getType());
        double shortest = std::numeric_limits<double>::max();
        double longest = 0.0;
        for (const auto &edgeNodes : edges)
        {
            const double edge = distance(nodes[element.getNodeId(edgeNodes.first)],
                                         nodes[element.getNodeId(edgeNodes.second)]);
            if (edge > 0.0)
            {
                shortest = std::min(shortest,edge);
                longest = std::max(longest,edge);
            }
        }
        if (shortest < std::numeric_limits<double>::max())
        {
            summary.maximumAspectRatio = std::max(summary.maximumAspectRatio,longest/shortest);
        }

        if (element.getType() == R_ELEMENT_TRI1 || element.getType() == R_ELEMENT_TRI2)
        {
            for (uint i=0;i<3;i++)
            {
                const double angle = angleDegrees(nodes[element.getNodeId((i+2)%3)],
                                                  nodes[element.getNodeId(i)],
                                                  nodes[element.getNodeId((i+1)%3)]);
                summary.minimumAngleDegrees = std::min(summary.minimumAngleDegrees,angle);
                summary.maximumSkewness = std::max(summary.maximumSkewness,std::fabs(angle-60.0)/120.0);
            }
        }
        else if (element.getType() == R_ELEMENT_QUAD1 || element.getType() == R_ELEMENT_QUAD2)
        {
            for (uint i=0;i<4;i++)
            {
                const double angle = angleDegrees(nodes[element.getNodeId((i+3)%4)],
                                                  nodes[element.getNodeId(i)],
                                                  nodes[element.getNodeId((i+1)%4)]);
                summary.minimumAngleDegrees = std::min(summary.minimumAngleDegrees,angle);
                summary.maximumSkewness = std::max(summary.maximumSkewness,std::fabs(angle-90.0)/90.0);
            }
        }
        else if (element.getType() == R_ELEMENT_TETRA1)
        {
            const uint faces[4][3] = {{0,1,2},{0,3,1},{1,3,2},{2,3,0}};
            for (const auto &face : faces)
            {
                for (uint i=0;i<3;i++)
                {
                    const double angle = angleDegrees(nodes[element.getNodeId(face[(i+2)%3])],
                                                      nodes[element.getNodeId(face[i])],
                                                      nodes[element.getNodeId(face[(i+1)%3])]);
                    summary.minimumAngleDegrees = std::min(summary.minimumAngleDegrees,angle);
                    summary.maximumSkewness = std::max(summary.maximumSkewness,std::fabs(angle-60.0)/120.0);
                }
            }
        }
    }
    if (summary.minimumJacobian == std::numeric_limits<double>::max()) summary.minimumJacobian = 0.0;
    if (summary.minimumAngleDegrees == 180.0) summary.minimumAngleDegrees = 0.0;
    return summary;
}
