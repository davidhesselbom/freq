#ifdef USE_OPENCL

#include <stdexcept>
#include "tfr/stft.h"
#include "tfr/stftkernel.h"
#include "tfr/complexbuffer.h"

#include "neat_math.h"

#include "fftclamdfft.h"
#include "openclcontext.h"
#include "cpumemorystorage.h"
#include "openclmemorystorage.h"
#include "TaskTimer.h"
#include "computationkernel.h"
#include "clamdfftkernelbuffer.h"

#include "clAmdFft.h"

#define TIME_STFT
//#define TIME_STFT if(0)


namespace Tfr {

FftClAmdFft::FftClAmdFft()
{
    std::auto_ptr< clAmdFftSetupData > setupData( new clAmdFftSetupData );
    clAmdFftStatus error = clAmdFftInitSetupData( setupData.get( ) );
    if (error != CLFFT_SUCCESS)
        throw std::runtime_error("Could not init setupdata for clAmdFFT.");

    setupData->debugFlags	|= CLFFT_DUMP_PROGRAMS;

    error = clAmdFftSetup( setupData.get( ) );
    if (error != CLFFT_SUCCESS)
        throw std::runtime_error("Could not setup clAmdFFT.");
}

FftClAmdFft::~FftClAmdFft()
{
    clAmdFftStatus error = clAmdFftTeardown( );
    if (error != CLFFT_SUCCESS)
        throw std::runtime_error("Could not tear down clAmdFFT.");
}

void FftClAmdFft:: // Once
        compute( Tfr::ChunkData::Ptr input, Tfr::ChunkData::Ptr output, FftDirection direction )
{
    TIME_STFT TaskTimer tt("Fft AmdClFft");

    unsigned n = input->getNumberOfElements().width;
    unsigned N = output->getNumberOfElements().width;

    if (-1 != direction)
        BOOST_ASSERT( n == N );

    {
        clAmdFftDirection dir = ((direction == FftDirection_Forward) ? CLFFT_FORWARD : CLFFT_BACKWARD);

        TIME_STFT TaskTimer tt("Computing fft(N=%u, n=%u, direction=%d)", N, n, direction);
        OpenCLContext *opencl = &OpenCLContext::Singleton();
        //cl_int fft_error;
        clAmdFftStatus clamdfft_error;

        //clFFT_Plan plan = CLFFTKernelBuffer::Singleton().getPlan(opencl->getContext(), n, fft_error);
        clAmdFftPlanHandle plan = CLAMDFFTKernelBuffer::Singleton().getPlan(opencl, n, clamdfft_error);
        //if (fft_error != CL_SUCCESS)
        if (clamdfft_error != CLFFT_SUCCESS)
            throw std::runtime_error("Could not create clAmdFFT compute plan.");

        // Run the fft in OpenCL :)
        // fft kernel needs to have read/write access to output data

        // clAmdFft client code:
        /*
        OPENCL_V_THROW( clAmdFftEnqueueTransform( plHandle, CLFFT_FORWARD, 1, &queue, 0, NULL, &outEvent,
                                                  &clMemBuffersIn[ 0 ], BuffersOut, clMedBuffer ),
                       "clAmdFftEnqueueTransform failed" );
        */

		cl_mem clMemBuffersIn [ 1 ] = { OpenClMemoryStorage::ReadWrite<1>( input ).ptr() };
		cl_mem clMemBuffersOut [ 1 ] = { OpenClMemoryStorage::ReadWrite<1>( output ).ptr() };

		
	    clAmdFftSetPlanBatchSize(plan, 1);
		{
			TIME_STFT TaskTimer tt5("Baking plan for batch 1");
			clamdfft_error = clAmdFftBakePlan(plan, 1, &opencl->getCommandQueue(), NULL, NULL);
		}
		
		clamdfft_error = clAmdFftEnqueueTransform(
                plan, dir, 1, &opencl->getCommandQueue(), 0, NULL, NULL,
				&clMemBuffersIn[0], &clMemBuffersOut[0],
                NULL );




        // old clFFT code:

        /*
        fft_error |= clFFT_ExecuteInterleaved(
                opencl->getCommandQueue(),
                plan, 1, (clFFT_Direction)direction,
                OpenClMemoryStorage::ReadOnly<1>( input ).ptr(),
                OpenClMemoryStorage::ReadWrite<1>( output ).ptr(),
                0, NULL, NULL );
        */

        //if (fft_error != CL_SUCCESS)
        if (clamdfft_error != CLFFT_SUCCESS)
            throw std::runtime_error("Bad stuff happened during FFT computation.");

        clFinish(opencl->getCommandQueue());
    }
}


void FftClAmdFft::
        computeR2C( DataStorage<float>::Ptr input, Tfr::ChunkData::Ptr output )
{
	/*
    unsigned denseWidth = output->size().width;
    unsigned redundantWidth = input->size().width;

   BOOST_ASSERT( denseWidth == redundantWidth/2+1 );

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
	*/
}


void FftClAmdFft::
        computeC2R( Tfr::ChunkData::Ptr input, DataStorage<float>::Ptr output )
{
/*    unsigned denseWidth = input->size().width;
    unsigned redundantWidth = output->size().width;

    BOOST_ASSERT( denseWidth == redundantWidth/2+1 );

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
	*/
}


void FftClAmdFft:: //Batch
        compute( Tfr::ChunkData::Ptr input, Tfr::ChunkData::Ptr output, DataStorageSize n, FftDirection direction )
{    
    TIME_STFT TaskTimer tt2("Fft AmdClFft");

    unsigned s = input->size().width;
    unsigned S = input->size().width;

    BOOST_ASSERT( output->numberOfBytes() == input->numberOfBytes() );

    clAmdFftDirection dir = ((direction == FftDirection_Forward) ? CLFFT_FORWARD : CLFFT_BACKWARD);

    TIME_STFT TaskTimer tt("Computing fft(N=%u, n=%u, direction=%d, batches=%u)", S, s, direction, n.height);
    OpenCLContext *opencl = &OpenCLContext::Singleton();
    clAmdFftStatus clamdfft_error;

    clAmdFftPlanHandle plan = CLAMDFFTKernelBuffer::Singleton().getPlan(opencl, s, clamdfft_error);
    if (clamdfft_error != CLFFT_SUCCESS)
        throw std::runtime_error("Could not create clAmdFFT compute plan.");

    cl_mem clMemBuffersIn [ 1 ] = { OpenClMemoryStorage::ReadWrite<1>( input ).ptr() };
    cl_mem clMemBuffersOut [ 1 ] = { OpenClMemoryStorage::ReadWrite<1>( output ).ptr() };


    clAmdFftSetPlanBatchSize(plan, n.height);

	{
		TIME_STFT TaskTimer tt5("Baking plan for multibatch");
		clamdfft_error = clAmdFftBakePlan(plan, 1, &opencl->getCommandQueue(), NULL, NULL);
	}


    clamdfft_error = clAmdFftEnqueueTransform(
            plan, dir, 1, &opencl->getCommandQueue(), 0, NULL, NULL,
            &clMemBuffersIn[0], &clMemBuffersOut[0],
            NULL );

    if (clamdfft_error != CLFFT_SUCCESS)
        throw std::runtime_error("Bad stuff happened during FFT computation.");

	/*
    TaskTimer tt("Stft::compute( matrix[%d, %d], %s )",
                 input->size().width,
                 input->size().height,
                 direction==FftDirection_Forward?"forward":"backward");

    BOOST_ASSERT( output->numberOfBytes() == input->numberOfBytes() );

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
	*/
}


void FftClAmdFft:: //R2C
        compute(DataStorage<float>::Ptr input, Tfr::ChunkData::Ptr output, DataStorageSize n )
{
	/*
    unsigned denseWidth = n.width/2+1;

    BOOST_ASSERT( output->numberOfElements()/denseWidth == n.height );
    BOOST_ASSERT( input->numberOfElements()/n.width == n.height );

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
	*/
}


void FftClAmdFft::
        inverse(Tfr::ChunkData::Ptr input, DataStorage<float>::Ptr output, DataStorageSize n )
{
	/*
    unsigned denseWidth = n.width/2+1;
    unsigned redundantWidth = n.width;
    unsigned batchcount1 = output->numberOfElements()/redundantWidth,
             batchcount2 = input->numberOfElements()/denseWidth;

    BOOST_ASSERT( batchcount1 == batchcount2 );
    BOOST_ASSERT( (denseWidth-1)*2 == redundantWidth );
    BOOST_ASSERT( redundantWidth*n.height == output->numberOfElements() );

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
	*/
}


unsigned powerprod(const unsigned*bases, const unsigned*b, unsigned N)
{
    unsigned v = 1;
    for (unsigned i=0; i<N; i++)
        for (unsigned x=0; x<b[i]; x++)
            v*=bases[i];
    return v;
}


unsigned findLargestSmaller(const unsigned* bases, unsigned* a, unsigned maxv, unsigned x, unsigned n, unsigned N)
{
    unsigned i = 0;
    while(true)
    {
        a[n] = i;

        unsigned v = powerprod(bases, a, N);
        if (v >= x)
            break;

        if (n+1<N)
            maxv = findLargestSmaller(bases, a, maxv, x, n+1, N);
        else if (v > maxv)
            maxv = v;

        ++i;
    }
    a[n] = 0;

    return maxv;
}


unsigned findSmallestGreater(const unsigned* bases, unsigned* a, unsigned minv, unsigned x, unsigned n, unsigned N)
{
    unsigned i = 0;
    while(true)
    {
        a[n] = i;

        unsigned v = powerprod(bases, a, N);
        if (n+1<N)
            minv = findSmallestGreater(bases, a, minv, x, n+1, N);
        else if (v > x && (v < minv || minv==0))
            minv = v;

        if (v > x)
            break;

        ++i;
    }
    a[n] = 0;

    return minv;
}

unsigned FftClAmdFft::
        lChunkSizeS(unsigned x, unsigned multiple)
{
    // It's faster but less flexible to only accept powers of 2
    //return lpo2s(x);

    multiple = std::max(1u, multiple);
    BOOST_ASSERT( spo2g(multiple-1) == lpo2s(multiple+1));

    unsigned bases[]={2, 3, 5};
    unsigned a[]={0, 0, 0};
    unsigned N_bases = 3; // could limit to 2 bases
    unsigned x2 = multiple*findLargestSmaller(bases, a, 0, int_div_ceil(x, multiple), 0, N_bases);
    BOOST_ASSERT( x2 < x );
    return x2;
}


unsigned FftClAmdFft::
        sChunkSizeG(unsigned x, unsigned multiple)
{
    // It's faster but less flexible to only accept powers of 2
    //return spo2g(x);

    multiple = std::max(1u, multiple);
    BOOST_ASSERT( spo2g(multiple-1) == lpo2s(multiple+1));

    unsigned bases[]={2, 3, 5};
    unsigned a[]={0, 0, 0};
    unsigned N_bases = 3;
    unsigned x2 = multiple*findSmallestGreater(bases, a, 0, x/multiple, 0, N_bases);
    BOOST_ASSERT( x2 > x );
    return x2;
}

} // namespace Tfr
#endif // #ifdef USE_OPENCL
