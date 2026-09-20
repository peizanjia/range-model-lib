#ifndef RML_DOF_H
#define RML_DOF_H

#include <QList>
#include <QString>

//! Nodal degrees of freedom supported by the model kernel.
enum class RDofType
{
    UX,
    UY,
    UZ,
    RX,
    RY,
    RZ,
    ElectricPotential,
    Temperature,
    Pressure
};

//! Physical field to which a degree of freedom belongs.
enum class RDofField
{
    Displacement,
    Rotation,
    Electric,
    Thermal,
    Pressure
};

//! Stable metadata for a nodal degree of freedom.
class RDofDescriptor
{
    private:

        RDofType type;
        QString id;
        QString name;
        QString unit;
        RDofField field;
        int component;

    public:

        RDofDescriptor(RDofType type = RDofType::UX,
                       const QString &id = QString(),
                       const QString &name = QString(),
                       const QString &unit = QString(),
                       RDofField field = RDofField::Displacement,
                       int component = -1);

        RDofType getType() const;
        const QString &getId() const;
        const QString &getName() const;
        const QString &getUnit() const;
        RDofField getField() const;
        int getComponent() const;

        static const RDofDescriptor &get(RDofType type);
        static const RDofDescriptor *find(const QString &id);
        static QList<RDofDescriptor> getAll();
};

#endif // RML_DOF_H
