#include "fftimplementation.h"
#include "neat_math.h"

#include "fftooura.h"
#if defined(USE_OPENCL)
#if defined(USE_AMD)
#include "clamdfft/fftclamdfft.h"
#else
#include "clfft/fftclfft.h"
#endif
#endif
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
#elif defined(USE_OPENCL)
	#if defined(USE_AMD)
    return make_shared<FftClAmdFft>(); // Gpu, OpenCL
	#else
    return make_shared<FftClFft>(); // Gpu, OpenCL
	#endif
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
