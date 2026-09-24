#include <rbl_error.h>

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
        if (!this->controllerId.isEmpty() || this->deterministicSeed != 0)
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
}
