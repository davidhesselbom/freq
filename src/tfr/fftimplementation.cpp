#include "fftimplementation.h"
#include "neat_math.h"

namespace Tfr {

unsigned FftImplementation::
        lChunkSizeS(unsigned x, unsigned)
{
    return lpo2s(x);
}


unsigned FftImplementation::
        sChunkSizeG(unsigned x, unsigned)
{
    return spo2g(x);
}

} // namespace Tfr
