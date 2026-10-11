#include <QtTest>

#include <cmath>

#include "rml_element.h"

class TestElement : public QObject
{
    Q_OBJECT

private slots:

    void nodeCountByType();
    void typeChangeResizesNodes();
    void nodeIds();
    void hasNodeId();
    void names();
    void localTransformationIsBlockDiagonal();
    void quadraticShapeFunctions();
    void quadraticTopologyUtilities();
    void linearTrussInterpolation();
};

void TestElement::linearTrussInterpolation()
{
    const std::vector<RNode> nodes{RNode(3,4,5),RNode(5,6,7)};
    RElement element(R_ELEMENT_TRUSS1);element.setNodeId(0,0);element.setNodeId(1,1);
    RRVector values(2);values[0]=-2;values[1]=10;
    for (double t : {0.0,.25,.5,.75,1.0})
    {
        const RNode location(3+2*t,4+2*t,5+2*t);
        const double expected=-2+12*t;
        QVERIFY(std::abs(element.interpolate(nodes,location,values)-expected)<1e-14);
        RRVector distances(2);distances[0]=std::sqrt(12.0)*t;distances[1]=std::sqrt(12.0)*(1-t);
        QVERIFY(std::abs(element.interpolate(nodes,location,values,distances)-expected)<1e-14);
    }
    element.swapNodeIds(0,1);std::swap(values[0],values[1]);
    QCOMPARE(element.interpolate(nodes,nodes[1],values),10.0);
    QCOMPARE(element.interpolate(nodes,nodes[0],values),-2.0);
}

void TestElement::nodeCountByType()
{
    QCOMPARE(RElement::getNNodes(R_ELEMENT_POINT), 1u);
    QCOMPARE(RElement::getNNodes(R_ELEMENT_TRUSS1), 2u);
    QCOMPARE(RElement::getNNodes(R_ELEMENT_TRI1), 3u);
    QCOMPARE(RElement::getNNodes(R_ELEMENT_TETRA1), 4u);
}

void TestElement::typeChangeResizesNodes()
{
    RElement element(R_ELEMENT_TRI1);
    QCOMPARE(element.getType(), R_ELEMENT_TRI1);
    QCOMPARE(element.size(), 3u);

    element.setType(R_ELEMENT_TETRA1);
    QCOMPARE(element.getType(), R_ELEMENT_TETRA1);
    QCOMPARE(element.size(), 4u);
}

void TestElement::nodeIds()
{
    RElement element(R_ELEMENT_TRI1);
    element.setNodeId(0, 10);
    element.setNodeId(1, 20);
    element.setNodeId(2, 30);

    QCOMPARE(element.getNodeId(0), 10u);
    QCOMPARE(element.getNodeId(1), 20u);
    QCOMPARE(element.getNodeId(2), 30u);

    element.swapNodeIds(0, 2);
    QCOMPARE(element.getNodeId(0), 30u);
    QCOMPARE(element.getNodeId(2), 10u);
}

void TestElement::hasNodeId()
{
    RElement element(R_ELEMENT_TRUSS1);
    element.setNodeId(0, 5);
    element.setNodeId(1, 7);

    QVERIFY(element.hasNodeId(5));
    QVERIFY(element.hasNodeId(7));
    QVERIFY(!element.hasNodeId(99));
}

void TestElement::names()
{
    QCOMPARE(RElement::getName(R_ELEMENT_POINT), QString("Point"));
    QCOMPARE(RElement::getName(R_ELEMENT_TRI1), QString("Triangle1"));
}

void TestElement::quadraticShapeFunctions()
{
    struct Expected { RElementType type; uint points; uint nodes; uint dimensions; double totalWeight; };
    const Expected expected[] = {
        {R_ELEMENT_TRUSS2,3,3,1,2.0},
        {R_ELEMENT_TRI2,3,6,2,0.5},
        {R_ELEMENT_QUAD2,9,9,2,4.0}
    };
    for (const Expected &item : expected)
    {
        QCOMPARE(RElement::getNIntegrationPoints(item.type),item.points);
        double totalWeight = 0.0;
        for (uint point=0;point<item.points;point++)
        {
            const RElementShapeFunction &shape = RElement::getShapeFunction(item.type,point);
            QCOMPARE(shape.getN().size(),item.nodes);
            QCOMPARE(shape.getDN().getNRows(),item.nodes);
            QCOMPARE(shape.getDN().getNColumns(),item.dimensions);
            double partition = 0.0;
            for (uint node=0;node<item.nodes;node++) partition += shape.getN()[node];
            QVERIFY(std::fabs(partition-1.0) < 1.0e-12);
            for (uint dimension=0;dimension<item.dimensions;dimension++)
            {
                double derivativeSum = 0.0;
                for (uint node=0;node<item.nodes;node++) derivativeSum += shape.getDN()[node][dimension];
                QVERIFY(std::fabs(derivativeSum) < 1.0e-12);
            }
            totalWeight += shape.getW();
        }
        QVERIFY(std::fabs(totalWeight-item.totalWeight) < 1.0e-12);
    }
}

void TestElement::quadraticTopologyUtilities()
{
    std::vector<RNode> triangleNodes = {
        RNode(0.0,0.0,0.0),RNode(1.0,0.0,0.0),RNode(0.0,1.0,0.0),
        RNode(0.5,0.0,0.0),RNode(0.5,0.5,0.0),RNode(0.0,0.5,0.0)
    };
    RElement triangle(R_ELEMENT_TRI2);
    for (uint i=0;i<6;i++) triangle.setNodeId(i,i);
    QCOMPARE(triangle.getNEdgeElements(),3u);
    const std::vector<RElement> triangleEdges = triangle.generateEdgeElements();
    QCOMPARE(triangleEdges.size(),std::size_t(3));
    QCOMPARE(triangleEdges[0].getType(),R_ELEMENT_TRUSS2);
    QCOMPARE(triangleEdges[0].getNodeId(2),3u);
    QCOMPARE(triangle.triangulate(triangleNodes).size(),4);
    QVERIFY(triangle.nodeIsOnEdge(3,0));
    QVERIFY(!triangle.nodeIsOnEdge(3,1));
    triangle.swapNormal();
    QCOMPARE(triangle.getNodeId(4),5u);
    QCOMPARE(triangle.getNodeId(5),4u);

    std::vector<RNode> quadNodes = {
        RNode(0.0,0.0,0.0),RNode(1.0,0.0,0.0),RNode(1.0,1.0,0.0),RNode(0.0,1.0,0.0),
        RNode(0.5,0.0,0.0),RNode(1.0,0.5,0.0),RNode(0.5,1.0,0.0),RNode(0.0,0.5,0.0),
        RNode(0.5,0.5,0.0)
    };
    RElement quad(R_ELEMENT_QUAD2);
    for (uint i=0;i<9;i++) quad.setNodeId(i,i);
    QCOMPARE(quad.getNEdgeElements(),4u);
    QCOMPARE(quad.generateEdgeElements().size(),std::size_t(4));
    QCOMPARE(quad.triangulate(quadNodes).size(),8);
    QVERIFY(quad.nodeIsOnEdge(7,3));
    QVERIFY(!quad.nodeIsOnEdge(8,0));
    quad.swapNormal();
    QCOMPARE(quad.getNodeId(4),7u);
    QCOMPARE(quad.getNodeId(5),6u);
}

void TestElement::localTransformationIsBlockDiagonal()
{
    std::vector<RNode> lineNodes = {RNode(0.0,0.0,0.0),RNode(1.0,1.0,0.0)};
    RElement line(R_ELEMENT_TRUSS1);
    line.setNodeId(0,0);
    line.setNodeId(1,1);
    RRMatrix J, Rt;
    line.findJacobian(lineNodes,0,J,Rt);
    QCOMPARE(Rt.getNRows(),6u);
    QCOMPARE(Rt.getNColumns(),2u);
    for (uint node=0;node<2;node++)
    {
        double norm = 0.0;
        for (uint row=0;row<3;row++)
        {
            norm += Rt[3*node+row][node]*Rt[3*node+row][node];
            QCOMPARE(Rt[3*node+row][1-node],0.0);
        }
        QVERIFY(std::fabs(norm-1.0) < 1.0e-12);
    }

    std::vector<RNode> surfaceNodes = {
        RNode(0.0,0.0,0.0),RNode(1.0,0.0,0.0),RNode(0.0,1.0,0.0)
    };
    RElement surface(R_ELEMENT_TRI1);
    surface.setNodeId(0,0);
    surface.setNodeId(1,1);
    surface.setNodeId(2,2);
    surface.findJacobian(surfaceNodes,0,J,Rt);
    QCOMPARE(Rt.getNRows(),9u);
    QCOMPARE(Rt.getNColumns(),6u);
    for (uint node=0;node<3;node++)
    {
        double norm0 = 0.0;
        double norm1 = 0.0;
        double dot = 0.0;
        for (uint row=0;row<3;row++)
        {
            const double a = Rt[3*node+row][2*node];
            const double b = Rt[3*node+row][2*node+1];
            norm0 += a*a;
            norm1 += b*b;
            dot += a*b;
            for (uint other=0;other<3;other++)
            {
                if (other != node)
                {
                    QCOMPARE(Rt[3*node+row][2*other],0.0);
                    QCOMPARE(Rt[3*node+row][2*other+1],0.0);
                }
            }
        }
        QVERIFY(std::fabs(norm0-1.0) < 1.0e-12);
        QVERIFY(std::fabs(norm1-1.0) < 1.0e-12);
        QVERIFY(std::fabs(dot) < 1.0e-12);
    }
}

QTEST_APPLESS_MAIN(TestElement)

#include "tst_element.moc"
