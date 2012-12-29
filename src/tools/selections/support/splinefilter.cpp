#include "splinefilter.h"
#include "splinefilterkernel.h"
#include "cpumemorystorage.h"
#include "tfr/chunk.h"

// gpumisc
#include <computationkernel.h>

// boost
#include <boost/foreach.hpp>

using namespace Tfr;

#define TIME_SPLINEFILTER
//#define TIME_SPLINEFILTER if(0)

//#define DEBUG_SPLINEFILTER
#define DEBUG_SPLINEFILTER if(0)

namespace Tools { namespace Selections { namespace Support {

SplineFilter::SplineFilter(bool save_inside) {
    _save_inside = save_inside;
}


std::string SplineFilter::
        name()
{
    std::stringstream ss;
    ss << "Polygon with " << v.size() << " vertices";
    if (!this->_save_inside)
        ss << ", saving outside";
    return ss.str();
}


bool SplineFilter::operator()( Chunk& chunk)
{
    TIME_SPLINEFILTER TaskTimer tt("SplineFilter chunk area (%g %g : %g %g)",
        chunk.startTime(), chunk.minHz(), chunk.endTime(), chunk.maxHz());

    unsigned N = v.size();

    std::vector<ChunkElement> p(N);

	unsigned j=0;
    float t1 = (chunk.chunk_offset/chunk.sample_rate).asFloat(),
          t2 = ((chunk.chunk_offset + chunk.nSamples())/chunk.sample_rate).asFloat();

    for (unsigned i=0; i<N; ++i)
    {
        unsigned pi = (i+N-1)%N;
        unsigned ni = (i+1)%N;
        if ((v[i].t < t1 && v[ni].t < t1 && v[pi].t < t1) ||
            (v[i].t > t2 && v[ni].t > t2 && v[pi].t > t2))
		{
			continue;
		}

        p[j] = ChunkElement(
				v[i].t * chunk.sample_rate - chunk.chunk_offset.asFloat(),
                chunk.freqAxis.getFrequencyScalarNotClamped( v[i].f ));

        DEBUG_SPLINEFILTER TaskTimer("(%g %g) -> p[%u] = (%g %g)",
                  v[i].t, v[i].f, i, p[i].real(), p[i].imag()).suppressTiming();

		j++;
    }

    if (0<j)
    {
        TIME_SPLINEFILTER TaskTimer tt("SplineFilter applyspline (using subset with %u points out of %u total points)", j, N);

        DataStorage<ChunkElement>::Ptr pts = CpuMemoryStorage::BorrowPtr( DataStorageSize(j), &p[0] );

        ::applyspline(
                chunk.transform_data,
                pts, _save_inside,
                chunk.sample_rate );
    }

    TIME_SPLINEFILTER ComputationSynchronize();

    return 0<j;
}


Signal::Intervals SplineFilter::
        zeroed_samples()
{
    return _save_inside ? outside_samples() : Signal::Intervals();
}


Signal::Intervals SplineFilter::
        affected_samples()
{
    return (_save_inside ? Signal::Intervals() : outside_samples()).inverse();
}


Signal::Intervals SplineFilter::
        outside_samples()
{
    if (v.size() < 2)
        return Signal::Intervals::Intervals_ALL;

    float
        start_time = v.front().t,
        end_time = v.front().t;

    BOOST_FOREACH( SplineVertex const& p, v )
    {
        start_time = std::min(start_time, p.t);
        end_time = std::max(end_time, p.t);
    }

    double FS = sample_rate();
    Signal::Intervals sid;
    Signal::Interval sidint(std::max(0.f, start_time)*FS, std::max(0.f, end_time)*FS);
    if (sidint.count())
        sid = sidint;
    sid = sid.enlarge( sample_rate()*0.1f );

    return ~sid;
}

}}} // namespace Tools::Selections::Support
