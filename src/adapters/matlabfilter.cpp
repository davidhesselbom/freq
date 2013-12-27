#include "matlabfilter.h"
#include "hdf5.h"

#include "tfr/chunk.h"

#include <signal.h>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace Signal;
using namespace Tfr;

//#define TIME_MatlabFilter
#define TIME_MatlabFilter if(0)

namespace Adapters {

MatlabFilter::
        MatlabFilter( std::string matlabFunction )
:   _matlab(new MatlabFunction(matlabFunction, 15, 0))
{
}


void MatlabFilter::
        operator()( Chunk& c)
{
    TIME_MatlabFilter TaskTimer tt("MatlabFilter::operator() [%g,%g)", c.startTime(), c.endTime() );

    _invalid_returns |= c.getInterval();

    std::string file = _matlab->isReady();
    if (!file.empty())
    {
        Tfr::pChunk pc = Hdf5Chunk::loadChunk( file );
        c.transform_data.swap( pc->transform_data );

        ::remove( file.c_str());

        Interval J = c.getInterval();

        DeprecatedOperation::invalidate_samples( _invalid_returns & J );
        _invalid_returns -= J;

        // TODO Perform inverse
        //return true;
        return;
    }

    if (!_matlab->isWaiting())
    {
        string file = _matlab->getTempName();

        Hdf5Chunk::saveChunk( file, c );

        _matlab->invoke( file );
    }

    // TODO Don't perform inverse. Tfr::ChunkFilter::NoInverseTag
    // Suggested: In general, compute an inverse but in this case compute a dummy inverse and return that instead.
    //return false;
}


Signal::Intervals MatlabFilter::
        ZeroedSamples( ) const
{
    // As far as we know, the matlab filter doesn't set anything to zero for sure
    return Signal::Intervals();
}

Signal::Intervals MatlabFilter::
        affected_samples( )
{
    // As far as we know, the matlab filter may touch anything
    return Signal::Intervals();
}


void MatlabFilter::
        restart()
{
    std::string fn = _matlab->matlabFunction();
    float t = _matlab->timeout();

    _matlab.reset();
    _matlab.reset( new MatlabFunction( fn, t, 0 ));
}


} // namespace Adapters
