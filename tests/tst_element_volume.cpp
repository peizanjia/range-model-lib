#include <QtTest>

#include "rml_element.h"

namespace
{
constexpr double tol = 1.0e-10;

std::vector<RNode> wedgeNodes()
{
    return {
        RNode(0.0,0.0,0.0), RNode(1.0,0.0,0.0), RNode(0.0,1.0,0.0),
        RNode(0.0,0.0,1.0), RNode(1.0,0.0,1.0), RNode(0.0,1.0,1.0)
    };
}

std::vector<RNode> hexaNodes()
{
    return {
        RNode(0.0,0.0,0.0), RNode(1.0,0.0,0.0),
        RNode(1.0,1.0,0.0), RNode(0.0,1.0,0.0),
        RNode(0.0,0.0,1.0), RNode(1.0,0.0,1.0),
        RNode(1.0,1.0,1.0), RNode(0.0,1.0,1.0)
    };
}

RElement makeElement(RElementType type)
{
    RElement element(type);
    for (uint i=0;i<element.size();i++)
    {
        element.setNodeId(i,i);
    }
    return element;
}
}

class TestElementVolume : public QObject
{
    Q_OBJECT

private slots:
    void integrationRules_data();
    void integrationRules();
    void jacobianAndAffinePatch_data();
    void jacobianAndAffinePatch();
    void volumeAndBoundaryFaces();
};

void TestElementVolume::integrationRules_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<uint>("nPoints");
    QTest::addColumn<double>("totalWeight");
    QTest::newRow("wedge6") << int(R_ELEMENT_WEDGE1) << 6u << 1.0;
    QTest::newRow("hex8") << int(R_ELEMENT_HEXA1) << 8u << 8.0;
}

void TestElementVolume::integrationRules()
{
    QFETCH(int,type);
    QFETCH(uint,nPoints);
    QFETCH(double,totalWeight);
    const RElementType elementType = static_cast<RElementType>(type);

    QCOMPARE(RElement::getNIntegrationPoints(elementType),nPoints);
    QVERIFY(qAbs(RElement::getTotalWeightFactor(elementType)-totalWeight) < tol);

    for (uint p=0;p<nPoints;p++)
    {
        const RElementShapeFunction &shape = RElement::getShapeFunction(elementType,p);
        const RRVector &N = shape.getN();
        const RRMatrix &dN = shape.getDN();
        double sumN = 0.0;
        RRVector sumDN(3,0.0);
        for (uint i=0;i<N.size();i++)
        {
            sumN += N[i];
            for (uint j=0;j<3;j++)
            {
                sumDN[j] += dN[i][j];
            }
        }
        QVERIFY(qAbs(sumN-1.0) < tol);
        for (uint j=0;j<3;j++)
        {
            QVERIFY(qAbs(sumDN[j]) < tol);
        }
    }

    const RRVector &massVector = RElement::getMassVector(elementType);
    const RRMatrix &massMatrix = RElement::getMassMatrix(elementType);
    double massSum = 0.0;
    for (uint i=0;i<massVector.size();i++)
    {
        QVERIFY(massVector[i] > 0.0);
        massSum += massVector[i];
        for (uint j=0;j<massVector.size();j++)
        {
            QVERIFY(qAbs(massMatrix[i][j]-massMatrix[j][i]) < tol);
        }
    }
    QVERIFY(qAbs(massSum-totalWeight) < tol);
}

void TestElementVolume::jacobianAndAffinePatch_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<double>("determinant");
    QTest::newRow("wedge6") << int(R_ELEMENT_WEDGE1) << 0.5;
    QTest::newRow("hex8") << int(R_ELEMENT_HEXA1) << 0.125;
}

void TestElementVolume::jacobianAndAffinePatch()
{
    QFETCH(int,type);
    QFETCH(double,determinant);
    const RElementType elementType = static_cast<RElementType>(type);
    const std::vector<RNode> nodes = elementType == R_ELEMENT_WEDGE1 ? wedgeNodes() : hexaNodes();
    const RElement element = makeElement(elementType);

    for (uint p=0;p<RElement::getNIntegrationPoints(elementType);p++)
    {
        RRMatrix J;
        RRMatrix Rt;
        QVERIFY(qAbs(element.findJacobian(nodes,p,J,Rt)-determinant) < tol);

        const RRMatrix &dN = RElement::getShapeFunction(elementType,p).getDN();
        for (uint coordinate=0;coordinate<3;coordinate++)
        {
            for (uint derivative=0;derivative<3;derivative++)
            {
                double reproduced = 0.0;
                for (uint i=0;i<element.size();i++)
                {
                    double globalDerivative = 0.0;
                    for (uint natural=0;natural<3;natural++)
                    {
                        globalDerivative += dN[i][natural]*J[natural][derivative];
                    }
                    const double value = coordinate == 0 ? nodes[i].getX()
                                       : coordinate == 1 ? nodes[i].getY()
                                                         : nodes[i].getZ();
                    reproduced += value*globalDerivative;
                }
                const double expected = coordinate == derivative ? 1.0 : 0.0;
                QVERIFY(qAbs(reproduced-expected) < tol);
            }
        }
    }
}

void TestElementVolume::volumeAndBoundaryFaces()
{
    const RElement wedge = makeElement(R_ELEMENT_WEDGE1);
    const RElement hexa = makeElement(R_ELEMENT_HEXA1);
    double volume = 0.0;

    QVERIFY(wedge.findVolume(wedgeNodes(),volume));
    QVERIFY(qAbs(volume-0.5) < tol);
    const std::vector<RElement> wedgeFaces = wedge.generateEdgeElements();
    QCOMPARE(wedgeFaces.size(),size_t(5));
    QCOMPARE(wedgeFaces[0].getType(),R_ELEMENT_TRI1);
    QCOMPARE(wedgeFaces[1].getType(),R_ELEMENT_TRI1);
    QCOMPARE(wedgeFaces[2].getType(),R_ELEMENT_QUAD1);

    QVERIFY(hexa.findVolume(hexaNodes(),volume));
    QVERIFY(qAbs(volume-1.0) < tol);
    const std::vector<RElement> hexaFaces = hexa.generateEdgeElements();
    QCOMPARE(hexaFaces.size(),size_t(6));
    for (const RElement &face : hexaFaces)
    {
        QCOMPARE(face.getType(),R_ELEMENT_QUAD1);
    }
}

QTEST_APPLESS_MAIN(TestElementVolume)

#include "tst_element_volume.moc"
