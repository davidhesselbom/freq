#include "heightmapprocessingpublisher.h"
#include "heightmap/collection.h"
#include "signal/processing/step.h"

#include "TaskTimer.h"

//#define TIME_PAINTGL_DETAILS
#define TIME_PAINTGL_DETAILS if(0)

using namespace Signal;
using namespace Processing;

namespace Tools {
namespace Support {

HeightmapProcessingPublisher::
        HeightmapProcessingPublisher(TargetNeeds::Ptr target_needs,
                      Heightmap::TfrMapping::Collections collections)
    :
      target_needs_(target_needs),
      collections_(collections),
      failed_allocation_(false)
{
}


void HeightmapProcessingPublisher::
        update(float t_center, OperationDesc::Extent x, UnsignedIntervalType preferred_update_size)
{
    TIME_PAINTGL_DETAILS TaskTimer tt("Find things to work on");

    //Intervals invalid_samples;
    Intervals things_to_add;
    Intervals needed_samples;
    IntervalType center = t_center * x.sample_rate.get ();

    // It should update the view in sections equal in size to the smallest
    // visible block if the view isn't currently being invalidated.
    UnsignedIntervalType update_size = preferred_update_size;

    foreach( const Heightmap::Collection::Ptr &c, collections_ ) {
        Heightmap::Collection::WritePtr wc(c);
        //invalid_samples |= wc->invalid_samples();
        things_to_add |= wc->recently_created();
        needed_samples |= wc->needed_samples(update_size);
    }

    if (needed_samples & x.interval.get ())
        needed_samples &= x.interval.get ();
    else
        needed_samples = needed_samples.fetchInterval (1, center);

    TIME_PAINTGL_DETAILS TaskInfo(boost::format(
            "RenderView needed_samples = %s, "
            "things_to_add = %s, center = %d, size = %d")
            % needed_samples
            % things_to_add
            % center
            % update_size);

    write1(target_needs_)->deprecateCache(things_to_add);
    write1(target_needs_)->updateNeeds(
                needed_samples,
                center,
                update_size,
                0
            );

    failed_allocation_ = false;
    foreach( const Heightmap::Collection::Ptr &c, collections_ )
    {
        failed_allocation_ |= write1(c)->failed_allocation ();
    }

    TIME_PAINTGL_DETAILS {
        Step::Ptr step = read1(target_needs_)->step().lock();
        Signal::Intervals not_started = read1(target_needs_)->not_started();

        if (step)
        {
            Step::ReadPtr stepp(step);
            TaskInfo(boost::format("RenderView step->out_of_date = %s, step->not_started = %s, target_needs->not_started = %s")
                             % stepp->out_of_date()
                             % stepp->not_started()
                             % not_started);
        }
    }
}


bool HeightmapProcessingPublisher::
        isHeightmapDone() const
{
    TargetNeeds::ReadPtr target_needs(target_needs_);
    return !target_needs->out_of_date();
}


bool HeightmapProcessingPublisher::
        failedAllocation() const
{
    return failed_allocation_;
}

} // namespace Support
} // namespace Tools

#include "signal/processing/bedroom.h"
#include "signal/processing/task.h"
#include "signal/processing/bedroomnotifier.h"
#include <QApplication>
#include <QGLWidget>

namespace Tools {
namespace Support {

void HeightmapProcessingPublisher::
        test()
{
    std::string name = "HeightmapProcessingPublisher";
    int argc = 1;
    char * argv = &name[0];
    QApplication a(argc,&argv);
    QGLWidget w;
    w.makeCurrent ();

    // It should update a processing target depending on which things that are
    // missing in a heightmap block cache
    {
        OperationDesc::Ptr operation_desc;
        Step::Ptr step(new Step(operation_desc));
        Bedroom::Ptr bedroom(new Bedroom);
        BedroomNotifier::Ptr notifier(new BedroomNotifier(bedroom));
        TargetNeeds::Ptr target_needs(new TargetNeeds(step, notifier));

        Heightmap::BlockLayout block_layout(10,10,1);
        Heightmap::VisualizationParams::Ptr visualization_params(new Heightmap::VisualizationParams);
        Heightmap::Collection::Ptr collection(new Heightmap::Collection(
                                                   block_layout, visualization_params));

        Heightmap::TfrMapping::Collections collections;
        collections.push_back (collection);

        HeightmapProcessingPublisher hpp(target_needs, collections);

        float t_center = 10;
        OperationDesc::Extent x;
        x.interval = Interval(-10,20);
        x.number_of_channels = 1;
        x.sample_rate = 1;
        UnsignedIntervalType preferred_update_size = 5;

        EXCEPTION_ASSERT(hpp.isHeightmapDone ());

        hpp.update(t_center, x, preferred_update_size);

        EXCEPTION_ASSERT(hpp.isHeightmapDone ());

        Heightmap::Reference entireHeightmap = read1(collection)->entireHeightmap();
        write1(collection)->getBlock(entireHeightmap);

        EXCEPTION_ASSERT(hpp.isHeightmapDone ());

        hpp.update(t_center, x, preferred_update_size);

        EXCEPTION_ASSERT(hpp.isHeightmapDone ());

        unsigned frame_number = read1(collection)->frame_number();
        write1(collection)->getBlock(entireHeightmap)->frame_number_last_used = frame_number;

        EXCEPTION_ASSERT(hpp.isHeightmapDone ());

        hpp.update(t_center, x, preferred_update_size);

        EXCEPTION_ASSERT(!hpp.isHeightmapDone ());

        Task task(write1(step),
                  std::vector<Signal::Processing::Step::Ptr>(),
                  Operation::Ptr(),
                  Signal::Interval(0,2), Signal::Interval());

        EXCEPTION_ASSERT(!hpp.isHeightmapDone ());
    }
}

} // namespace Support
} // namespace Tools