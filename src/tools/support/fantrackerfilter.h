#ifndef FANTRACKERFILTER_H
#define FANTRACKERFILTER_H

#include "tfr/cepstrumfilter.h"

#include <vector>

namespace Tools {
namespace Support {

class FanTrackerFilter : public Tfr::CepstrumFilter, public Tfr::ChunkFilter::NoInverseTag
{
public:
    FanTrackerFilter();

    virtual void operator()( Tfr::Chunk& );

    virtual Signal::Intervals affected_samples();

    virtual Signal::DeprecatedOperation* affecting_source( const Signal::Interval& I );
    virtual void source(Signal::pOperation v);
    virtual void invalidate_samples(const Signal::Intervals& I);

    struct Point
    {
        float Hz;
        float amplitude;
    };

    typedef std::map<unsigned, Point> PointsT;
    std::vector<PointsT> track;

};

} // namespace Support
} // namespace Tools

#endif // FANTRACKERFILTER_H
