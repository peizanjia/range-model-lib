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
};

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
