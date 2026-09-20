#include <QJsonArray>
#include <QtTest>

#include "rml_dof.h"
#include "rml_capability_registry.h"
#include "rml_element_descriptor.h"

class TestElementDescriptor : public QObject
{
    Q_OBJECT

private slots:
    void dofRegistryHasStableIds();
    void everyElementTypeHasDescriptor();
    void beamDeclaresRotations();
    void continuumElementsDeclareTranslations();
    void surfaceFormulationsAreExplicit();
    void descriptorJsonIsAgentQueryable();
    void registryPublishesElementDescriptor();
};

void TestElementDescriptor::dofRegistryHasStableIds()
{
    QCOMPARE(RDofDescriptor::getAll().size(),9);
    QCOMPARE(RDofDescriptor::get(RDofType::UX).getId(),QString("ux"));
    QCOMPARE(RDofDescriptor::get(RDofType::RZ).getUnit(),QString("rad"));
    QCOMPARE(RDofDescriptor::get(RDofType::ElectricPotential).getUnit(),QString("V"));
    QCOMPARE(RDofDescriptor::find("temperature")->getField(),RDofField::Thermal);
    QVERIFY(!RDofDescriptor::find("not-a-dof"));
}

void TestElementDescriptor::everyElementTypeHasDescriptor()
{
    QCOMPARE(RElementDescriptor::getAll().size(),int(R_ELEMENT_N_TYPES)-1);
    for (int value=int(R_ELEMENT_POINT); value<int(R_ELEMENT_N_TYPES); ++value)
    {
        const RElementType type = RElementType(value);
        const RElementDescriptor &descriptor = RElementDescriptor::get(type);
        QCOMPARE(descriptor.getType(),type);
        QCOMPARE(descriptor.getNNodes(),RElement::getNNodes(type));
        QVERIFY(!descriptor.getDofsPerNode().isEmpty());
        QVERIFY(!descriptor.getFormulations().isEmpty());
    }

    for (RElementType type : {R_ELEMENT_TRUSS1,R_ELEMENT_TRI1,R_ELEMENT_QUAD1,R_ELEMENT_TETRA1})
    {
        QCOMPARE(RElementDescriptor::get(type).getDefaultIntegrationPoints(),
                 RElement::getNIntegrationPoints(type));
    }
}

void TestElementDescriptor::beamDeclaresRotations()
{
    const RElementDescriptor &beam = RElementDescriptor::get(R_ELEMENT_BEAM1);
    QCOMPARE(beam.getDofsPerNode().size(),6);
    QVERIFY(beam.hasDof(RDofType::UX));
    QVERIFY(beam.hasDof(RDofType::RX));
    QVERIFY(beam.hasDof(RDofType::RY));
    QVERIFY(beam.hasDof(RDofType::RZ));
    QVERIFY(beam.supports(RElementFormulation::Frame3D));
}

void TestElementDescriptor::continuumElementsDeclareTranslations()
{
    for (RElementType type : {R_ELEMENT_TRI1,R_ELEMENT_QUAD1,R_ELEMENT_TETRA1,
                              R_ELEMENT_WEDGE1,R_ELEMENT_HEXA1})
    {
        const RElementDescriptor &descriptor = RElementDescriptor::get(type);
        QCOMPARE(descriptor.getDofsPerNode().size(),3);
        QVERIFY(descriptor.hasDof(RDofType::UX));
        QVERIFY(descriptor.hasDof(RDofType::UY));
        QVERIFY(descriptor.hasDof(RDofType::UZ));
        QVERIFY(!descriptor.hasDof(RDofType::RX));
    }
}

void TestElementDescriptor::surfaceFormulationsAreExplicit()
{
    const RElementDescriptor &triangle = RElementDescriptor::get(R_ELEMENT_TRI1);
    QVERIFY(triangle.supports(RElementFormulation::PlaneStress));
    QVERIFY(triangle.supports(RElementFormulation::PlaneStrain));
    QVERIFY(triangle.supports(RElementFormulation::Axisymmetric));
    QCOMPARE(triangle.getNaturalDimension(),2u);
}

void TestElementDescriptor::descriptorJsonIsAgentQueryable()
{
    const QJsonObject json = RElementDescriptor::get(R_ELEMENT_HEXA1).toJson();
    QCOMPARE(json.value("type").toString(),RElement::getId(R_ELEMENT_HEXA1));
    QCOMPARE(json.value("topology").toString(),QString("hexahedron"));
    QCOMPARE(json.value("nodes").toInt(),8);
    QVERIFY(json.value("dofsPerNode").toArray().contains("ux"));
    QVERIFY(json.value("formulations").toArray().contains("solid-3d"));
    QVERIFY(json.value("supportedLoads").toArray().contains("gravity"));
}

void TestElementDescriptor::registryPublishesElementDescriptor()
{
    const RCapability *beam = RCapabilityRegistry::getBuiltIn().find(
        RCapabilityDomain::Element,RElement::getId(R_ELEMENT_BEAM1));
    QVERIFY(beam);
    const QJsonObject descriptor = beam->getMetadata().value("descriptor").toObject();
    QVERIFY(descriptor.value("dofsPerNode").toArray().contains("rx"));
    QVERIFY(descriptor.value("formulations").toArray().contains("frame-3d"));
}

QTEST_APPLESS_MAIN(TestElementDescriptor)

#include "tst_element_descriptor.moc"
