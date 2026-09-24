#ifndef RML_PARALLEL_EXECUTION_CONFIG_H
#define RML_PARALLEL_EXECUTION_CONFIG_H

#include <QString>

//! Execution policy. FixedThreads uses requestedThreads; the other modes do not.
enum class RParallelExecutionMode
{
    Auto = 0,
    SerialDeterministic = 1,
    FixedThreads = 2
};

class RParallelExecutionConfig
{
    public:
        RParallelExecutionConfig() = default;

        RParallelExecutionMode getMode() const;
        void setMode(RParallelExecutionMode mode);

        uint getRequestedThreads() const;
        void setRequestedThreads(uint requestedThreads);

        bool getDeterministicReduction() const;
        void setDeterministicReduction(bool deterministicReduction);

        //! Check combinations, including the thread count required by FixedThreads.
        void validate() const;
        QString toString() const;

    private:
        RParallelExecutionMode mode = RParallelExecutionMode::Auto;
        uint requestedThreads = 0;
        bool deterministicReduction = true;
};

#endif // RML_PARALLEL_EXECUTION_CONFIG_H
