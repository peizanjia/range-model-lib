#include <rbl_error.h>

#include <cmath>
#include <set>

#include "rml_structural_control_setup.h"

void RStructuralControlSetup::validate() const
{
    if (this->controlProtocolVersion != 1)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Unsupported structural control protocol version (%u).",
                     this->controlProtocolVersion);
    }
    if (!this->enabled)
    {
        if (!this->controllerId.isEmpty() || this->deterministicSeed != 0 ||
            !this->channelConditioning.empty())
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Disabled structural control setup must not retain controller metadata.");
        return;
    }
    if (this->controllerId.isEmpty() || this->controllerId.size() > 128 ||
        this->controllerId.trimmed() != this->controllerId)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural controller ID must be nonempty, bounded and unpadded.");
    }
    for (const QChar character : this->controllerId)
    {
        if (character.isSpace() || character.isNull() || character == QLatin1Char('"'))
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Structural controller ID cannot contain whitespace, NUL or quotes.");
    }
    if (this->channelConditioning.size() > 4096u)
        throw RError(RError::Type::Application,R_ERROR_REF,
                     "Structural control conditioning exceeds 4096 actuator channels.");
    std::set<QString> ids;
    uint totalDelaySlots = 0;
    for (const auto &channel : this->channelConditioning)
    {
        if (channel.actuatorId.isEmpty() || channel.actuatorId.size() > 128 ||
            channel.actuatorId.trimmed() != channel.actuatorId ||
            !ids.insert(channel.actuatorId).second || channel.holdSteps == 0 ||
            channel.delaySteps > 16384u || totalDelaySlots > 16384u-channel.delaySteps ||
            !std::isfinite(channel.noiseAmplitude) || channel.noiseAmplitude < 0.0)
            throw RError(RError::Type::Application,R_ERROR_REF,
                         "Invalid structural control conditioning policy.");
        for (const QChar character : channel.actuatorId)
        {
            if (character.isSpace() || character.isNull() || character == QLatin1Char('"'))
                throw RError(RError::Type::Application,R_ERROR_REF,
                             "Invalid structural control conditioning actuator ID.");
        }
        totalDelaySlots += channel.delaySteps;
    }
}
