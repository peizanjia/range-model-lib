#include <QtTest>

#include "rml_mesh_generator.h"

class TestMeshPipeline : public QObject
{
    Q_OBJECT

private slots:
    void qualityMetrics();
    void tetGenVolume();
    void gmshLine();
    void gmshCurve();
    void gmshSurface_data();
    void gmshSurface();
    void gmshVolume();
};

void TestMeshPipeline::qualityMetrics()
{
    std::vector<RNode> nodes={RNode(0,0,0),RNode(1,0,0),RNode(0,1,0)};
    RElement triangle(R_ELEMENT_TRI1);
    triangle.setNodeId(0,0); triangle.setNodeId(1,1); triangle.setNodeId(2,2);
    const RMeshQualitySummary quality=RMeshQuality::evaluate(nodes,{triangle});
    QVERIFY(quality.minimumJacobian > 0.0);
    QVERIFY(quality.maximumAspectRatio >= 1.0);
    QVERIFY(quality.maximumSkewness >= 0.0);
    QCOMPARE(quality.minimumAngleDegrees,45.0);
    QCOMPARE(quality.invalidElementCount,0u);
}

void TestMeshPipeline::tetGenVolume()
{
    const std::vector<RNode> n={RNode(0,0,0),RNode(1,0,0),RNode(1,1,0),RNode(0,1,0),
                                RNode(0,0,1),RNode(1,0,1),RNode(1,1,1),RNode(0,1,1)};
    const uint f[12][3]={{0,2,1},{0,3,2},{4,5,6},{4,6,7},{0,1,5},{0,5,4},
                         {1,2,6},{1,6,5},{2,3,7},{2,7,6},{3,0,4},{3,4,7}};
    std::vector<RTriangle> triangles;
    for (const auto &face : f) triangles.emplace_back(n[face[0]],n[face[1]],n[face[2]]);
    const std::vector<RTetrahedron> tetrahedra=RMeshGenerator::generate(triangles);
    QVERIFY(!tetrahedra.empty());
}

void TestMeshPipeline::gmshLine()
{
#ifndef RANGE_HAVE_GMSH
    QSKIP("Gmsh SDK not configured");
#else
    RMeshGenerationRequest request;
    request.primitive=RMeshPrimitiveType::Line;
    request.size=RR3Vector(2.0,0.0,0.0);
    request.divisionsX=4;
    request.biasX=1.5;
    request.elementOrder=2;
    request.lineFamily=RMeshLineFamily::Beam;
    request.assignments.push_back({"sensor-left","x-min",RMeshAssignmentKind::Sensor});
    request.assignments.push_back({"missing","not-a-set",RMeshAssignmentKind::Constraint});
    const RMeshGenerationResult result=RMeshGenerator::generate(request);
    QCOMPARE(result.elements.size(),size_t(4));
    for (const RElement &element : result.elements) QCOMPARE(element.getType(),R_ELEMENT_BEAM2);
    QVERIFY(result.namedSets.contains("x-min"));
    QCOMPARE(result.transferredAssignments.size(),size_t(1));
    QCOMPARE(result.rejectedAssignments.size(),size_t(1));
    QCOMPARE(result.quality.invalidElementCount,0u);
#endif
}

void TestMeshPipeline::gmshCurve()
{
#ifndef RANGE_HAVE_GMSH
    QSKIP("Gmsh SDK not configured");
#else
    RMeshGenerationRequest request;
    request.primitive=RMeshPrimitiveType::Curve;
    request.curvePoints={RNode(1,0,0),RNode(0.7071067811865476,0.7071067811865476,0),RNode(0,1,0)};
    request.divisionsX=8;
    const RMeshGenerationResult result=RMeshGenerator::generate(request);
    QCOMPARE(result.elements.size(),size_t(8));
    QVERIFY(result.namedSets.contains("curve-start"));
    QVERIFY(result.namedSets.contains("curve-end"));
    QCOMPARE(result.quality.invalidElementCount,0u);
#endif
}

void TestMeshPipeline::gmshSurface_data()
{
    QTest::addColumn<bool>("mapped");
    QTest::addColumn<bool>("recombine");
    QTest::addColumn<RElementType>("expectedType");
    QTest::newRow("constrained triangles") << false << false << R_ELEMENT_TRI1;
    QTest::newRow("mapped quads") << true << true << R_ELEMENT_QUAD1;
}

void TestMeshPipeline::gmshSurface()
{
#ifndef RANGE_HAVE_GMSH
    QSKIP("Gmsh SDK not configured");
#else
    QFETCH(bool,mapped);
    QFETCH(bool,recombine);
    QFETCH(RElementType,expectedType);
    RMeshGenerationRequest request;
    request.primitive=RMeshPrimitiveType::Rectangle;
    request.size=RR3Vector(2.0,1.0,0.0);
    request.targetSize=0.25;
    if (!mapped) request.localSizes.push_back({RNode(1.0,0.5,0.0),0.08,0.3});
    request.divisionsX=8;
    request.divisionsY=4;
    request.mapped=mapped;
    request.recombine=recombine;
    request.assignments.push_back({"steel","domain",RMeshAssignmentKind::Material});
    request.assignments.push_back({"fixed","x-min",RMeshAssignmentKind::Constraint});
    const RMeshGenerationResult result=RMeshGenerator::generate(request);
    QVERIFY(!result.elements.empty());
    for (const RElement &element : result.elements) QCOMPARE(element.getType(),expectedType);
    QCOMPARE(result.transferredAssignments.size(),size_t(2));
    QCOMPARE(result.rejectedAssignments.size(),size_t(0));
    QCOMPARE(result.quality.invalidElementCount,0u);
    QVERIFY(result.quality.minimumAngleDegrees > 0.0);
#endif
}

void TestMeshPipeline::gmshVolume()
{
#ifndef RANGE_HAVE_GMSH
    QSKIP("Gmsh SDK not configured");
#else
    RMeshGenerationRequest request;
    request.primitive=RMeshPrimitiveType::Box;
    request.size=RR3Vector(1.0,0.5,0.25);
    request.targetSize=0.2;
    request.assignments.push_back({"solid-material","domain",RMeshAssignmentKind::Material});
    request.assignments.push_back({"surface-load","boundary",RMeshAssignmentKind::Actuator});
    const RMeshGenerationResult result=RMeshGenerator::generate(request);
    QVERIFY(!result.elements.empty());
    for (const RElement &element : result.elements) QCOMPARE(element.getType(),R_ELEMENT_TETRA1);
    QCOMPARE(result.transferredAssignments.size(),size_t(2));
    QCOMPARE(result.quality.invalidElementCount,0u);

    RModel model;
    RMeshGenerator::apply(result,model);
    QCOMPARE(model.getNNodes(),uint(result.nodes.size()));
    QCOMPARE(model.getNElements(),uint(result.elements.size()));
    QVERIFY(model.getNVolumes() > 0);
#endif
}

QTEST_APPLESS_MAIN(TestMeshPipeline)

#include "tst_mesh_pipeline.moc"
