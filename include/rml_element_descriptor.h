#ifndef RML_ELEMENT_DESCRIPTOR_H
#define RML_ELEMENT_DESCRIPTOR_H

#include <QJsonObject>
#include <QList>
#include <QStringList>

#include "rml_dof.h"
#include "rml_element.h"

enum class RElementTopology
{
    Point,
    Line,
    Triangle,
    Quadrilateral,
    Tetrahedron,
    Wedge,
    Hexahedron
};

enum class RElementFormulation
{
    Auxiliary,
    Truss3D,
    Frame3D,
    PlaneStress,
    PlaneStrain,
    Axisymmetric,
    Solid3D
};

//! Declarative description of an element's nodal and solver contract.
class RElementDescriptor
{
    private:

        RElementType type;
        RElementTopology topology;
        uint order;
        uint nNodes;
        uint naturalDimension;
        QList<RDofType> dofsPerNode;
        QList<RElementFormulation> formulations;
        uint defaultIntegrationPoints;
        bool consistentMass;
        bool lumpedMass;
        QStringList requiredMaterialProperties;
        QStringList supportedLoads;
        QStringList producedResults;

    public:

        RElementDescriptor(RElementType type = R_ELEMENT_NONE,
                           RElementTopology topology = RElementTopology::Point,
                           uint order = 0,
                           uint nNodes = 0,
                           uint naturalDimension = 0,
                           const QList<RDofType> &dofsPerNode = QList<RDofType>(),
                           const QList<RElementFormulation> &formulations = QList<RElementFormulation>(),
                           uint defaultIntegrationPoints = 0,
                           bool consistentMass = false,
                           bool lumpedMass = false,
                           const QStringList &requiredMaterialProperties = QStringList(),
                           const QStringList &supportedLoads = QStringList(),
                           const QStringList &producedResults = QStringList());

        RElementType getType() const;
        RElementTopology getTopology() const;
        uint getOrder() const;
        uint getNNodes() const;
        uint getNaturalDimension() const;
        const QList<RDofType> &getDofsPerNode() const;
        const QList<RElementFormulation> &getFormulations() const;
        uint getDefaultIntegrationPoints() const;
        bool hasConsistentMass() const;
        bool hasLumpedMass() const;
        const QStringList &getRequiredMaterialProperties() const;
        const QStringList &getSupportedLoads() const;
        const QStringList &getProducedResults() const;
        bool hasDof(RDofType type) const;
        bool supports(RElementFormulation formulation) const;

        QJsonObject toJson() const;

        static const RElementDescriptor &get(RElementType type);
        static QList<RElementDescriptor> getAll();
        static QString topologyToId(RElementTopology topology);
        static QString formulationToId(RElementFormulation formulation);
};

#endif // RML_ELEMENT_DESCRIPTOR_H
