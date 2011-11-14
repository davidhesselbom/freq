#include "ellipse.h"
#include "ellipsekernel.h"

// gpumisc
#include <TaskTimer.h>
#include <computationkernel.h>


//#define TIME_FILTER
#define TIME_FILTER if(0)

using namespace Tfr;

namespace Filters {

Ellipse::Ellipse(float t1, float f1, float t2, float f2, bool save_inside)
{
    _t1 = t1;
    _f1 = f1;
    _t2 = t2;
    _f2 = f2;
    _save_inside = save_inside;
}


std::string Ellipse::
        name()
{
    std::stringstream ss;
    ss << std::setiosflags(std::ios::fixed)
       << std::setprecision(1)
       << "Ellipse at " <<  _t1 << " s, "
       << std::setprecision(0) << _f1 << " Hz, size " << std::log(std::fabs((_t2-_t1)*(_f2-_f1)*M_PI));

    if (!this->_save_inside)
        ss << ", save outside";

    return ss.str();
}


void Ellipse::
        operator()( Chunk& chunk )
{
    TIME_FILTER TaskTimer tt("Ellipse");

    Area area = {
            _t1 * chunk.sample_rate - chunk.chunk_offset.asFloat(),
            chunk.freqAxis.getFrequencyScalarNotClamped( _f1 ),
            _t2 * chunk.sample_rate - chunk.chunk_offset.asFloat(),
            chunk.freqAxis.getFrequencyScalarNotClamped( _f2 )};

    ::removeDisc( chunk.transform_data,
                  area, _save_inside, chunk.sample_rate );

    TIME_FILTER ComputationSynchronize();
}


Signal::Intervals Ellipse::
        zeroed_samples()
{
    return _save_inside ? outside_samples() : Signal::Intervals();
}


Signal::Intervals Ellipse::
        affected_samples()
{
    return (_save_inside ? Signal::Intervals() : outside_samples()).inverse();
}


Signal::Intervals Ellipse::
        outside_samples()
{
    long double FS = sample_rate();

    float r = fabsf(_t1 - _t2);
    r += 0.06f;

    long double
        start_time_d = std::max(0.f, (std::min(_t1,_t2) - r))*FS,
        end_time_d = std::max(0.f, (std::max(_t1,_t2) + r))*FS;

    Signal::IntervalType
        start_time = std::min((long double)Signal::Interval::IntervalType_MAX, start_time_d),
        end_time = std::min((long double)Signal::Interval::IntervalType_MAX, end_time_d);

    Signal::Intervals sid;
    if (start_time < end_time)
        sid = Signal::Intervals(start_time, end_time);

    return ~sid;
}

} // namespace Filters
