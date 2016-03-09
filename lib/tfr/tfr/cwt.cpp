#include "cwt.h"
#include "cwtchunk.h"
#include "stft.h"
#include "waveletkernel.h"
#include "supersample.h"

#include "signal/buffersource.h"

// gpumisc
//#include <cufft.h>
#include "throwInvalidArgument.h"
#include "computationkernel.h"
#include "neat_math.h"
#include "Statistics.h"
#include "unused.h"

#ifdef USE_CUDA
#include "cudaglobalstorage.h"
#include "cudaMemsetFix.cu.h"
#endif

// std
#include <cmath>
#include <float.h>
#include <limits>

// boost
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>

#ifdef _WIN32
    #include "msc_stdc.h"

    #define _USE_MATH_DEFINES
    #include <math.h>
#endif


#define TIME_CWT if(0)
//#define TIME_CWT

#define STAT_CWT if(0)
//#define STAT_CWT

#define TIME_CWTPART if(0)
//#define TIME_CWTPART

#define TIME_ICWT if(0)
//#define TIME_ICWT

#define DEBUG_CWT if(0)
//#define DEBUG_CWT

//#define CWT_NOBINS // Also change cwtfilter.cpp

//#define CWT_DISCARD_PREVIOUS_FT
#define CWT_DISCARD_PREVIOUS_FT if(0)

const bool AdjustToBin0 = true;

using namespace boost::lambda;
using namespace std;

namespace Tfr {

Cwt::
        Cwt( float scales_per_octave, float wavelet_time_suppport, float number_of_octaves )
:   _number_of_octaves( number_of_octaves ),
    _scales_per_octave( 0 ),
    _tf_resolution( 2.5 ), // 2.5 is Ulfs magic constant
    _least_meaningful_fraction_of_r( 0.01f ),
    _least_meaningful_samples_per_chunk( 1024 ),
    _wavelet_time_suppport( wavelet_time_suppport ),
    _wavelet_def_time_suppport( wavelet_time_suppport ),
    _wavelet_scale_suppport( 6 ),
    _jibberish_normalization( 1 )
{
#ifdef USE_CUDA
    storageCudaMemsetFix = &cudaMemsetFix;
#endif
    fft(0); // init
    this->scales_per_octave( scales_per_octave );
}


pChunk Cwt::
        operator()( Signal::pMonoBuffer buffer )
{
#ifdef USE_CUDA
    try {
#endif

    boost::scoped_ptr<TaskTimer> tt;
    TIME_CWT tt.reset( new TaskTimer (
            "Forward CWT( buffer interval=%s, [%g, %g)%g# s)",
            buffer->getInterval().toString().c_str(),
            buffer->start(), buffer->length()+buffer->start(), buffer->length() ));

    TIME_CWT STAT_CWT Statistics<float>(buffer->waveform_data());

#ifdef USE_CUDA
    // move data to Gpu to start with, this will make new buffers created from this
    // to also be allocated on the Gpu
    CudaGlobalStorage::ReadOnly<1>(buffer->waveform_data());
#endif

    Signal::BufferSource bs( buffer );

    Signal::IntervalType offset = buffer->sample_offset().asInteger();
    int std_samples = wavelet_time_support_samples();
    //unsigned std_samples0 = time_support_bin0( buffer->sample_rate() );
    Signal::IntervalType first_valid_sample = std_samples;

    EXCEPTION_ASSERT(buffer->number_of_samples() > 2*std_samples);
    // Align first_valid_sample with chunks (round upwards)
    first_valid_sample = align_up(offset + first_valid_sample, (Signal::IntervalType) chunk_alignment()) - offset;

    EXCEPTION_ASSERT( std_samples + first_valid_sample < buffer->number_of_samples());

    unsigned valid_samples = (unsigned)(buffer->number_of_samples() - std_samples - first_valid_sample);
    // Align valid_samples with chunks (round downwards)
    unsigned alignment = chunk_alignment();
    valid_samples = align_down(valid_samples, alignment);
    EXCEPTION_ASSERT( 0 < valid_samples );

    //unsigned L = 2*std_samples0 + valid_samples;
    //bool ispowerof2 = spo2g(L-1) == lpo2s(L+1);

    size_t free = availableMemoryForSingleAllocation();
    bool trypowerof2;
    {
        unsigned r, T0 = required_length( 0, r );
        unsigned smallest_L2 = spo2g(T0-1) - 2*r;
        size_t smallest_required2 = required_gpu_bytes(smallest_L2);
        trypowerof2 = smallest_required2 <= free;
    }

    DEBUG_CWT {
        TaskInfo("Free memory: %s. Required: %f MB",
             DataStorageVoid::getMemorySizeText( free ).c_str(),
             DataStorageVoid::getMemorySizeText( required_gpu_bytes(valid_samples) ).c_str());
    }

    DEBUG_CWT TaskTimer("offset = %l", (long)offset).suppressTiming();
    DEBUG_CWT TaskTimer("std_samples = %lu", std_samples).suppressTiming();
    DEBUG_CWT TaskTimer("first_valid_sample = %l", (long)first_valid_sample).suppressTiming();
    DEBUG_CWT TaskTimer("valid_samples = %u", valid_samples).suppressTiming();

    // find all sub chunks
    unsigned prev_j = 0;
    unsigned n_j = nScales();

    pChunk ft;
    pChunk wt( new CwtChunk() );

    DEBUG_CWT
    {
        static bool list_scales = true;
        if (list_scales)
        {
            list_scales = false;

            TaskTimer tt("bins, scales per octave = %g, tf_resolution = %g, sigma = %g",
                         _scales_per_octave, _tf_resolution, sigma());

            for (unsigned j = prev_j; j<n_j; ++j)
            {
                TaskInfo(boost::format("j = %s, nf = %s, bin = %s, time_support in samples = %s")
                                      % j % j_to_nf( j ) % find_bin( j ) % wavelet_time_support_samples( j_to_nf(j) ));
            }
        }
    }

    unsigned max_bin = find_bin( nScales() - 1 );
    for( unsigned c=0; prev_j<n_j; ++c )
    {
        // find the biggest j that is required to be in this chunk
        unsigned next_j;
        if (c == max_bin)
            next_j = n_j;
        else
        {
            next_j = prev_j;

            for (unsigned j = prev_j; j<n_j; ++j)
            {
                if ( c == find_bin( j ) )
                    next_j = j;
            }

            if (next_j==prev_j)
                continue;
        }

        // If the biggest j required to be in this chunk is close to the end
        // 'n_j' then include all remaining scales in this chunk as well.
        if (2*(n_j - next_j) < n_j - prev_j || next_j+2>=n_j)
            next_j = n_j;

        // Move next_j forward one step so that it points to the first 'j'
        // that is not needed in this chunk part
        next_j = min(n_j, next_j+1);

        // Include next_j in this chunk so that parts can be interpolated
        // between in filters
        unsigned stop_j = min(n_j, next_j+1);
        unsigned nScales_value = nScales();
        EXCEPTION_ASSERT( stop_j <= nScales_value);

        unsigned n_scales = stop_j - prev_j;
        float nf = j_to_nf(stop_j-1);
        DEBUG_CWT TaskTimer("c=%u, nf=%g, 2^c=%u, n_scales=%u", c, nf, 1<<c, n_scales).suppressTiming();

        int sub_std_samples = wavelet_time_support_samples( nf );
        EXCEPTION_ASSERT( sub_std_samples <= std_samples );

        DEBUG_CWT TaskTimer("sub_std_samples=%u", sub_std_samples).suppressTiming();
        EXCEPTION_ASSERT( sub_std_samples <= first_valid_sample );

        Signal::IntervalType sub_start = offset + first_valid_sample - sub_std_samples;
        unsigned sub_length = sub_std_samples + valid_samples + sub_std_samples;
        EXCEPTION_ASSERT( sub_length == valid_samples + 2*sub_std_samples );

        // Add some extra length to make length align with fast fft calculations
        unsigned extra = 0;
        if (trypowerof2)
            extra = spo2g(sub_length - 1) - sub_length;
        else
            extra = fft()->sChunkSizeG(sub_length - 1, chunkpart_alignment( c )) - sub_length;

        //this can be asserted if we compute valid interval based on the widest chunk
        if (!AdjustToBin0)
            if (nScales_value == stop_j && 0 < offset)
                EXCEPTION_ASSERT( 0 == extra );

#ifdef _DEBUG
        UNUSED( Signal::IntervalType sub_start_org ) = sub_start;
        UNUSED( unsigned sub_std_samples_org ) = sub_std_samples;
        UNUSED( unsigned sub_length_org ) = sub_length;
#endif

        //sub_std_samples += extra/2;
        sub_start -= extra/2;
        sub_length += extra;

        Signal::Interval subinterval(sub_start, sub_start + sub_length );

        CWT_DISCARD_PREVIOUS_FT
                ft.reset();

        if (!ft || ft->getInterval() != subinterval)
        {
            TIME_CWTPART TaskTimer tt(
                    "Computing forward fft of interval %s, was %s",
                    subinterval.toString().c_str(),
                    ft?ft->getInterval().toString().c_str():0 );

            Signal::pMonoBuffer data;

            //this can be asserted if we compute valid interval based on the widest chunk
            if (!AdjustToBin0)
                EXCEPTION_ASSERT( (Signal::Intervals(subinterval) - buffer->getInterval()).empty() );
            data = bs.readFixedLength( subinterval )->getChannel (0); // bs is created from a monobuffer

            ComputationSynchronize();

            TIME_CWTPART TaskTimer t2("Doing fft");
            ft = Fft()( data );
            size_t c = ft->getInterval().count();
            size_t c2 = data->number_of_samples();
            EXCEPTION_ASSERT( c == c2 );
            ComputationSynchronize();
        }

        // downsample the signal by shortening the fourier transform
        ((StftChunk*)ft.get())->setHalfs( c );
        pChunk chunkpart = computeChunkPart( ft, prev_j, n_scales );

        // The fft is most often bigger than strictly needed because it is
        // faster to compute lengths that are powers of 2.
        // However, to do proper merging we want to guarantee that all
        // chunkparts describe the exact same region. Thus we discard the extra
        // samples we added when padding to a power of 2
        chunkpart->first_valid_sample = int((offset + first_valid_sample - subinterval.first) >> c);
        chunkpart->n_valid_samples = valid_samples >> c;

        DEBUG_CWT {
            TaskTimer tt("Intervals");
            TaskInfo(boost::format("ft(c)=%s") % ft->getInterval());
            TaskInfo(boost::format(" adjusted chunkpart=%s") % chunkpart->getInterval());
            TaskInfo(boost::format(" units=[%s, %s), count=%s") % (chunkpart->getInterval().first >> (max_bin-c)) %
                           (chunkpart->getInterval().last >> (max_bin-c)) %
                           (chunkpart->getInterval().count() >> (max_bin-c)));
        }

        ((CwtChunk*)wt.get())->chunks.push_back( chunkpart );

        // reset halfs if ft can be used for the next bin
        ((StftChunk*)ft.get())->setHalfs( 0 );

        prev_j = next_j;
    }


    wt->freqAxis = freqAxis( buffer->sample_rate() );
    wt->chunk_offset = buffer->sample_offset() + first_valid_sample;
    wt->first_valid_sample = 0;
    wt->n_valid_samples = valid_samples;
    wt->sample_rate = buffer->sample_rate();
    wt->original_sample_rate = buffer->sample_rate();

    DEBUG_CWT {
        size_t sum = 0;
        size_t alt = 0;
        BOOST_FOREACH( const pChunk& chunkpart, ((CwtChunk*)wt.get())->chunks )
        {
            size_t s = chunkpart->transform_data->numberOfBytes();
            sum += s;
            size_t tmp_stft_and_fft = s + sizeof(Tfr::ChunkElement)*chunkpart->nSamples()*chunkpart->original_sample_rate/chunkpart->sample_rate;
            if (sum + tmp_stft_and_fft > alt)
                alt = sum + tmp_stft_and_fft; // biggest stft
        }

        if (alt>sum)
            sum = alt;

        TaskInfo("Free memory: %s. Allocated %s",
                 DataStorageVoid::getMemorySizeText( free ).c_str(),
                 DataStorageVoid::getMemorySizeText( sum ).c_str());
    }

    DEBUG_CWT TaskTimer("wt->max_hz = %g, wt->min_hz = %g", wt->maxHz(), wt->minHz()).suppressTiming();

    TIME_CWT STAT_CWT TaskInfo(boost::format("Resulting interval = %s") % wt->getInterval());
    TIME_CWT ComputationSynchronize();
    ComputationCheckError();

    TIME_CWT STAT_CWT
    {
        TaskInfo ti("stat cwt");
        BOOST_FOREACH( const pChunk& chunkpart, ((CwtChunk*)wt.get())->chunks )
        {
            DataStorageSize sz = chunkpart->transform_data->size();
            sz.width *= 2;
            Statistics<float> s( CpuMemoryStorage::BorrowPtr<float>( sz, (float*)CpuMemoryStorage::ReadOnly<2>(chunkpart->transform_data).ptr() ));
        }
    }

#ifdef _DEBUG
    Signal::Interval chunkInterval = wt->getInterval();

    int subchunki = 0;
    BOOST_FOREACH( const pChunk& chunk, dynamic_cast<Tfr::CwtChunk*>(wt.get())->chunks )
    {
        Signal::Interval cii = chunk->getInterval();
        EXCEPTION_ASSERT( chunkInterval == cii );

        ++subchunki;
    }
#endif

    return wt;
#ifdef USE_CUDA
    } catch (CufftException const& /*x*/) {
        TaskInfo("Cwt::operator() caught CufftException");
        throw;
    } catch (CudaException const& /*x*/) {
        TaskInfo("Cwt::operator() caught CudaException");
        throw;
    }
#endif

    //clearFft();
    //return wt;
}


TransformDesc::ptr Cwt::
        copy() const
{
    return TransformDesc::ptr(new Cwt(*this));
}


pTransform Cwt::
        createTransform() const
{
    return pTransform(new Cwt(*this));
}


FreqAxis Cwt::
        freqAxis( float FS ) const
{
    FreqAxis fa;
    fa.setLogarithmic(
            get_min_hz( FS ),
            get_max_hz( FS ),
            nScales() - 1 );
    return fa;
}


float Cwt::
        displayedTimeResolution( float FS, float hz ) const
{
    return morlet_sigma_samples( hz_to_nf(FS, hz)) / FS;
}


Signal::Interval Cwt::
        requiredInterval( const Signal::Interval& I, Signal::Interval* expectedOutput ) const
{
    const Tfr::Cwt& cwt = *this;

    unsigned chunk_alignment = cwt.chunk_alignment();
    Signal::IntervalType firstSample = I.first;
    firstSample = align_down(firstSample, (Signal::IntervalType) chunk_alignment);

    unsigned time_support = cwt.wavelet_time_support_samples();
    firstSample -= time_support;
    auto c = I.count ();
    if (c > ((unsigned)-1)/8)
        c = ((unsigned)-1)/8;
    else if (c < 2)
        c = 2;
    unsigned numberOfSamples = cwt.next_good_size( (int)c-1 );

    // hack to make it work without subsampling
    #ifdef CWT_NOBINS
    numberOfSamples = cwt.next_good_size( 1 );
    #endif

    unsigned L = time_support + numberOfSamples + time_support;

    if (expectedOutput)
        *expectedOutput = Signal::Interval(firstSample + time_support, firstSample+L - time_support);

    return Signal::Interval(firstSample, firstSample+L);
}


Signal::Interval Cwt::
        affectedInterval( const Signal::Interval& I ) const
{
    Signal::IntervalType n = wavelet_time_support_samples();

    return Signal::Intervals(I).enlarge( n ).spannedInterval ();
}

//Signal::Interval Cwt::
//        validLength(Signal::pBuffer buffer)
//{
//    return Signal::Intervals(buffer->getInterval()).shrink( wavelet_time_support_samples(buffer->sample_rate()) ).spannedInterval();
//}


bool Cwt::
        operator==(const TransformDesc& b) const
{
    const Cwt* p = dynamic_cast<const Cwt*>(&b);
    if (!p)
        return false;

    return
            _number_of_octaves == p->_number_of_octaves &&
            _scales_per_octave == p->_scales_per_octave &&
            _tf_resolution == p->_tf_resolution &&
            _least_meaningful_fraction_of_r == p->_least_meaningful_fraction_of_r &&
            _least_meaningful_samples_per_chunk == p->_least_meaningful_samples_per_chunk &&
            _wavelet_time_suppport == p->_wavelet_time_suppport &&
            _wavelet_def_time_suppport == p->_wavelet_def_time_suppport &&
            _wavelet_scale_suppport == p->_wavelet_scale_suppport &&
            _jibberish_normalization == p->_jibberish_normalization;
}


pChunk Cwt::
        computeChunkPart( pChunk ft, unsigned first_scale, unsigned n_scales )
{
    EXCEPTION_ASSERT( n_scales > 1 || (first_scale == 0 && n_scales==1) );
    TIME_CWTPART TaskTimer tt("computeChunkPart first_scale=%u, n_scales=%u, (%g to %g Hz)",
                              first_scale, n_scales, j_to_hz(ft->original_sample_rate, first_scale+n_scales-1),
                              j_to_hz(ft->original_sample_rate, first_scale));

    pChunk intermediate_wt( new CwtChunkPart() );

    {
        DataStorageSize requiredWtSz( dynamic_cast<StftChunk*>(ft.get())->transformSize(), n_scales, 1 );
#ifdef USE_CUDA
        TIME_CWTPART TaskTimer tt("Allocating chunk part (%u, %u, %u), %g kB",
                              requiredWtSz.width, requiredWtSz.height, requiredWtSz.depth,
                              requiredWtSz.width* requiredWtSz.height* requiredWtSz.depth * sizeof(Tfr::ChunkElement) / 1024.f);
#endif

        // allocate a new chunk
        intermediate_wt->transform_data.reset(new ChunkData( requiredWtSz ));

#ifdef USE_CUDA
        TIME_CWTPART {
            CudaGlobalStorage::useCudaPitch( intermediate_wt->transform_data, false );
            CudaGlobalStorage::ReadOnly<1>( ft->transform_data );
            CudaGlobalStorage::WriteAll<1>( intermediate_wt->transform_data );
        }
#endif

        TIME_CWTPART ComputationSynchronize();
    }

    unsigned half_sizes = dynamic_cast<Tfr::StftChunk*>(ft.get())->halfs();

    {
        TIME_CWTPART TaskTimer tt("inflating");

        // ft->sample_rate is related to intermediate_wt->sample_rate by
        // intermediate_wt->sample_rate == ft->n_valid_samples * ft->sample_rate
        // (except for numerical errors)
        intermediate_wt->sample_rate = ldexp(ft->original_sample_rate, -(int)half_sizes);
        intermediate_wt->original_sample_rate = ft->original_sample_rate;

        unsigned last_scale = first_scale + n_scales-1;
        intermediate_wt->freqAxis.setLogarithmic(
                get_max_hz(ft->original_sample_rate)*exp2f( last_scale/-_scales_per_octave ),
                get_max_hz(ft->original_sample_rate)*exp2f( first_scale/-_scales_per_octave ),
                intermediate_wt->nScales()-1 );

        DEBUG_CWT TaskInfo tinfo("scales [%u,%u]%u#, hz [%g, %g]",
                 first_scale, last_scale, n_scales,
                 intermediate_wt->maxHz(), intermediate_wt->minHz());

        DEBUG_CWT
        {
            TaskTimer("ft->sample_rate = %g", ft->sample_rate).suppressTiming();
            TaskTimer("ft->original_sample_rate = %g", ft->original_sample_rate).suppressTiming();
            TaskTimer("ft->halfs = %u", half_sizes).suppressTiming();
            TaskTimer("intermediate_wt->sample_rate = %g", intermediate_wt->sample_rate).suppressTiming();
            TaskTimer("intermediate_wt->min_hz = %g", intermediate_wt->minHz()).suppressTiming();
            TaskTimer("intermediate_wt->max_hz = %g", intermediate_wt->maxHz()).suppressTiming();
        }

        if( intermediate_wt->maxHz() > intermediate_wt->sample_rate/2 * (1.0+10*FLT_EPSILON) )
        {
            TaskInfo("intermediate_wt->max_hz = %g", intermediate_wt->maxHz());
            TaskInfo("intermediate_wt->sample_rate = %g", intermediate_wt->sample_rate);

            EXCEPTION_ASSERT( intermediate_wt->maxHz() <= intermediate_wt->sample_rate/2 * (1.0+10*FLT_EPSILON) );
        }

        ::wtCompute( ft->transform_data,
                     intermediate_wt->transform_data,
                     intermediate_wt->sample_rate,
                     intermediate_wt->minHz(),
                     intermediate_wt->maxHz(),
                     1<<half_sizes,
                     _scales_per_octave, sigma(),
                     _jibberish_normalization );

        TIME_CWTPART ComputationSynchronize();
    }

    {
        // Compute the inverse fourier transform to get the filter banks back
        // in time space
        ChunkData::ptr g = intermediate_wt->transform_data;
        DataStorageSize n = g->size();
        TIME_CWTPART TaskTimer tt("inverse stft(%u, %u)", n.width, n.height);

        intermediate_wt->chunk_offset = ft->getInterval().first;

        float intermediate_wt_min_nf = intermediate_wt->minHz() / (ft->original_sample_rate/2);
        unsigned time_support = wavelet_time_support_samples( intermediate_wt_min_nf );
        time_support >>= half_sizes;
        intermediate_wt->first_valid_sample = time_support;

        if (0 == ft->chunk_offset.asFloat())
            intermediate_wt->first_valid_sample=0;

        DEBUG_CWT {
            TaskTimer("time_support = %u", time_support).suppressTiming();
            TaskTimer("intermediate_wt->first_valid_sample=%u", intermediate_wt->first_valid_sample).suppressTiming();
            TaskTimer("ft->n_valid_samples=%u", ft->n_valid_samples).suppressTiming();
        }

        EXCEPTION_ASSERT( time_support + intermediate_wt->first_valid_sample < ft->getInterval().count() );

        intermediate_wt->n_valid_samples = (int)(ft->getInterval().count() - time_support - intermediate_wt->first_valid_sample);

        StftDesc stft;
        stft.set_exact_chunk_size(n.width);
        Stft(stft).compute( g, g, Tfr::FftDirection_Inverse );

//        if (0 /* cpu version */ ) {
//            TIME_CWTPART TaskTimer tt("inverse ooura, redundant=%u+%u valid=%u",
//                                  intermediate_wt->first_valid_sample,
//                                  intermediate_wt->nSamples() - intermediate_wt->n_valid_samples - intermediate_wt->first_valid_sample,
//                                  intermediate_wt->n_valid_samples);

//            // Move to CPU
//            ChunkElement* p = g->getCpuMemory();

//            pChunk c( new CwtChunk );
//            for (unsigned h=0; h<n.height; h++) {
//                c->transform_data = CpuMemoryStorage::BorrowPtr<ChunkElement>(
//                        DataStorageSize(n.width, 1, 1), p + n.width*h );

//                DataStorage<float>::Ptr fb = Fft().backward( c )->waveform_data();
//                memcpy( p + n.width*h, fb->getCpuMemory(), fb->getSizeInBytes1D() );
//            }

//            // Move back to GPU
//            CudaGlobalStorage::ReadWrite<1>( g );
//        }
//        if (1 /* gpu version */ ) {
//            TIME_CWTPART TaskTimer tt("inverse cufft, redundant=%u+%u valid=%u, size(%u, %u)",
//                                  intermediate_wt->first_valid_sample,
//                                  intermediate_wt->nSamples() - intermediate_wt->n_valid_samples - intermediate_wt->first_valid_sample,
//                                  intermediate_wt->n_valid_samples,
//                                  n.width, n.height);

//            cufftComplex *d = (cufftComplex *)CudaGlobalStorage::ReadWrite<1>( g ).device_ptr();


//            {
//                //CufftHandleContext& fftctx = _fft_many[ n.width*n.height ];
//                CufftHandleContext fftctx; // TODO time optimization of keeping CufftHandleContext, "seems" unstable

//                {
//                    //TIME_CWTPART TaskTimer tt("Allocating inverse fft");
//                    fftctx(n.width, n.height);
//                }

//                DEBUG_CWT {
//                    size_t free = availableMemoryForSingleAllocation();
//                    size_t required = n.width*n.height*sizeof(float2)*2;
//                    TaskInfo("free = %g MB, required = %g MB", free/1024.f/1024.f, required/1024.f/1024.f);
//                }
//                TIME_CWTPART TaskInfo("n = { %u, %u, %u }, d = %ul", n.width, n.height, n.depth, g->numberOfElements());
//                CufftException_SAFE_CALL(cufftExecC2C(fftctx(n.width, n.height), d, d, CUFFT_INVERSE));
//            }

//            TIME_CWTPART ComputationSynchronize();
//        }
    }

    return intermediate_wt;
}


Signal::pMonoBuffer Cwt::
        inverse( pChunk pchunk )
{
    ComputationCheckError();

    Chunk* chunk = pchunk.get();
    Tfr::CwtChunk* cwtchunk = dynamic_cast<Tfr::CwtChunk*>(chunk);
    if (cwtchunk)
        return inverse(cwtchunk);

    Tfr::CwtChunkPart* cwtchunkpart = dynamic_cast<Tfr::CwtChunkPart*>(chunk);
    if (cwtchunkpart)
        return inverse(cwtchunkpart);

    throw invalid_argument("Doesn't recognize chunk of type " + demangle( typeid(*chunk)));
}


Signal::pMonoBuffer Cwt::
        inverse( Tfr::CwtChunk* pchunk )
{
    TIME_ICWT TaskTimer tt("Inverse CWT( chunk %s, first_valid_sample=%u, [%g, %g] s)",
        pchunk->getInterval().toString().c_str(),
        pchunk->first_valid_sample,
        pchunk->startTime(),
        pchunk->endTime()
        );

    Signal::Interval v = pchunk->getInterval();
    Signal::pMonoBuffer r( new Signal::MonoBuffer( v, pchunk->original_sample_rate ));
    memset( r->waveform_data()->getCpuMemory(), 0, r->waveform_data()->numberOfBytes() );

    BOOST_FOREACH( pChunk& part, pchunk->chunks )
    {
        DEBUG_CWT TaskTimer tt("ChunkPart inverse, c=%g, [%g, %g] Hz",
            log2f(part->original_sample_rate/part->sample_rate),
            part->freqAxis.min_hz, part->freqAxis.max_hz());

        Signal::pMonoBuffer inv = inverse(part);
        Signal::pMonoBuffer super = SuperSample::supersample(inv, pchunk->original_sample_rate);

        DEBUG_CWT TaskInfo(boost::format("Upsampled inv %s by factor %s to %s")
                     % inv->getInterval()
                     % (pchunk->sample_rate/inv->sample_rate())
                     % super->getInterval());

        //TaskInfo("super->getInterval() = %s, first_valid_sample = %u",
        //         super->getInterval().toString().c_str(), part->first_valid_sample);

        *r += *super;
    }

    EXCEPTION_ASSERT( pchunk->getInterval() == r->getInterval() );
    TIME_ICWT {
        STAT_CWT Statistics<float>( r->waveform_data() );
    }

    TIME_ICWT ComputationSynchronize();

    return r;
}


Signal::pMonoBuffer Cwt::
        inverse( Tfr::CwtChunkPart* pchunk )
{
    Chunk &chunk = *pchunk;

    DataStorageSize x = chunk.transform_data->size();

    Signal::pMonoBuffer r( new Signal::MonoBuffer(
            chunk.chunk_offset,
            x.width,
            chunk.sample_rate
            ));

    if (pchunk->original_sample_rate != pchunk->sample_rate)
    {
        // Skip last row
        x.height--;
    }

    ::wtInverse( chunk.transform_data,
                 r->waveform_data(),
                 x );

    TIME_ICWT ComputationSynchronize();

    return r;
}


Tfr::FftImplementation::ptr Cwt::
        fft() const
{
    return fft_instances.find (0)->second;
}


Tfr::FftImplementation::ptr Cwt::
        fft(int width)
{
    Tfr::FftImplementation::ptr& i = fft_instances[width];
    i = Tfr::FftImplementation::newInstance ();
    fft_usage.insert (width);
    return i;
}


void Cwt::
        clearFft()
{
    typedef map<int, Tfr::FftImplementation::ptr> fmap;

    for (fmap::iterator i = fft_instances.begin (); i!=fft_instances.end (); ++i)
    {
        if (0 < fft_usage.count (i->first))
            fft_usage.erase (i->first);
    }

    fft_usage.erase (0);

    for (set<int>::iterator i = fft_usage.begin (); i!=fft_usage.end (); ++i)
        fft_instances.erase (*i);

    fft_usage.clear ();
}


float Cwt::
        get_min_hz( float fs ) const
{
    unsigned n_j = nScales();
    return j_to_hz( fs, n_j - 1 );
}


float Cwt::
        get_min_nf() const
{
    unsigned n_j = nScales();
    return j_to_nf( n_j - 1 );
}


float Cwt::
        get_wanted_min_hz(float fs) const
{
     return get_max_hz(fs) / exp2f(_number_of_octaves);
}


void Cwt::
        set_wanted_min_hz(float min_hz, float fs)
{
    float number_of_octaves = log2f(get_max_hz(fs)) - log2f(min_hz);

    if (number_of_octaves < 0)
        number_of_octaves = 0;

    if (number_of_octaves == _number_of_octaves)
        return;

    _number_of_octaves = number_of_octaves;
}


unsigned Cwt::
        nScales() const
{
    return 1 + ceil(_number_of_octaves * _scales_per_octave);
}


unsigned Cwt::
        nBins() const
{
    return find_bin( nScales()-1 );
}


void Cwt::
        scales_per_octave( float value, float fs )
{
    scales_per_octave_internal( value );

    if (fs != 0)
    {
        // check available memory
        next_good_size(1, fs);
    }
}


void Cwt::
        scales_per_octave_internal( float value )
{
    if (value==_scales_per_octave)
        return;

    EXCEPTION_ASSERT(!isinf(value));
    EXCEPTION_ASSERT(!isnan(value));

    _scales_per_octave=value;

    float w = M_PI/2;
    float phi_sum = 0;
    float v = _scales_per_octave;
    float log2_a = 1.f / v;

    DEBUG_CWT TaskInfo ti("Cwt::scales_per_octave( %g )", value);
    for (int j=0; j<2*_scales_per_octave; ++j)
    {
        float aj = exp2f(log2_a * j );
        float q = (-w*aj + M_PI)*sigma();
        float phi_star = expf( -q*q );

        DEBUG_CWT TaskInfo("%d: %g", j, phi_star );
        phi_sum += phi_star;
    }

    float sigma_constant = sqrt( 4*M_PI*sigma() );
    _jibberish_normalization = 1 / (phi_sum * sigma_constant);

    DEBUG_CWT TaskInfo("phi_sum  = %g", phi_sum );
    DEBUG_CWT TaskInfo("sigma_constant  = %g", sigma_constant );
    DEBUG_CWT TaskInfo("_jibberish_normalization  = %g", _jibberish_normalization );
}


void Cwt::
        tf_resolution( float value )
{
    if (value == _tf_resolution) return;

    _tf_resolution = value;
}


float Cwt::
        sigma() const
{
    return _scales_per_octave/_tf_resolution;
}


float Cwt::
        morlet_sigma_samples( float nf ) const
{
    // float scale = hz/get_max_hz( fs );
    // float j = -_scales_per_octave*log2f(scale);
    // const float log2_a = 1.f / _scales_per_octave; // a = 2^(1/v), v = _scales_per_octave
    // float aj = exp2f( log2_a * j );

    // float aj = get_max_hz( fs )/hz;

    float aj = 1 / nf;

    return aj*sigma();
}


float Cwt::
        morlet_sigma_f( float hz ) const
{
    // float aj = get_max_hz( fs )/hz; // see morlet_sigma_t
    // float sigmaW0 = get_max_hz( fs ) / (2*M_PI*aj* sigma() );
    float sigmaW0 = hz / (2*M_PI* sigma() );
    return sigmaW0;
}


unsigned Cwt::
        wavelet_time_support_samples() const
{
    return max(1u, wavelet_time_support_samples( get_min_nf() ));
}


unsigned Cwt::
        wavelet_time_support_samples( float nf ) const
{
    unsigned support_samples = ceil(morlet_sigma_samples( nf ) * _wavelet_time_suppport);
    unsigned c = find_bin( nf_to_j(nf) );
    // Make 2*support_samples align to chunk_alignment for the lowest frequency
    unsigned half_chunkpart_alignment = chunkpart_alignment( c )/2;
    support_samples = align_up( support_samples, half_chunkpart_alignment );
    return support_samples;
}


unsigned Cwt::
        required_length( unsigned current_valid_samples_per_chunk, unsigned &r ) const
{
    unsigned alignment = chunk_alignment();
    if (AdjustToBin0)
        r = time_support_bin0();
    else
    {
        r = wavelet_time_support_samples();
        EXCEPTION_ASSERT( ((2*r) % alignment) == 0 );
    }
    current_valid_samples_per_chunk = max((unsigned)(_least_meaningful_fraction_of_r*r), current_valid_samples_per_chunk);
    current_valid_samples_per_chunk = max(_least_meaningful_samples_per_chunk, current_valid_samples_per_chunk);
    current_valid_samples_per_chunk = align_up(current_valid_samples_per_chunk, alignment);
    unsigned T = r + current_valid_samples_per_chunk + r;
    if (AdjustToBin0)
        T = fft()->sChunkSizeG(T-1, chunkpart_alignment( 0 ));
    else
        T = fft()->sChunkSizeG(T-1, alignment);
    return T;
}


unsigned Cwt::
        next_good_size( unsigned current_valid_samples_per_chunk ) const
{
    DEBUG_CWT TaskInfo ti(boost::format("next_good_size(%u)") % current_valid_samples_per_chunk);
    unsigned r, T0 = required_length( 0, r );

    size_t free = availableMemoryForSingleAllocation();

    // check power of 2 if possible, multi-radix otherwise
    unsigned smallest_L2 = spo2g(T0-1) - 2*r;
    size_t smallest_required2 = required_gpu_bytes(smallest_L2);
    bool testPo2  = smallest_required2 <= free;

    unsigned T = required_length( current_valid_samples_per_chunk, r );
    if (T - 2*r > current_valid_samples_per_chunk)
        T--;

    unsigned L;
    unsigned alignment = chunk_alignment();

    if (AdjustToBin0)
    {
        unsigned nT;
        if (testPo2)
            nT = align_up( spo2g(T), chunkpart_alignment( 0 ));
        else
            nT = fft()->sChunkSizeG(T, chunkpart_alignment( 0 ));
        EXCEPTION_ASSERT(nT != T);

        unsigned nL = nT - 2*r;
        L = align_down(nL, alignment);
        if (current_valid_samples_per_chunk >= L || alignment > L)
        {
            L = max(alignment, align_up(nL, alignment));
            T = L + 2*r;
            if (testPo2)
                nT = align_up( spo2g(T), chunkpart_alignment( 0 ));
            else
                nT = fft()->sChunkSizeG(T, chunkpart_alignment( 0 ));
            EXCEPTION_ASSERT(nT != T);
            nL = nT - 2*r;
            L = align_down(nL, alignment);
        }
    }
    else
    {
        unsigned nT;
        if (testPo2)
            nT = align_up( spo2g(T), alignment);
        else
            nT = fft()->sChunkSizeG(T, alignment);
        EXCEPTION_ASSERT(nT != T);

        L = nT - 2*r;
        EXCEPTION_ASSERT( L % alignment == 0 );
    }

    size_t required = required_gpu_bytes( L );

    if (free < required)
    {
        DEBUG_CWT TaskInfo("next_good_size: current_valid_samples_per_chunk = %u (L=%u, r=%u) requires %s. Free: %s",
                 current_valid_samples_per_chunk, L, r,
                 DataStorageVoid::getMemorySizeText( required ).c_str(),
                 DataStorageVoid::getMemorySizeText( free ).c_str() );
        return prev_good_size( L );
    }
/*        unsigned nTtest = Fft::sChunkSizeG(T, chunk_alignment( fs ));
        unsigned Ltest = nTtest - 2*r;

        required = required_gpu_bytes(Ltest, fs );
        TaskInfo("next_good_size: Ltest = %u requires %f MB",
                 Ltest,
                 required/1024.f/1024.f);
        if (free < required)
            return prev_good_size( L, fs );
        else
            L = Ltest;
    }*/

    DEBUG_CWT TaskInfo("Cwt::next_good_size free = %s, required = %s",
                       DataStorageVoid::getMemorySizeText( free ).c_str(),
                       DataStorageVoid::getMemorySizeText( required ).c_str());
    return L;
}


unsigned Cwt::
        prev_good_size( unsigned current_valid_samples_per_chunk ) const
{
    DEBUG_CWT TaskInfo ti(boost::format("prev_good_size(%u)") % current_valid_samples_per_chunk);

    // Check if the smallest possible size is ok memory-wise
    unsigned r, smallest_T = required_length( 1, r );
    unsigned smallest_L = smallest_T - 2*r;
    unsigned alignment = chunk_alignment();

    size_t free = availableMemoryForSingleAllocation();
    size_t smallest_required = required_gpu_bytes(smallest_L);

    DEBUG_CWT TaskInfo(
            "prev_good_size: smallest_L = %u, chunk_alignment() = %u. Free: %s, required memory: %s",
             smallest_L, alignment,
             DataStorageVoid::getMemorySizeText( free ).c_str(),
             DataStorageVoid::getMemorySizeText( smallest_required ).c_str() );

    EXCEPTION_ASSERT( smallest_L + 2*r >= alignment );

    if (smallest_required <= free)
    {
        // current scales per octave is ok, take something smaller than
        // 'current_valid_samples_per_chunk'. Don't bother with
        // detailed sizes (sChunkSizeG), just use powers of 2 but make sure
        // it's still larger than chunk_alignment.

        // start searching from the requested number of valid samples per chunk
        unsigned T = required_length(current_valid_samples_per_chunk, r);

        // check power of 2 if possible, multi-radix otherwise
        unsigned smallest_L2 = spo2g(smallest_L-1 + 2*r) - 2*r;
        size_t smallest_required2 = required_gpu_bytes(smallest_L2);
        if (smallest_required2 <= free)
            smallest_L = smallest_L2;

        bool testPo2 = smallest_L == smallest_L2;

        while (true)
        {
            if (testPo2)
                T = align_up( lpo2s(T), (unsigned) 2);
            else
                T = fft()->lChunkSizeS(T, 2);

            //check whether we have reached the smallest acceptable length
            if ( T <= smallest_L + 2*r)
                return smallest_L;

            unsigned L = T - 2*r;
            L = max(alignment, align_down(L, alignment));

            size_t required = required_gpu_bytes(L);

            if(required <= free)
                // found something that works, take it
                return L;
        }
    }

    DEBUG_CWT TaskInfo("prev_good_size: scales_per_octave was %g", scales_per_octave());

    // No, the current size doesn't fit in memory
    return 0;

//    // decrease scales per octave to the largest possible value
//    largest_scales_per_octave( fs, scales_per_octave() );

//    // scales per octave has been changed here, recompute 'r' and return
//    // the smallest possible L
//    smallest_T = required_length( 1, fs, r );
//    smallest_L = smallest_T - 2*r;

//    smallest_required = required_gpu_bytes(smallest_L, fs);
//    DEBUG_CWT TaskInfo("prev_good_size: scales_per_octave is %g, smallest_L = %u, required = %f MB",
//                       scales_per_octave(), smallest_L,
//                       DataStorageVoid::getMemorySizeText( smallest_required ).c_str());

//    return smallest_L;
}


void Cwt::
        largest_scales_per_octave( float scales, float last_ok )
{   
    if (scales_per_octave()<2)
    {
        scales = 0;
        last_ok = 2;
    }

    if (scales <= 0.1)
    {
        EXCEPTION_ASSERT(last_ok != 0);
        scales_per_octave( last_ok );

        TaskInfo("largest_scales_per_octave is %g", scales_per_octave());
        return;
    }

    scales /= 2;

    if (is_small_enough())
    {
        last_ok = scales_per_octave();
        scales_per_octave(scales_per_octave() + scales);
    }
    else
    {
        scales_per_octave(scales_per_octave() - scales);
    }

    largest_scales_per_octave( scales, last_ok );
}


bool Cwt::
        is_small_enough() const
{
    unsigned r, T = required_length( 1, r );
    unsigned L = T - 2*r;

    size_t free = availableMemoryForSingleAllocation();
    size_t required = required_gpu_bytes( L );
    return free >= required;
}


size_t Cwt::
        required_gpu_bytes(unsigned valid_samples) const
{
    DEBUG_CWT TaskInfo ti("required_gpu_bytes(%u), scales_per_octave=%g, wavelet_time_suppport=%g", valid_samples, _scales_per_octave, _wavelet_time_suppport);

/*    unsigned r = wavelet_time_support_samples( sample_rate );
    unsigned max_bin = find_bin( nScales( sample_rate ) - 1 );
    long double sum = sizeof(float2)*(L + 2*r)*nScales( sample_rate )/(1+max_bin);

    // sum is now equal to the amount of memory required by the biggest CwtChunk
    // the stft algorithm needs to allocate the same amount of memory while working (+sum)
    // the rest of the chunks are in total basically equal to the size of the biggest chunk (+sum)
    sum = 3*sum;
*/
    size_t sum = 0;
    size_t alt = 0;

    unsigned max_bin = find_bin( nScales() - 1 );
    unsigned
            prev_j = 0,
            n_j = nScales();

    unsigned std_samples = wavelet_time_support_samples();

    for( unsigned c=0; prev_j<n_j; ++c )
    {
        // find the biggest j that is required to be in this chunk
        unsigned next_j;
        if (c == max_bin)
            next_j = n_j;
        else
        {
            next_j = prev_j;

            for (unsigned j = prev_j; j<n_j; ++j)
            {
                if ( c == find_bin( j ) )
                    next_j = j;
            }

            if (next_j==prev_j)
                continue;
        }

        // If the biggest j required to be in this chunk is close to the end
        // 'n_j' then include all remaining scales in this chunk as well.
        if (2*(n_j - next_j) < n_j - prev_j || next_j+2>=n_j)
            next_j = n_j;

        // Move next_j forward one step so that it points to the first 'j'
        // that is not needed in this chunk part
        next_j = min(n_j, next_j+1);

        // Include next_j in this chunk so that parts can be interpolated
        // between in filters
        unsigned stop_j = min(n_j, next_j+1);

        unsigned n_scales = stop_j - prev_j;
        float nf = j_to_nf(stop_j-1);

        unsigned sub_std_samples = wavelet_time_support_samples( nf );
        unsigned sub_length = 2*sub_std_samples+valid_samples;

        unsigned L = valid_samples + 2*std_samples;
        bool ispowerof2 = spo2g(L-1) == lpo2s(L+1);
        if (ispowerof2)
            sub_length = spo2g(sub_length - 1);
        else
            sub_length = fft()->sChunkSizeG(sub_length - 1, chunk_alignment());

        size_t s = 2*sizeof(Tfr::ChunkElement)*(sub_length >> c)*n_scales;

        sum += s;
        size_t tmp_stft_and_fft = s + 2*sizeof(Tfr::ChunkElement)*(sub_length);
        if (sum + tmp_stft_and_fft > alt)
            alt = sum + tmp_stft_and_fft;

        //TaskInfo("c = %d -> %f MB", c, (s+ tmp_stft_and_fft)/1024.f/1024.f);
        prev_j = next_j;
    }

    if (alt>sum)
        sum = alt;

    DEBUG_CWT TaskInfo(boost::format("%s") % DataStorageVoid::getMemorySizeText(sum));
    return sum;
}


string Cwt::
        toString() const
{
#ifdef _DEBUG
    stringstream ss;
    ss << "Tfr::Cwt"
       << ", number_of_octaves=" << _number_of_octaves
       << ", scales_per_octave=" << _scales_per_octave
       << ", wavelet_time_suppport=" << _wavelet_time_suppport
       << ", wavelet_scale_suppport=" << _wavelet_scale_suppport;
    return ss.str();
#else
    stringstream ss;
    ss << "Wavelet " << _scales_per_octave << " scales/octave";
    return ss.str();
#endif
}


unsigned Cwt::
        chunk_alignment() const
{
    return chunkpart_alignment(nBins());
}


unsigned Cwt::
        chunkpart_alignment(unsigned c) const
{
    unsigned sample_size = 1<<c;
    // In subchunk number 'c', one sample equals 'sample_size' samples in the
    // original sample rate.
    // To be able to use supersampling properly the length must be a multiple
    // of 2. Thus all subchunks need to be aligned to their respective
    // '2*sample_size'.
    return 2*sample_size;
}


unsigned Cwt::
        find_bin( unsigned j ) const
{
#ifdef CWT_NOBINS
    return 0;
#endif

    float v = _scales_per_octave;
    float log2_a = 1.f/v;
    float bin = log2_a * j - log2f( 1.f + _wavelet_scale_suppport/(2*M_PI*sigma()) );

    if (bin < 0)
        bin = 0;

    // could take maximum number of bins into account and meld all the last
    // ones into the same bin, effectively making the second last bin all empty
    // unsigned n_j = nScales( fs );

    return floor(bin);
}


unsigned Cwt::
        time_support_bin0() const
{
//    return wavelet_time_support_samples( fs, j_to_hz( fs, 0 ) );
    float highest_nf_inBin0 = 0.f;
    unsigned j;
    for (j=0; j<1000u;j++)
        if (0 != find_bin(j))
        {
            highest_nf_inBin0 = j_to_nf( j-1 );
            break;
        }

    EXCEPTION_ASSERT_LESS(j,1000u);
    return wavelet_time_support_samples( highest_nf_inBin0 );
}


float Cwt::
        j_to_hz( float sample_rate, unsigned j ) const
{
    float nf = j_to_nf(j);
    return nf_to_hz(sample_rate, nf);
}


unsigned Cwt::
        hz_to_j( float sample_rate, float hz ) const
{
    float nf = hz_to_nf(sample_rate, hz);
    return nf_to_j(nf);
}


unsigned Cwt::
        nf_to_j( float normalized_frequency ) const
{
    // normalized_frequency = hz / (fs/2)
    float v = _scales_per_octave;
    float j = -log2f(normalized_frequency)*v;
    j = floor(j+.5f);
    if (j<0)
        j=0;
    return (unsigned)j;
}


float Cwt::
        j_to_nf( unsigned j ) const
{
    float v = _scales_per_octave;
    return exp2f(j/-v);
}


float Cwt::
        hz_to_nf( float sample_rate, float hz ) const
{
    return hz / get_max_hz(sample_rate);
}


float Cwt::
        nf_to_hz( float sample_rate, float nf ) const
{
    return nf * get_max_hz(sample_rate);
}



} // namespace Tfr
