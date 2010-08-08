#include "tfr-cwt.h"
#include <cufft.h>
#include "tfr-stft.h"
#include <throwInvalidArgument.h>
#include <CudaException.h>
#include "tfr-wavelet.cu.h"
#include <neat_math.h>

#ifdef _MSC_VER
#include <msc_stdc.h>
#endif

#define TIME_CWT if(0)
//#define TIME_CWT

namespace Tfr {

Cwt::
        Cwt( float scales_per_octave, float wavelet_std_t, cudaStream_t stream )
:   _fft( /*stream*/ ),
    _stream( stream ),
    _min_hz( 20 ),
    _scales_per_octave( scales_per_octave ),
    _tf_resolution( 1 ),
//    _fft_many(stream),
    _wavelet_std_t( wavelet_std_t )
{
}

pChunk Cwt::
        operator()( Signal::pBuffer buffer )
{
    std::stringstream ss;
    TIME_CWT TaskTimer tt("Cwt buffer %s, %u samples. [%g, %g] s", ((std::stringstream&)(ss<<buffer->getInterval())).str().c_str(), buffer->number_of_samples(), buffer->start(), buffer->length()+buffer->start() );

    pFftChunk ft ( _fft( buffer ) );

    pChunk intermediate_wt( new Chunk() );

    {
        cudaExtent requiredWtSz = make_cudaExtent( ft->getNumberOfElements().width, nScales(buffer->sample_rate), 1 );
        TIME_CWT TaskTimer tt("prerequisites (%u, %u, %u)", requiredWtSz.width, requiredWtSz.height, requiredWtSz.depth);

        // allocate a new chunk
        intermediate_wt->transform_data.reset(new GpuCpuData<float2>( 0, requiredWtSz, GpuCpuVoidData::CudaGlobal ));

        TIME_CWT CudaException_ThreadSynchronize();
    }

    {
        TIME_CWT TaskTimer tt("inflating");
        intermediate_wt->sample_rate =  buffer->sample_rate;
        intermediate_wt->min_hz = _min_hz;
        intermediate_wt->max_hz = max_hz(buffer->sample_rate);

        ::wtCompute( ft->getCudaGlobal().ptr(),
                     intermediate_wt->transform_data->getCudaGlobal().ptr(),
                     intermediate_wt->sample_rate,
                     intermediate_wt->min_hz,
                     intermediate_wt->max_hz,
                     intermediate_wt->transform_data->getNumberOfElements(),
                     _scales_per_octave, _tf_resolution );

        TIME_CWT CudaException_ThreadSynchronize();
    }

    {
        // Transform signal back
        GpuCpuData<float2>* g = intermediate_wt->transform_data.get();
        cudaExtent n = g->getNumberOfElements();

        intermediate_wt->chunk_offset = buffer->sample_offset;
        intermediate_wt->first_valid_sample = wavelet_std_samples( buffer->sample_rate );

        if (0==buffer->sample_offset)
            intermediate_wt->first_valid_sample=0;

        intermediate_wt->max_hz = max_hz( buffer->sample_rate );
        intermediate_wt->min_hz = min_hz();

        BOOST_ASSERT(wavelet_std_samples( buffer->sample_rate ) + intermediate_wt->first_valid_sample < buffer->number_of_samples());

        intermediate_wt->n_valid_samples = buffer->number_of_samples() - wavelet_std_samples( buffer->sample_rate ) - intermediate_wt->first_valid_sample;

        intermediate_wt->sample_rate = buffer->sample_rate;

        if (0 /* cpu version */ ) {
            TIME_CWT TaskTimer tt("inverse ooura, redundant=%u+%u valid=%u",
                                  intermediate_wt->first_valid_sample,
                                  intermediate_wt->nSamples() - intermediate_wt->n_valid_samples - intermediate_wt->first_valid_sample,
                                  intermediate_wt->n_valid_samples);

            // Move to CPU
            float2* p = g->getCpuMemory();

            Signal::pBuffer b( new Signal::Buffer(Signal::Buffer::Interleaved_Complex) );
            for (unsigned h=0; h<n.height; h++) {
                b->waveform_data.reset(
                        new GpuCpuData<float>(p + n.width*h,
                                       make_cudaExtent(2*n.width,1,1),
                                       GpuCpuVoidData::CpuMemory, true));
                pFftChunk fc = _fft.backward( b );
                memcpy( p + n.width*h, fc->getCpuMemory(), fc->getSizeInBytes1D() );
            }

            // Move back to GPU
            g->getCudaGlobal( false );
        }
        if (1 /* gpu version */ ) {
            TIME_CWT TaskTimer tt("inverse cufft, redundant=%u+%u valid=%u",
                                  intermediate_wt->first_valid_sample,
                                  intermediate_wt->nSamples() - intermediate_wt->n_valid_samples - intermediate_wt->first_valid_sample,
                                  intermediate_wt->n_valid_samples);

            cufftComplex *d = g->getCudaGlobal().ptr();

            CufftHandleContext _fft_many;
            CufftException_SAFE_CALL(cufftExecC2C(_fft_many(n.width, n.height), d, d, CUFFT_INVERSE));

            TIME_CWT CudaException_ThreadSynchronize();
        }
    }

    return intermediate_wt;
}

void Cwt::
        min_hz(float value)
{
    if (value == _min_hz) return;

    _min_hz = value;
}

float Cwt::
        number_of_octaves( unsigned sample_rate ) const
{
    return log2(max_hz(sample_rate)) - log2(_min_hz);
}

void Cwt::
        scales_per_octave( float value )
{
    if (value==_scales_per_octave) return;

    _scales_per_octave=value;
}

void Cwt::
        tf_resolution( float value )
{
    if (value == _tf_resolution) return;

    _tf_resolution = value;
}

float Cwt::
        compute_frequency(float normalized_scale, unsigned FS ) const
{
    float start = min_hz();
    float steplogsize = log(max_hz(FS))-log(min_hz());

    float ff = normalized_scale;
    float hz = start*exp(ff*steplogsize);
    return hz;
}

float Cwt::
        morlet_std_t( float normalized_scale, unsigned FS )
{
    float start = 1.f/min_hz();
    float steplogsize = log(max_hz(FS))-log(min_hz());

    float ff = normalized_scale;
    float period = start*exp(-ff*steplogsize);

    // Compute value of analytic FT of wavelet
    float f0 = 2.0f + 35*ff*ff*ff;
    f0 *= tf_resolution();
    //const float pi = 3.141592654f;
    //const float two_pi_f0 = 2.0f * pi * f0;
    //const float multiplier = 1.8827925275534296252520792527491f;

    period *= f0;

    //float factor = 4*pi*(ff)*period-two_pi_f0;
    // float basic = multiplier * exp(-0.5f*factor*factor);

    //float m = jibberish_normalization*cufft_normalize*basic*f0/sqrt(tf_resolution);

    // morlet time space:      const*exp(-.5t^2+i*freq*t)
    // morlet frequency space: const*exp(-.5(freq-w)^2)
    // w = two_pi_f0
    // freq = factor + two_pi_f0
//    return period;
    return period;
}

float Cwt::
        morlet_std_f( float normalized_scale, unsigned FS )
{
    float start = .5f*FS/min_hz();
    float steplogsize = log(max_hz(FS))-log(min_hz());

    float ff = normalized_scale;
    float period = start*exp(-ff*steplogsize);

    // Compute value of analytic FT of wavelet
    float f0 = 2.0f + 35*ff*ff*ff;
    f0 *= tf_resolution();
    const float pi = 3.141592654f;
    const float two_pi_f0 = 2.0f * pi * f0;
    // const float multiplier = 1.8827925275534296252520792527491f;

    period *= f0;

    float freq = compute_frequency( normalized_scale, FS );
    freq = (freq-min_hz())/(max_hz(FS)-min_hz());
    float factor = 4*pi*(freq)*period-two_pi_f0;
    // float basic = multiplier * exp(-0.5f*factor*factor);

    return factor;//*(max_hz(FS)-min_hz());
}

unsigned Cwt::
        wavelet_std_samples( unsigned sample_rate ) const
{
    return ((unsigned)(_wavelet_std_t*sample_rate+31))/32*32;
}

unsigned Cwt::
        next_good_size( unsigned current_valid_samples_per_chunk, unsigned sample_rate )
{
    unsigned r = wavelet_std_samples( sample_rate );
    unsigned T = r + current_valid_samples_per_chunk + r;
    unsigned nT = spo2g(T);
    if(nT <= 2*r)
        nT = spo2g(2*r);
    return nT - 2*r;
}

unsigned Cwt::
        prev_good_size( unsigned current_valid_samples_per_chunk, unsigned sample_rate )
{
    unsigned r = wavelet_std_samples( sample_rate );
    unsigned T = r + current_valid_samples_per_chunk + r;
    unsigned nT = lpo2s(T);
    if (nT<= 2*r)
        nT = spo2g(2*r);
    return nT - 2*r;
}

pChunk CwtSingleton::
        operate( Signal::pBuffer b )
{
    return (*instance())( b );
}

pCwt CwtSingleton::
        instance()
{
    static pCwt cwt( new Cwt ());
    return cwt;
}

} // namespace Tfr
