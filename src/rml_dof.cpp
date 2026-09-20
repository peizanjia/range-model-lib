#include "rml_dof.h"

namespace
{

const QList<RDofDescriptor> &descriptors()
{
    static const QList<RDofDescriptor> values{
        {RDofType::UX,"ux","Displacement X","m",RDofField::Displacement,0},
        {RDofType::UY,"uy","Displacement Y","m",RDofField::Displacement,1},
        {RDofType::UZ,"uz","Displacement Z","m",RDofField::Displacement,2},
        {RDofType::RX,"rx","Rotation X","rad",RDofField::Rotation,0},
        {RDofType::RY,"ry","Rotation Y","rad",RDofField::Rotation,1},
        {RDofType::RZ,"rz","Rotation Z","rad",RDofField::Rotation,2},
        {RDofType::ElectricPotential,"electric-potential","Electric potential","V",RDofField::Electric,-1},
        {RDofType::Temperature,"temperature","Temperature","K",RDofField::Thermal,-1},
        {RDofType::Pressure,"pressure","Pressure","Pa",RDofField::Pressure,-1}
    };
    return values;
}

}

RDofDescriptor::RDofDescriptor(RDofType type,
                               const QString &id,
                               const QString &name,
                               const QString &unit,
                               RDofField field,
                               int component)
    : type(type)
    , id(id)
    , name(name)
    , unit(unit)
    , field(field)
    , component(component)
{
}

RDofType RDofDescriptor::getType() const { return this->type; }
const QString &RDofDescriptor::getId() const { return this->id; }
const QString &RDofDescriptor::getName() const { return this->name; }
const QString &RDofDescriptor::getUnit() const { return this->unit; }
RDofField RDofDescriptor::getField() const { return this->field; }
int RDofDescriptor::getComponent() const { return this->component; }

const RDofDescriptor &RDofDescriptor::get(RDofType type)
{
    for (const RDofDescriptor &descriptor : descriptors())
    {
        if (descriptor.getType() == type)
        {
            return descriptor;
        }
    }
    return descriptors().first();
}

const RDofDescriptor *RDofDescriptor::find(const QString &id)
{
    for (const RDofDescriptor &descriptor : descriptors())
    {
        if (descriptor.getId() == id)
        {
            return &descriptor;
        }
    }
    return nullptr;
}

QList<RDofDescriptor> RDofDescriptor::getAll()
{
    return descriptors();
}
