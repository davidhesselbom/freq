#include "fftimplementation.h"
#include "neat_math.h"

namespace Tfr {

#include "fftooura.h"
#include "clfft/fftclfft.h"
#include "clamdfft/fftclamdfft.h"
#include "fftcufft.h"

#include <boost/make_shared.hpp>

using namespace boost;

#if defined(USE_CUDA) && !defined(USE_CUFFT)
#define USE_CUFFT
#endif

namespace Tfr {

shared_ptr<FftImplementation> FftImplementation::
        newInstance()
{
#ifdef USE_CUFFT
    return make_shared<FftCufft>(); // Gpu, Cuda
#elif defined(USE_AMDFFT)
    return make_shared<FftClAmdFft>(); // Gpu, OpenCL
#elif defined(USE_OPENCL)
    return make_shared<FftClFft>(); // Gpu, OpenCL
#else
    return make_shared<FftOoura>(); // Cpu
#endif
}


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
