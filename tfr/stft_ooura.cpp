#if !defined(USE_CUDA) && !defined(USE_OPENCL)
#include "stft.h"
#include "stftkernel.h"

#include "cpumemorystorage.h"
#include "complexbuffer.h"
#include "TaskTimer.h"
#include "computationkernel.h"

//#define TIME_STFT
#define TIME_STFT if(0)

// TODO translate cdft to take floats instead of doubles
//extern "C" { void cdft(int, int, double *); }
extern "C" { void cdft(int, int, double *, int *, double *); }
// extern "C" { void cdft(int, int, float *, int *, float *); }


namespace Tfr {


void Fft::
        computeWithOoura( Tfr::ChunkData::Ptr input, Tfr::ChunkData::Ptr output, FftDirection direction )
{
    TIME_STFT TaskTimer tt("Fft Ooura");

    unsigned n = input->getNumberOfElements().width;
    unsigned N = output->getNumberOfElements().width;

    if (-1 != direction)
        BOOST_ASSERT( n == N );

    int magic = 12345678;
    bool vector_length_test = true;

    std::vector<double> w(N/2 + vector_length_test);
    std::vector<int> ip(2+(1<<(int)(log2f(N+0.5)-1)) + vector_length_test);
    std::vector<double> q(2*N + vector_length_test);

    ip[0] = 0;

    if (vector_length_test)
    {
        q.back() = magic;
        w.back() = magic;
        ip.back() = magic;
    }


    {
        TIME_STFT TaskTimer tt("Converting from float2 to double2" );

        float* p = (float*)input->getCpuMemory();
        for (unsigned i=0; i<2*n; i++)
            q[i] = p[i];

        for (unsigned i=2*n; i<2*N; i++)
            q[i] = 0;
    }


    {
        TIME_STFT TaskTimer tt("Computing fft(N=%u, n=%u, direction=%d)", N, n, direction);
        cdft(2*N, direction, &q[0], &ip[0], &w[0]);

        if (vector_length_test)
        {
            BOOST_ASSERT(q.back() == magic);
            BOOST_ASSERT(ip.back() == magic);
            BOOST_ASSERT(w.back() == magic);
        }
    }

    {
        TIME_STFT TaskTimer tt("Converting from double2 to float2");

        float* p = (float*)output->getCpuMemory();
        for (unsigned i=0; i<2*N; i++)
            p[i] = (float)q[i];
    }
}


void Fft::
        computeWithOouraR2C( DataStorage<float>::Ptr input, Tfr::ChunkData::Ptr output )
{
    unsigned denseWidth = output->size().width;
    unsigned redundantWidth = input->size().width;

   BOOST_ASSERT( denseWidth == redundantWidth/2+1 );

    // interleave input to complex data
    Tfr::ChunkData::Ptr complexinput( new Tfr::ChunkData( input->size()));
    ::stftToComplex( input, complexinput );

    // make room for full output
    Tfr::ChunkData::Ptr redundantOutput( new Tfr::ChunkData( redundantWidth ));

    // compute
    computeWithOoura(complexinput, redundantOutput, FftDirection_Forward);

    // discard redundant output
    {
        Tfr::ChunkElement* in = CpuMemoryStorage::ReadOnly<1>( redundantOutput ).ptr();
        Tfr::ChunkElement* out = CpuMemoryStorage::WriteAll<1>( output ).ptr();
        unsigned x;
        for (x=0; x<denseWidth; ++x)
            out[x] = in[x];
    }
}


void Fft::
        computeWithOouraC2R( Tfr::ChunkData::Ptr input, DataStorage<float>::Ptr output )
{
    unsigned denseWidth = input->size().width;
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

    ComplexBuffer buffer( 0, redundantWidth, 1 );

    computeWithOoura(redundantInput, buffer.complex_waveform_data(), FftDirection_Inverse);

    *output = *buffer.get_real()->waveform_data();
}


void Stft::
        computeWithOoura( Tfr::ChunkData::Ptr inputdata, Tfr::ChunkData::Ptr outputdata, DataStorageSize n, FftDirection direction )
{
    Tfr::ChunkElement* input = CpuMemoryStorage::ReadOnly<1>( inputdata ).ptr();
    Tfr::ChunkElement* output = CpuMemoryStorage::WriteAll<1>( outputdata ).ptr();

    BOOST_ASSERT( inputdata->numberOfBytes() == outputdata->numberOfBytes() );

    // Transform signal

    Fft ft(true);

    int i=0;
    ft.computeWithOoura(
            CpuMemoryStorage::BorrowPtr<Tfr::ChunkElement>( n.width,
                                                            input + i*n.width),
            CpuMemoryStorage::BorrowPtr<Tfr::ChunkElement>( n.width,
                                                            output + i*n.width),
            direction
    );

#pragma omp parallel for
    for (i=1; i < (int)n.height; ++i)
    {
        ft.computeWithOoura(
                CpuMemoryStorage::BorrowPtr<Tfr::ChunkElement>( n.width,
                                                                input + i*n.width),
                CpuMemoryStorage::BorrowPtr<Tfr::ChunkElement>( n.width,
                                                                output + i*n.width),
                direction
        );
    }

    TIME_STFT ComputationSynchronize();
}


void Stft::
        computeWithOoura(DataStorage<float>::Ptr input, Tfr::ChunkData::Ptr output, DataStorageSize n)
{
    unsigned denseWidth = n.width;

    BOOST_ASSERT( output->numberOfElements()/denseWidth == n.height );
    BOOST_ASSERT( input->numberOfElements()/_window_size == n.height );
    BOOST_ASSERT( denseWidth == _window_size/2+1 );

    // interleave input to complex data
    Tfr::ChunkData::Ptr complexinput( new Tfr::ChunkData( input->size()));
    ::stftToComplex( input, complexinput );

    // make room for full output
    Tfr::ChunkData::Ptr redundantOutput( new Tfr::ChunkData( _window_size*n.height ));

    // compute
    computeWithOoura(complexinput, redundantOutput, DataStorageSize( _window_size, n.height ), FftDirection_Forward);

    // discard redundant output
    Tfr::ChunkElement* in = CpuMemoryStorage::ReadOnly<1>( redundantOutput ).ptr();
    Tfr::ChunkElement* out = CpuMemoryStorage::WriteAll<1>( output ).ptr();

#pragma omp parallel for
    for (int i=0; i < (int)n.height; ++i)
    {
        unsigned x;
        for (x=0; x<denseWidth; ++x)
            out[i*denseWidth + x] = in[i*_window_size+x];
    }
}


void Stft::
        inverseWithOoura(Tfr::ChunkData::Ptr input, DataStorage<float>::Ptr output, DataStorageSize n)
{
    unsigned denseWidth = n.width/2+1;
    unsigned redundantWidth = n.width;
    unsigned batchcount1 = output->numberOfElements()/redundantWidth,
             batchcount2 = input->numberOfElements()/denseWidth;

    BOOST_ASSERT( batchcount1 == batchcount2 );
    BOOST_ASSERT( (denseWidth-1)*2 == redundantWidth );

    Tfr::ChunkData::Ptr redundantInput( new Tfr::ChunkData( n.height*redundantWidth ));

    {
        Tfr::ChunkElement* in = CpuMemoryStorage::ReadOnly<1>( input ).ptr();
        Tfr::ChunkElement* out = CpuMemoryStorage::WriteAll<1>( redundantInput ).ptr();
#pragma omp parallel for
        for (int i=0; i<(int)n.height; ++i)
        {
            unsigned x;
            for (x=0; x<denseWidth; ++x)
                out[i*redundantWidth + x] = in[i*denseWidth + x];
            for (; x<redundantWidth; ++x)
                out[i*redundantWidth + x] = conj(in[i*denseWidth + redundantWidth - x]);
        }
    }

    ComplexBuffer buffer( 0, redundantWidth*n.height, 1 );

    computeWithOoura(redundantInput, buffer.complex_waveform_data(), DataStorageSize( redundantWidth, n.height), FftDirection_Inverse);

    *output = *buffer.get_real()->waveform_data();

    TIME_STFT ComputationSynchronize();
}


} // namespace Tfr
#endif // #if !defined(USE_CUDA) && !defined(USE_OPENCL)
