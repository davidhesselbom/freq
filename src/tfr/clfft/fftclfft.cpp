#ifdef USE_OPENCL

#include <stdexcept>
#include "tfr/stft.h"
#include "tfr/stftkernel.h"
#include "openclcontext.h"

#include "cpumemorystorage.h"
#include "openclmemorystorage.h"
#include "TaskTimer.h"
#include "computationkernel.h"
#include "clfftkernelbuffer.h"

#include "clFFT.h"

#include "fftclfft.h"

//#define TIME_STFT
#define TIME_STFT if(0)


namespace Tfr {

void FftClFft::createPlan(unsigned n)
{
	OpenCLContext *opencl = &OpenCLContext::Singleton();
    cl_int fft_error;

    clFFT_Plan plan = CLFFTKernelBuffer::Singleton().getPlan(opencl->getContext(), n, fft_error);
    if (fft_error != CL_SUCCESS)
        throw std::runtime_error("Could not create clFFT compute plan.");
}

void FftClFft::
        compute( Tfr::ChunkData::Ptr input, Tfr::ChunkData::Ptr output, FftDirection direction )
{
    TIME_STFT TaskTimer tt("Fft ClFft");

    unsigned n = input->numberOfElements();
    unsigned N = output->numberOfElements();

    if (-1 != direction)
        EXCEPTION_ASSERT( n == N );

    {
        TIME_STFT TaskTimer tt("Computing fft(N=%u, n=%u, direction=%d)", N, n, direction);
        OpenCLContext *opencl = &OpenCLContext::Singleton();
        cl_int fft_error;

        clFFT_Plan plan = CLFFTKernelBuffer::Singleton().getPlan(opencl->getContext(), n, fft_error);
        if (fft_error != CL_SUCCESS)
            throw std::runtime_error("Could not create clFFT compute plan.");

		cl_mem clMemBuffersIn [ 1 ] = { OpenClMemoryStorage::ReadWrite<1>( input ).ptr() };
		cl_mem clMemBuffersOut [ 1 ] = { OpenClMemoryStorage::ReadWrite<1>( output ).ptr() };

        // Run the fft in OpenCL :)
        // fft kernel needs to have read/write access to output data
		{
	        TIME_STFT TaskTimer tt("Executing fft(N=%u)", N);

			cl_event outEvent = NULL;
			cl_ulong startTime, endTime;

	        fft_error |= clFFT_ExecuteInterleaved(
                opencl->getCommandQueue(),
                plan, 1, (clFFT_Direction)direction,
				clMemBuffersIn[0],
				clMemBuffersOut[0],
				0, NULL, &outEvent );

			clFinish(opencl->getCommandQueue());
			
			clGetEventProfilingInfo(outEvent, CL_PROFILING_COMMAND_START, 
				 sizeof(cl_ulong), &startTime, NULL);

			clGetEventProfilingInfo(outEvent,  CL_PROFILING_COMMAND_END,
				 sizeof(cl_ulong), &endTime, NULL);

            kernelExecTime = endTime-startTime;
		}

        if (fft_error != CL_SUCCESS)
            throw std::runtime_error("Bad stuff happened during FFT computation.");
    }
}


void FftClFft::
        computeR2C( DataStorage<float>::Ptr input, Tfr::ChunkData::Ptr output )
{
    unsigned denseWidth = output->size().width;
    unsigned redundantWidth = input->size().width;

   EXCEPTION_ASSERT( denseWidth == redundantWidth/2+1 );

    // interleave input to complex data
   Tfr::ChunkData::Ptr complexinput( new Tfr::ChunkData( input->size()));
   ::stftToComplex( input, complexinput );

    // make room for full output
    Tfr::ChunkData::Ptr redundantOutput( new Tfr::ChunkData( redundantWidth ));

    // compute
    compute(complexinput, redundantOutput, FftDirection_Forward);

    // discard redundant output
    {
        Tfr::ChunkElement* in = CpuMemoryStorage::ReadOnly<1>( redundantOutput ).ptr();
        Tfr::ChunkElement* out = CpuMemoryStorage::WriteAll<1>( output ).ptr();
        unsigned x;
        for (x=0; x<denseWidth; ++x)
            out[x] = in[x];
    }
}


void FftClFft::
        computeC2R( Tfr::ChunkData::Ptr input, DataStorage<float>::Ptr output )
{
    unsigned denseWidth = input->size().width;
    unsigned redundantWidth = output->size().width;

    EXCEPTION_ASSERT( denseWidth == redundantWidth/2+1 );

    Tfr::ChunkData::Ptr redundantInput( new Tfr::ChunkData( redundantWidth, input->size().height ));

    {
        Tfr::ChunkElement* in = CpuMemoryStorage::ReadOnly<1>( input ).ptr();
        Tfr::ChunkElement* out = CpuMemoryStorage::WriteAll<1>( redundantInput ).ptr();
        unsigned x;
        for (x=0; x<denseWidth; ++x)
            out[x] = in[x];
        for (; x<redundantWidth; ++x)
            out[x] = conj(in[redundantWidth - x]);
    }

    Tfr::ChunkData::Ptr complexoutput( new Tfr::ChunkData( output->size()));

    compute(redundantInput, complexoutput, FftDirection_Inverse);

    ::stftDiscardImag( complexoutput, output );
}


void FftClFft::
        compute( Tfr::ChunkData::Ptr input, Tfr::ChunkData::Ptr output, DataStorageSize n, FftDirection direction )
{
    TaskTimer tt("Stft::computeWithClFft( matrix[%d, %d], %s )",
                 input->size().width,
                 input->size().height,
                 direction==FftDirection_Forward?"forward":"backward");

    EXCEPTION_ASSERT( output->numberOfBytes() == input->numberOfBytes() );

    const int batchSize = n.height;

    OpenCLContext *opencl = &OpenCLContext::Singleton();
    cl_int fft_error;

    clFFT_Plan plan = CLFFTKernelBuffer::Singleton().getPlan(opencl->getContext(), n.width, fft_error);
    if(fft_error != CL_SUCCESS)
        throw std::runtime_error("Could not create clFFT compute plan.");

    {
        TaskTimer tt("Calculating batches");

        // Run the fft in OpenCL :)
        fft_error |= clFFT_ExecuteInterleaved(
                opencl->getCommandQueue(),
                plan, batchSize, direction==FftDirection_Forward?clFFT_Forward:clFFT_Inverse,
                OpenClMemoryStorage::ReadOnly<1>( input ).ptr(),
                OpenClMemoryStorage::ReadWrite<1>( output ).ptr(),
                0, NULL, NULL );
        if(fft_error != CL_SUCCESS)
            throw std::runtime_error("Bad stuff happened during FFT computation.");
    }
}


void FftClFft::
        compute(DataStorage<float>::Ptr input, Tfr::ChunkData::Ptr output, DataStorageSize n )
{
    unsigned denseWidth = n.width/2+1;

    EXCEPTION_ASSERT( output->numberOfElements()/denseWidth == n.height );
    EXCEPTION_ASSERT( input->numberOfElements()/n.width == n.height );

    // interleave input to complex data
    Tfr::ChunkData::Ptr complexinput( new Tfr::ChunkData( input->size()));
    ::stftToComplex( input, complexinput );

    // make room for full output
    Tfr::ChunkData::Ptr redundantOutput( new Tfr::ChunkData( n.width*n.height ));

    // compute
    compute(complexinput, redundantOutput, n, FftDirection_Forward);

    // discard redundant output
    Tfr::ChunkElement* in = CpuMemoryStorage::ReadOnly<1>( redundantOutput ).ptr();
    Tfr::ChunkElement* out = CpuMemoryStorage::WriteAll<1>( output ).ptr();
#pragma omp parallel for
    for (int i=0; i < (int)n.height; ++i)
    {
        unsigned x;
        for (x=0; x<denseWidth; ++x)
            out[i*denseWidth + x] = in[i*n.width + x];
    }
}


void FftClFft::
        inverse(Tfr::ChunkData::Ptr input, DataStorage<float>::Ptr output, DataStorageSize n )
{
    unsigned denseWidth = n.width/2+1;
    unsigned redundantWidth = n.width;
    unsigned batchcount1 = output->numberOfElements()/redundantWidth,
             batchcount2 = input->numberOfElements()/denseWidth;

    EXCEPTION_ASSERT( batchcount1 == batchcount2 );
    EXCEPTION_ASSERT( (denseWidth-1)*2 == redundantWidth );
    EXCEPTION_ASSERT( redundantWidth*n.height == output->numberOfElements() );

    Tfr::ChunkData::Ptr redundantInput( new Tfr::ChunkData( n.height*redundantWidth ));

    {
        Tfr::ChunkElement* in = CpuMemoryStorage::ReadOnly<1>( input ).ptr();
        Tfr::ChunkElement* out = CpuMemoryStorage::WriteAll<1>( redundantInput ).ptr();
#pragma omp parallel for
        for (int i=0; i < (int)n.height; ++i)
        {
            unsigned x;
            for (x=0; x<denseWidth; ++x)
                out[i*redundantWidth + x] = in[i*denseWidth + x];
            for (; x<redundantWidth; ++x)
                out[i*redundantWidth + x] = conj(in[i*denseWidth + redundantWidth - x]);
        }
    }

    Tfr::ChunkData::Ptr complexoutput( new Tfr::ChunkData( output->size()));

    compute(redundantInput, complexoutput, DataStorageSize( redundantWidth, n.height), FftDirection_Inverse);

    ::stftDiscardImag( complexoutput, output );

    TIME_STFT ComputationSynchronize();
}


} // namespace Tfr
#endif // #ifdef USE_OPENCL