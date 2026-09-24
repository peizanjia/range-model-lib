#include <cmath>

#include <QCryptographicHash>

#include <rbl_error.h>

#include "rml_model.h"
#include "rml_structural_dynamics_restart_data.h"

namespace
{
void appendUInt(QByteArray &bytes, uint value)
{
    for (unsigned int byte = 0; byte < 4; ++byte)
    {
        bytes.append(char((value >> (8 * byte)) & 0xffu));
    }
}

void appendCoordinate(QByteArray &bytes, double coordinate)
{
    // Model ASCII output uses QTextStream's six-significant-digit default.
    // One guard digit makes the identifier survive ordinary ASCII round trips
    // and reversible solver scaling.  This is an identity check, not a
    // substitute for a geometric-tolerance comparison during remeshing.
    const QByteArray normalized = QString::number(coordinate == 0.0 ? 0.0 : coordinate,
                                                   'g',5).toLatin1();
    appendUInt(bytes,uint(normalized.size()));
    bytes.append(normalized);
}

QString digest(const QByteArray &bytes)
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
}

bool isDigest(const QString &value)
{
    if (value.size() != 64) return false;
    for (const QChar character : value)
    {
        if (!character.isDigit() && (character < QLatin1Char('a') || character > QLatin1Char('f')))
        {
            return false;
        }
    }
    return true;
}
}

void RStructuralDynamicsRestartData::clear()
{
    *this = RStructuralDynamicsRestartData();
}

void RStructuralDynamicsRestartData::validate() const
{
    if (!this->available) return;
    if (!std::isfinite(this->time) || !std::isfinite(this->fixedTimeStep) ||
        this->fixedTimeStep <= 0.0 || this->meshNodeCount == 0 ||
        this->meshElementCount == 0 || this->physicalDofCount == 0 ||
        !isDigest(this->meshFingerprint) || !isDigest(this->dofFingerprint) ||
        !isDigest(this->systemFingerprint))
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural restart metadata is incomplete or invalid.");
    }
    if (this->q.empty() || this->q.size() > 20000000u ||
        this->q.size() > this->physicalDofCount ||
        this->v.size() != this->q.size() || this->a.size() != this->q.size() ||
        this->externalForce.size() != this->q.size())
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural restart reduced-state vector lengths do not agree.");
    }
    const auto finiteVector = [](const std::vector<double> &values)
    {
        for (const double value : values) if (!std::isfinite(value)) return false;
        return true;
    };
    if (!finiteVector(this->q) || !finiteVector(this->v) ||
        !finiteVector(this->a) || !finiteVector(this->externalForce) ||
        !std::isfinite(this->cumulativeExternalWork) ||
        !std::isfinite(this->cumulativeDampingDissipation) ||
        !std::isfinite(this->initialMechanicalEnergy))
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural restart state contains a non-finite value.");
    }
}

bool RStructuralDynamicsRestartData::matches(
    const RModel &model,
    const std::vector<RStructuralDofRef> &orderedPhysicalDofs) const
{
    if (!this->available) return false;
    this->validate();
    if (this->meshNodeCount != model.getNNodes() ||
        this->meshElementCount != model.getNElements() ||
        this->physicalDofCount != orderedPhysicalDofs.size())
    {
        return false;
    }
    return this->meshFingerprint == computeMeshFingerprint(model) &&
           this->dofFingerprint == computeDofFingerprint(orderedPhysicalDofs);
}

QString RStructuralDynamicsRestartData::computeMeshFingerprint(const RModel &model)
{
    QByteArray bytes("range-fea/mesh/restart/v1/5sig");
    appendUInt(bytes,model.getNNodes());
    for (uint i = 0; i < model.getNNodes(); ++i)
    {
        const RNode &node = model.getNode(i);
        appendCoordinate(bytes,node.getX());
        appendCoordinate(bytes,node.getY());
        appendCoordinate(bytes,node.getZ());
    }
    appendUInt(bytes,model.getNElements());
    for (uint i = 0; i < model.getNElements(); ++i)
    {
        const RElement &element = model.getElement(i);
        appendUInt(bytes,uint(element.getType()));
        appendUInt(bytes,element.size());
        for (uint j = 0; j < element.size(); ++j) appendUInt(bytes,element.getNodeId(j));
    }
    return digest(bytes);
}

QString RStructuralDynamicsRestartData::computeDofFingerprint(
    const std::vector<RStructuralDofRef> &orderedPhysicalDofs)
{
    QByteArray bytes("range-fea/dof/restart/v1");
    appendUInt(bytes,uint(orderedPhysicalDofs.size()));
    for (const RStructuralDofRef &reference : orderedPhysicalDofs)
    {
        appendUInt(bytes,reference.nodeId);
        appendUInt(bytes,uint(reference.dof));
    }
    return digest(bytes);
}
