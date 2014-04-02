#ifndef SIGNAL_PROCESSING_TARGETS_H
#define SIGNAL_PROCESSING_TARGETS_H

#include "step.h"
#include "targetneeds.h"
#include "inotifier.h"
#include "dag.h"

namespace Signal {
namespace Processing {

/**
 * @brief The Targets class should keep track of targets and let callers update
 * what each target needs afterwards.
 *
 * Multiple targets can be added for the same Step.
 */
class Targets
{
public:
    typedef shared_state<Targets> ptr;

    typedef std::vector<TargetNeeds::ptr> TargetNeedsCollection;

    Targets(INotifier::weak_ptr notifier);

    /**
     * @brief insert adds a new target to this collection of Targets.
     *
     * @param step A step for which to create a TargetNeed.
     *
     * @return A TargetNeeds instance to update what schedulers should focus
     * on in order to keep the target up-to-date. Note that the scheduler will
     * not create any task at all for this Target until you have specified
     * needed_samples through TargetNeeds::updateNeeds.
     *
     * Ownership is of TargetNeeds is given to the caller.
     */
    TargetNeeds::ptr              addTarget(Step::ptr::weak_ptr step);
    TargetNeedsCollection         getTargets() const;

private:
    INotifier::weak_ptr notifier_;

    std::vector<TargetNeeds::ptr::weak_ptr> targets;

public:
    static void test();
};

} // namespace Processing
} // namespace Signal

#endif // SIGNAL_PROCESSING_TARGETS_H
