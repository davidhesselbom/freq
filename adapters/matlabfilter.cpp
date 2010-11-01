#include "matlabfilter.h"
#include "hdf5.h"

#include <signal.h>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace Signal;
using namespace Tfr;

namespace Adapters {

MatlabFilter::
        MatlabFilter( std::string matlabFunction )
:   _matlab(new MatlabFunction(matlabFunction, 15))
{
}

void MatlabFilter::
        operator()( Chunk& c)
{
    TaskTimer tt("MatlabFilter::operator()");
//    TaskTimer tt("MatlabFilter::operator() (%f,%f)", c.startTime(), c.endTime() );

    string file = _matlab->getTempName();

    Hdf5Chunk::saveChunk( file, c );

    file = _matlab->invokeAndWait( file );

	if (file.empty())
		return;

    Tfr::pChunk pc = Hdf5Chunk::loadChunk( file );
    c.transform_data.swap( pc->transform_data );

    ::remove( file.c_str());
}

Signal::Intervals MatlabFilter::
        ZeroedSamples( ) const
{
    // As far as we know, the matlab filter doesn't set anything to zero for sure
    return Signal::Intervals();
}

Signal::Intervals MatlabFilter::
        affected_samples( ) const
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
    _matlab.reset( new MatlabFunction( fn, t ));
}


} // namespace Adapters
