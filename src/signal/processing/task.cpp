#include "task.h"

#include "TaskTimer.h"
#include "demangle.h"
#include "expectexception.h"
#include "log.h"

#include <boost/foreach.hpp>

//#define TIME_TASK
#define TIME_TASK if(0)

namespace Signal {
namespace Processing {

Task::
        Task(const Step::WritePtr& step,
             std::vector<Step::Ptr> children,
             Signal::Operation::Ptr operation,
             Signal::Interval expected_output,
             Signal::Interval required_input)
    :
      step_(step.getPtr()),
      children_(children),
      operation_(operation),
      expected_output_(expected_output),
      required_input_(required_input)
{
    step->registerTask (this, expected_output);
}


Task::
        ~Task()
{
    cancel();
}


Signal::Interval Task::
        expected_output() const
{
    return expected_output_;
}


void Task::
        run()
{
    try
      {
        run_private();
      }
    catch (const boost::exception& x)
      {
        // Append info to be used at the catch site
        x   << Step::crashed_step(step_)
            << Task::crashed_expected_output(expected_output_);

        try {
            write1(step_)->mark_as_crashed();
        } catch(const std::exception& y) {
            x << unexpected_exception_info(boost::current_exception());
        }

        throw;
      }
}


void Task::
        run_private()
{
    Signal::OperationDesc::Ptr od;
    TIME_TASK od = read1(step_)->operation_desc ();
    TIME_TASK TaskTimer tt(boost::format("Task::run %1%")
                           % read1(od)->toString ().toStdString ());

    Signal::Operation::Ptr o = this->operation_;

    Signal::pBuffer input_buffer, output_buffer;

    {
        TIME_TASK TaskTimer tt(boost::format("expect  %s")
                               % expected_output());
        input_buffer = get_input();
    }

    {
        TIME_TASK TaskTimer tt(boost::format("process %s") % input_buffer->getInterval ());
        output_buffer = write1(o)->process (input_buffer);
        finish(output_buffer);
    }
}


Signal::pBuffer Task::
        get_input() const
{
    Signal::OperationDesc::Ptr operation_desc = read1(step_)->operation_desc ();

    // Sum all sources
    std::vector<Signal::pBuffer> buffers;
    buffers.reserve (children_.size ());

    Signal::OperationDesc::Extent x = read1(operation_desc)->extent ();

    unsigned num_channels = x.number_of_channels.get_value_or (0);
    float sample_rate = x.sample_rate.get_value_or (0.f);
    for (size_t i=0;i<children_.size(); ++i)
    {
        Signal::pBuffer b = read1(children_[i])->readFixedLengthFromCache(required_input_);
        if (b) {
            num_channels = std::max(num_channels, b->number_of_channels ());
            sample_rate = std::max(sample_rate, b->sample_rate ());
            buffers.push_back ( b );
        }
    }

    if (0==num_channels || 0.f==sample_rate) {
        // Undefined signal. Shouldn't have created this task.
        Log("required_input = %s") % required_input_;
        if (children_.empty ()) {
            EXCEPTION_ASSERT(x.sample_rate.is_initialized ());
            EXCEPTION_ASSERT(x.number_of_channels.is_initialized ());
        } else {
            EXCEPTION_ASSERT_LESS(0u, buffers.size ());
        }
        EXCEPTION_ASSERT_LESS(0u, num_channels);
        EXCEPTION_ASSERT_LESS(0u, sample_rate);
    }

    Signal::pBuffer input_buffer(new Signal::Buffer(required_input_, sample_rate, num_channels));

    BOOST_FOREACH( Signal::pBuffer b, buffers )
    {
        for (unsigned c=0; c<num_channels && c<b->number_of_channels (); ++c)
            *input_buffer->getChannel (c) += *b->getChannel(c);
    }

    return input_buffer;
}


void Task::
        finish(Signal::pBuffer b)
{
    if (step_)
        write1(step_)->finishTask(this, b);
    step_.reset();
}


void Task::
        cancel()
{
    if (step_)
        write1(step_)->finishTask(this, Signal::pBuffer());
    step_.reset();
}

} // namespace Processing
} // namespace Signal

#include "test/randombuffer.h"

namespace Signal {
namespace Processing {

void Task::
        test()
{
    // It should store results of an operation in the cache
    {
        // setup a known signal processing operation (take data from a predefined buffer)
        pBuffer b = Test::RandomBuffer::randomBuffer (Interval(60,70), 40, 7);
        Signal::OperationDesc::Ptr od(new BufferSource(b));

        // setup a known signal processing step
        Step::Ptr step (new Step(od));
        std::vector<Step::Ptr> children; // empty
        Signal::Interval expected_output(-10,80);
        Signal::Interval required_input;
        Signal::Operation::Ptr o;
        {
            Signal::ComputingEngine::Ptr c(new Signal::ComputingCpu);
            Signal::OperationDesc::ReadPtr r(od);
            required_input = r->requiredInterval(expected_output, 0);
            o = r->createOperation (c.get ());
        }

        // perform a signal processing task
        Task t(write1(step), children, o, expected_output, required_input);
        t.run ();

        Signal::Interval to_read = Signal::Intervals(expected_output).enlarge (2).spannedInterval ();
        Signal::pBuffer r = write1(step)->readFixedLengthFromCache(to_read);
        EXCEPTION_ASSERT_EQUALS(b->sample_rate (), r->sample_rate ());
        EXCEPTION_ASSERT_EQUALS(b->number_of_channels (), r->number_of_channels ());

        Signal::Buffer expected_r(to_read, 40, 7);
        expected_r += *b;

        EXCEPTION_ASSERT(expected_r == *r);
    }
}


} // namespace Processing
} // namespace Signal