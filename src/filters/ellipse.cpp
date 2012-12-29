#include "ellipse.h"
#include "ellipsekernel.h"

#include "tfr/cwt.h"
#include "tfr/chunk.h"

// gpumisc
#include <TaskTimer.h>
#include <computationkernel.h>

// std
#include <iomanip>

//#define TIME_FILTER
#define TIME_FILTER if(0)

using namespace Tfr;

namespace Filters {

Ellipse::Ellipse(float t1, float f1, float t2, float f2, bool save_inside)
{
    _centre_t = t1;
    _centre_f = f1;
    _centre_plus_radius_t = t2;
    _centre_plus_radius_f = f2;
    _save_inside = save_inside;
}


std::string Ellipse::
        name()
{
    std::stringstream ss;
    ss << std::setiosflags(std::ios::fixed)
       << std::setprecision(1)
       << "Ellipse at " <<  _centre_t << " s, "
       << std::setprecision(0) << _centre_f << " Hz, size " << std::log(std::fabs((_centre_plus_radius_t-_centre_t)*(_centre_plus_radius_f-_centre_f)*M_PI));

    if (!this->_save_inside)
        ss << ", save outside";

    return ss.str();
}


bool Ellipse::
        operator()( Chunk& chunk )
{
    TIME_FILTER TaskTimer tt("Ellipse");

    Area area = {
            _centre_t * chunk.sample_rate - chunk.chunk_offset.asFloat(),
            chunk.freqAxis.getFrequencyScalarNotClamped( _centre_f ),
            _centre_plus_radius_t * chunk.sample_rate - chunk.chunk_offset.asFloat(),
            chunk.freqAxis.getFrequencyScalarNotClamped( _centre_plus_radius_f )};

    ::removeDisc( chunk.transform_data,
                  area, _save_inside, chunk.sample_rate );

    TIME_FILTER ComputationSynchronize();

    return true;
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

    float r = fabsf(_centre_t - _centre_plus_radius_t);
    r += ((Tfr::Cwt*)transform().get())->wavelet_time_support_samples(FS)/FS;

    long double
        start_time_d = std::max(0.f, _centre_t - r)*FS,
        end_time_d = std::max(0.f, _centre_t + r)*FS;

    Signal::IntervalType
        start_time = std::min((long double)Signal::Interval::IntervalType_MAX, start_time_d),
        end_time = std::min((long double)Signal::Interval::IntervalType_MAX, end_time_d);

    Signal::Intervals sid;
    if (start_time < end_time)
        sid = Signal::Intervals(start_time, end_time);

    return ~sid;
}

} // namespace Filters
