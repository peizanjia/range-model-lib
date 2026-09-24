#include <rbl_error.h>

#include "rml_parallel_execution_config.h"

RParallelExecutionMode RParallelExecutionConfig::getMode() const
{
    return this->mode;
}

void RParallelExecutionConfig::setMode(RParallelExecutionMode mode)
{
    if (mode != RParallelExecutionMode::Auto &&
        mode != RParallelExecutionMode::SerialDeterministic &&
        mode != RParallelExecutionMode::FixedThreads)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Unsupported parallel execution mode (%d).",int(mode));
    }
    this->mode = mode;
}

uint RParallelExecutionConfig::getRequestedThreads() const
{
    return this->requestedThreads;
}

void RParallelExecutionConfig::setRequestedThreads(uint requestedThreads)
{
    this->requestedThreads = requestedThreads;
}

bool RParallelExecutionConfig::getDeterministicReduction() const
{
    return this->deterministicReduction;
}

void RParallelExecutionConfig::setDeterministicReduction(bool deterministicReduction)
{
    this->deterministicReduction = deterministicReduction;
}

void RParallelExecutionConfig::validate() const
{
    if (this->mode != RParallelExecutionMode::Auto &&
        this->mode != RParallelExecutionMode::SerialDeterministic &&
        this->mode != RParallelExecutionMode::FixedThreads)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Unsupported parallel execution mode (%d).",int(this->mode));
    }
    if (this->mode == RParallelExecutionMode::FixedThreads && this->requestedThreads == 0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Fixed-thread execution requires at least one thread.");
    }
    if (this->mode != RParallelExecutionMode::FixedThreads && this->requestedThreads != 0)
    {
        throw RError(RError::Type::Application,R_ERROR_REF,"Requested thread count is only valid in fixed-thread mode.");
    }
}

QString RParallelExecutionConfig::toString() const
{
    return QString("{ Mode: %1, Requested threads: %2, Deterministic reduction: %3 }")
        .arg(int(this->mode)).arg(this->requestedThreads)
        .arg(this->deterministicReduction ? "True" : "False");
}
