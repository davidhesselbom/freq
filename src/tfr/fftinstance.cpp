#include "fftimplementation.h"

#include "fftooura.h"
#include "clfft/fftclfft.h"
#include "clamdfft/fftclamdfft.h"
#include "fftcufft.h"

#if defined(USE_CUDA) && !defined(USE_CUFFT)
#define USE_CUFFT
#endif

namespace Tfr {

    FftImplementation& FftImplementation::
            Singleton()
    {

    #ifdef USE_CUFFT
        static FftCufft fft;
    #elif defined(USE_AMDFFT)
        static FftClAmdFft fft;
    #elif defined(USE_OPENCL)
        static FftClFft fft;
    #else
        static FftOoura fft;
    #endif
        return fft;
    }

} //namespace Tfr
