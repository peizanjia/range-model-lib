#ifndef RML_STRUCTURAL_CONTROL_SETUP_H
#define RML_STRUCTURAL_CONTROL_SETUP_H

#include <QtGlobal>
#include <QString>

//! Portable Stage-6 control-session configuration, independent of GUI objects.
//! Protocol v1 samples one controller command per accepted fixed solver step.
struct RStructuralControlSetup
{
    bool enabled = false;
    uint controlProtocolVersion = 1;
    QString controllerId;
    quint64 deterministicSeed = 0;

    void validate() const;
};

#endif // RML_STRUCTURAL_CONTROL_SETUP_H
