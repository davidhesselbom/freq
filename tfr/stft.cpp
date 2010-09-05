#include "stft.h"

#include <cufft.h>
#include <throwInvalidArgument.h>
#include <CudaException.h>
#include <neat_math.h>

#ifdef _MSC_VER
#include <msc_stdc.h>
#endif

//#define TIME_STFT
#define TIME_STFT if(0)

using namespace boost::posix_time;
using namespace boost;

namespace Tfr {

CufftHandleContext::
        CufftHandleContext( cudaStream_t stream )
:   _handle(0),
    _stream(stream)
{}

CufftHandleContext::
        ~CufftHandleContext()
{
    destroy();
    _creator_thread.reset();
}

cufftHandle CufftHandleContext::
        operator()( unsigned elems, unsigned batch_size )
{
    if (0 == _handle || _elems != elems || _batch_size != batch_size) {
        _elems = elems;
        _batch_size = batch_size;
        create();
	} else {
		_creator_thread.throwIfNotSame(__FUNCTION__);
	}
    return _handle;
}

void CufftHandleContext::
        create()
{
    destroy();
    CufftException_SAFE_CALL(cufftPlan1d(&_handle, _elems, CUFFT_C2C, _batch_size));
    CufftException_SAFE_CALL(cufftSetStream(_handle, _stream ));
    _creator_thread.reset();
}

void CufftHandleContext::
        destroy()
{
    if (_handle) {
		_creator_thread.throwIfNotSame(__FUNCTION__);

		CufftException_SAFE_CALL(cufftDestroy(_handle));

        _handle = 0;
    }
}

Fft::
        Fft(/*cudaStream_t stream*/)
//:   _fft_single( stream )
{
}

Fft::
        ~Fft()
{
}

pChunk Fft::
        forward( Signal::pBuffer b)
{
    // cufft is faster for larger ffts, but as the GPU is the bottleneck we can
    // just as well do it on the CPU instead

    if (b->interleaved() != Signal::Buffer::Interleaved_Complex)
    {
        // TODO implement void computeWithOoura( GpuCpuData<double2>& input, GpuCpuData<double2>& output, int direction );
        b = b->getInterleaved( Signal::Buffer::Interleaved_Complex );
    }

    cudaExtent input_n = b->waveform_data->getNumberOfElements();
    cudaExtent output_n = input_n;

    // The in-signal is padded to a power of 2 (cufft manual suggest a "power
    // of a small prime") for faster fft calculations
    output_n.width = spo2g( output_n.width / 2 - 1 );

    pChunk chunk( new Chunk );
    chunk->transform_data.reset( new GpuCpuData<float2>(
            0,
            output_n,
            GpuCpuVoidData::CpuMemory ));

    input_n.width /= 2;
    GpuCpuData<float2> input(
            b->waveform_data->getCpuMemory(),
            input_n,
            GpuCpuVoidData::CpuMemory,
            true );

    // computeWithCufft( input, *chunk->transform_data, -1);
    computeWithOoura( input, *chunk->transform_data, -1 );

    chunk->axis_scale = AxisScale_Linear;
    chunk->chunk_offset = b->sample_offset;
    chunk->first_valid_sample = 0;
    chunk->max_hz = b->sample_rate / 2;
    chunk->min_hz = chunk->max_hz / chunk->nSamples();
    chunk->n_valid_samples = chunk->nSamples();
    chunk->order = Chunk::Order_column_major;
    chunk->sample_rate = b->sample_rate / chunk->nSamples();

    return chunk;
}

Signal::pBuffer Fft::
        backward( pChunk chunk)
{
    cudaExtent output_n = chunk->transform_data->getNumberOfElements();

    // The in-signal is padded to a power of 2 (cufft manual suggest a "power
    // of a small prime") for faster fft calculations
    output_n.width = spo2g( output_n.width - 1 );

    Signal::pBuffer ret( new Signal::Buffer( Signal::Buffer::Interleaved_Complex) );
    ret->waveform_data.reset( new GpuCpuData<float>(
            0,
            output_n,
            GpuCpuVoidData::CpuMemory ));

    cudaExtent output_n2 = output_n;
    output_n2.width *= 2;

    GpuCpuData<float2> output(
                ret->waveform_data->getCpuMemory(),
                output_n2,
                GpuCpuVoidData::CpuMemory,
                true );

    // chunk = computeWithCufft(*chunk->transform_data, output, 1);
    computeWithOoura(*chunk->transform_data, output, 1);

    // output shares ptr with ret
    ret->sample_offset = chunk->chunk_offset;
    ret->sample_rate = chunk->sample_rate * chunk->nSamples();

    return ret;
}

// TODO translate cdft to take floats instead of doubles
//extern "C" { void cdft(int, int, double *); }
extern "C" { void cdft(int, int, double *, int *, double *); }

void Fft::
        computeWithOoura( GpuCpuData<float2>& input, GpuCpuData<float2>& output, int direction )
{
    TIME_STFT TaskTimer tt("Fft Ooura");

    unsigned n = input.getNumberOfElements().width;
    unsigned N = output.getNumberOfElements().width;

    if (q.size() != 2*N) {
        TIME_STFT TaskTimer tt("Resizing buffers");
        q.resize(2*N);
        w.resize(N/2);
        ip.resize(N/2);
        ip[0] = 0;
    } else {
        TIME_STFT TaskTimer("Reusing data").suppressTiming();
    }

    float* p = (float*)input.getCpuMemory();

    {
        TIME_STFT TaskTimer tt("Converting from float2 to double2" );

        for (unsigned i=0; i<2*n; i++)
            q[i] = p[i];

        for (unsigned i=2*n; i<2*N; i++)
            q[i] = 0;
    }

    /*TODO remove {
        TIME_STFT TaskTimer tt("Converting from float%c to double2", buffer->interleaved() == Signal::Buffer::Interleaved_Complex?'2':'1');

        if (buffer->interleaved() == Signal::Buffer::Interleaved_Complex) {
            for (unsigned i=0; i<2*n; i++)
                q[i] = p[i];
        } else {
            for (unsigned i=0; i<n; i++)
            {
                q[2*i + 0] = p[i];
                q[2*i + 1] = 0;
            }
        }

        for (unsigned i=2*n; i<2*N; i++)
            q[i] = 0;
    }*/


    {
        TIME_STFT TaskTimer tt("Computing fft");
        cdft(2*N, direction, &q[0], &ip[0], &w[0]);
    }

    {
        TIME_STFT TaskTimer tt("Converting from double2 to float2");

        p = (float*)output.getCpuMemory();
        for (unsigned i=0; i<2*N; i++)
            p[i] = (float)q[i];
    }
}


void Fft::
        computeWithCufft( GpuCpuData<float2>& input, GpuCpuData<float2>& output, int direction )
{
    TIME_STFT TaskTimer tt("FFt cufft");

    cufftComplex* d = output.getCudaGlobal().ptr();
    cudaMemset( d, 0, output.getSizeInBytes1D() );
    cudaMemcpy( d,
                input.getCudaGlobal().ptr(),
                input.getSizeInBytes().width,
                cudaMemcpyDeviceToDevice );

    // Transform signal
    CufftHandleContext _fft_single;
    CufftException_SAFE_CALL(cufftExecC2C(
        _fft_single(output.getNumberOfElements().width, 1),
        d, d,
        direction==-1?CUFFT_FORWARD:CUFFT_INVERSE));

    TIME_STFT CudaException_ThreadSynchronize();
}


/// STFT

std::vector<unsigned> Stft::_ok_chunk_sizes;

Stft::
        Stft( cudaStream_t stream )
:   _stream( stream ),
    _chunk_size( 1<<11 )
//    _fft_many( -1 )
{
}


// static
Stft& Stft::
        Singleton()
{
    return *dynamic_cast<Stft*>(SingletonP().get());
}


// static
pTransform Stft::
        SingletonP()
{
    static pTransform P(new Stft());
    return P;
}


Tfr::pChunk Stft::
        operator() (Signal::pBuffer b)
{
    const unsigned stream = 0;

    if (b->interleaved() != Signal::Buffer::Interleaved_Complex)
    {
        b = b->getInterleaved( Signal::Buffer::Interleaved_Complex );
    }

    cudaExtent n = make_cudaExtent(
            _chunk_size,
            b->number_of_samples()/_chunk_size,
            1 );

    if (0==n.height)
        return Tfr::pChunk();

    Tfr::pChunk chunk( new Tfr::Chunk() );

    chunk->transform_data.reset( new GpuCpuData<float2>(
            0,
            n,
            GpuCpuVoidData::CudaGlobal ));


    cufftComplex* input = (cufftComplex*)b->waveform_data->getCudaGlobal().ptr();
    cufftComplex* output = (cufftComplex*)chunk->transform_data->getCudaGlobal().ptr();

    // Transform signal
    cufftHandle fft_many;
    unsigned count = n.height;

    unsigned
            slices = n.height,
            i = 0;

    while(i < count)
    {
        try
        {
            CufftException_SAFE_CALL(cufftPlan1d(&fft_many, _chunk_size, CUFFT_C2C, slices));

            CufftException_SAFE_CALL(cufftSetStream(fft_many, stream));
            CufftException_SAFE_CALL(cufftExecC2C(fft_many, &input[i*n.height], &output[i*n.height], CUFFT_FORWARD));
            cufftDestroy(fft_many);

            i += slices;
        } catch (const CufftException&) {
            if (slices>0)
                slices/=2;
            else
                throw;
        }
    }

    chunk->axis_scale = AxisScale_Linear;
    chunk->chunk_offset = b->sample_offset;
    chunk->first_valid_sample = 0;
    chunk->max_hz = b->sample_rate / 2;
    chunk->min_hz = chunk->max_hz / _chunk_size;
    chunk->n_valid_samples = (chunk->nSamples() / _chunk_size)*_chunk_size;
    chunk->order = Chunk::Order_column_major;
    chunk->sample_rate = b->sample_rate / (float)_chunk_size;

    return chunk;
}


static unsigned absdiff(unsigned a, unsigned b)
{
    return a < b ? a - b : b-a;
}


unsigned Stft::set_approximate_chunk_size( unsigned preferred_size )
{
    if (_ok_chunk_sizes.empty())
        build_performance_statistics();

    std::vector<unsigned>::iterator itr =
            std::lower_bound( _ok_chunk_sizes.begin(), _ok_chunk_sizes.end(), preferred_size );

    unsigned N1 = *itr, N2;
    if (itr != _ok_chunk_sizes.end())
        N2 = *++itr;
    else
    {
        N2 = spo2g( preferred_size - 1 );
        N1 = lpo2s( preferred_size + 1 );
    }

    _chunk_size = absdiff(N1, preferred_size) < absdiff(N2, preferred_size) ? N1 : N2;
    return _chunk_size;
}


unsigned Stft::build_performance_statistics(bool writeOutput, float size_of_test_signal)
{
    _ok_chunk_sizes.clear();
    Tfr::Stft S;
    scoped_ptr<TaskTimer> tt;
    Signal::pBuffer B = boost::shared_ptr<Signal::Buffer>( new Signal::Buffer( 0, 44100*60*size_of_test_signal, 44100 ) );
    {
        if(writeOutput) tt.reset( new TaskTimer("Filling buffer with random data"));

        float* p = B->waveform_data->getCpuMemory();
        for (unsigned i = 0; i < B->number_of_samples(); i++)
            p[i] = rand() / (float)RAND_MAX;

        tt.reset();
    }

    time_duration fastest_time;
    unsigned fastest_size = 0;
    unsigned ok_size = 0;
    Tfr::pChunk C;
    unsigned prevN = -1;
    time_duration latest_time[4];
    unsigned max_base = 3;
    double base[] = {2,3,5,7};
    for (unsigned n = 2; n<10000000; n++ )
    {
        unsigned N = -1;
        unsigned selectedBase = 0;
        for (unsigned b=0; b<sizeof(base)/sizeof(base[0]) && b<=max_base; b++)
        {
            unsigned N2 = pow(base[b], (unsigned)(log(n)/log(base[b])));

            unsigned d1 = N>n ? N - n : n - N;
            unsigned d2 = N2>n ? N2 - n : n - N2;
            if (d2<d1)
            {
                selectedBase = b;
                N = N2;
            }

            N2 = pow(base[b], (unsigned)(log(n)/log(base[b])) + 1);

            d1 = N>n ? N - n : n - N;
            d2 = N2>n ? N2 - n : n - N2;
            if (d2<d1)
            {
                selectedBase = b;
                N = N2;
            }
        }

        if (N == prevN)
            continue;

        prevN = N;

        S._chunk_size = N;

        {
            if(writeOutput) tt.reset( new TaskTimer( "n=%u, _chunk_size = %u = %g ^ %g ",
                                                     n, S._chunk_size,
                                                     base[selectedBase],
                                                     log(S._chunk_size)/log(base[selectedBase])));

            ptime startTime = microsec_clock::local_time();
            C = S( B );
            time_duration diff = microsec_clock::local_time() - startTime;

            if (0<selectedBase)
                if (diff.total_microseconds() > latest_time[0].total_microseconds()*1.5)
                {
                    max_base = selectedBase-1;
                    if(tt) tt->info("max_base = %u", max_base);
                }

            latest_time[selectedBase] = diff;

            if (diff.total_milliseconds() < fastest_time.total_microseconds()*1.2)
                ok_size = S._chunk_size;

            if (diff < fastest_time || 0==fastest_size)
            {
                max_base = sizeof(base)/sizeof(base[0]) - 1;
                fastest_time = diff;
                fastest_size = S._chunk_size;
            }

            _ok_chunk_sizes.push_back( S._chunk_size );

            tt.reset();
        }
        C.reset();

        if (S._chunk_size > B->number_of_samples())
            break;
    }

    return fastest_size;
}

} // namespace Tfr
