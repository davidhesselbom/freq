#ifndef SIGNAL_PROCESSING_ISCHEDULEALGORITHM_H
#define SIGNAL_PROCESSING_ISCHEDULEALGORITHM_H

#include "shared_state.h"
#include "task.h"
#include "dag.h"
#include "workers.h"

namespace Signal {
namespace Processing {

class IScheduleAlgorithm
{
public:
    typedef shared_state<IScheduleAlgorithm> ptr;

    virtual ~IScheduleAlgorithm() {}

    virtual Task getTask(
            const Graph& g,
            GraphVertex target,
            Signal::Intervals needed, //=Intervals::Intervals_ALL,
            Signal::IntervalType center, //=Interval::IntervalType_MIN,
            Signal::IntervalType preferred_size, //=Interval::IntervalType_MAX,
            Workers::ptr workers, //=Workers::Ptr(),
            Signal::ComputingEngine::ptr worker) const = 0;
};

} // namespace Processing
} // namespace Signal

#endif // SIGNAL_PROCESSING_ISCHEDULEALGORITHM_H
