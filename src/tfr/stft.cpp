#include "stft.h"
#include "stftkernel.h"
#include "complexbuffer.h"
#include "signal/buffersource.h"
#include "cpumemorystorage.h"
#include "exceptionassert.h"

#include <throwInvalidArgument.h>
#include <neat_math.h>
#include <computationkernel.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef _MSC_VER
#include <msc_stdc.h>
#endif


//#define TIME_STFT
#define TIME_STFT if(0)

//#define TEST_FT_INVERSE
#define TEST_FT_INVERSE if(0)


#define STFT_ASSERT EXCEPTION_ASSERT
// #define STFT_ASSERT EXCEPTION_ASSERT

#if defined(USE_CUDA) && !defined(USE_CUFFT)
#define USE_CUFFT
#endif

#ifdef USE_CUFFT
#include "CudaProperties.h"
#endif

using namespace boost::posix_time;
using namespace boost;

namespace Tfr {


Fft::
        Fft( bool computeRedundant )
            :
            _compute_redundant( computeRedundant ),
            fft( FftImplementation::newInstance () )
{
}


Fft::
        ~Fft()
{
}


pChunk Fft::
        forward( Signal::pMonoBuffer real_buffer)
{
    TIME_STFT TaskTimer tt("Fft::forward %s", real_buffer->getInterval().toString().c_str());

    // cufft is faster for larger ffts, but as the GPU is the bottleneck we can
    // just as well offload it some and do it on the CPU instead

    DataStorageSize input_n = real_buffer->waveform_data()->size();
    DataStorageSize output_n = input_n;

    // The in-signal is padded to a power of 2 (cufft manual suggest a "multiple
    // of 2, 3, 5 or 7" but a power of one is even better) for faster fft calculations
    output_n.width = Fft::sChunkSizeG( output_n.width - 1 );

    pChunk chunk;

    // TODO choose method based on data size and locality
    if (_compute_redundant)
    {
        Tfr::ChunkData::Ptr input( new Tfr::ChunkData( real_buffer->waveform_data()->size()));
        ::stftToComplex( real_buffer->waveform_data(), input );

        chunk.reset( new StftChunk( output_n.width, StftParams::WindowType_Rectangular, output_n.width, true ) );
        chunk->transform_data.reset( new ChunkData( output_n ));

        fft->compute( input, chunk->transform_data, FftDirection_Forward );
        chunk->freqAxis.setLinear( real_buffer->sample_rate(), chunk->nScales()/2 );
    }
    else
    {
        STFT_ASSERT(output_n.width == input_n.width);

        if (output_n.width != input_n.width)
        {
            Signal::Interval I( real_buffer->sample_offset().asInteger(), (real_buffer->sample_offset() + output_n.width).asInteger());
            real_buffer = Signal::BufferSource( real_buffer ).readFixedLength( I )->getChannel (0);
        }

        chunk.reset( new StftChunk(output_n.width, StftParams::WindowType_Rectangular, output_n.width, false) );
        output_n.width = ((StftChunk*)chunk.get())->nScales();
        chunk->transform_data.reset( new ChunkData( output_n ));

        fft->computeR2C( real_buffer->waveform_data(), chunk->transform_data );
        chunk->freqAxis.setLinear( real_buffer->sample_rate(), chunk->nScales() );
    }

    chunk->chunk_offset = real_buffer->sample_offset()/(float)input_n.width;
    chunk->first_valid_sample = 0;
    chunk->n_valid_samples = 1;
    chunk->sample_rate = real_buffer->sample_rate()/(float)input_n.width;
    chunk->original_sample_rate = real_buffer->sample_rate();

    TIME_STFT ComputationSynchronize();

    return chunk;
}


pTransform Fft::
        createTransform() const
{
    return pTransform(new Fft(_compute_redundant));
}


FreqAxis Fft::
        freqAxis( float FS ) const
{
    FreqAxis fa;
    fa.setLinear( FS, FS/2 );
    return fa;
}


float Fft::
        displayedTimeResolution( float FS, float /*hz*/ ) const
{
    return 1/FS;
}


unsigned Fft::
        next_good_size( unsigned current_valid_samples_per_chunk, float /*sample_rate*/ ) const
{
    size_t maxsize = std::min( (size_t)(64<<20), (size_t)availableMemoryForSingleAllocation() );
    maxsize /= 3*sizeof(ChunkElement);
    return std::min(maxsize, (size_t)fft->sChunkSizeG(current_valid_samples_per_chunk));
}


unsigned Fft::
        prev_good_size( unsigned current_valid_samples_per_chunk, float /*sample_rate*/ ) const
{
    size_t maxsize = std::min( (size_t)(64<<20), (size_t)availableMemoryForSingleAllocation() );
    maxsize /= 3*sizeof(ChunkElement);
    return std::min(maxsize, (size_t)fft->lChunkSizeS(current_valid_samples_per_chunk));
}


std::string Fft::
        toString() const
{
    std::stringstream ss;
    ss << "Tfr::Fft"
       << ", redundant=" << _compute_redundant;
    return ss.str();
}


bool Fft::
        operator==(const TransformParams& b) const
{
    if (typeid(b)!=typeid(*this))
        return false;

    const Fft* p = dynamic_cast<const Fft*>(&b);

    return _compute_redundant == p->_compute_redundant;
}


Signal::pMonoBuffer Fft::
        backward( pChunk chunk)
{
    TIME_STFT TaskTimer tt("Fft::backward %s", chunk->getInterval().toString().c_str());

    Signal::pMonoBuffer r;
    float fs = chunk->original_sample_rate;
    if (((StftChunk*)chunk.get())->redundant())
    {
        unsigned scales = chunk->nScales();

        // original_sample_rate == fs * scales
        Tfr::ChunkData::Ptr output( new Tfr::ChunkData( scales ));

        fft->compute( chunk->transform_data, output, FftDirection_Inverse );

        r.reset( new Signal::MonoBuffer(0, scales, fs ));
        ::stftDiscardImag( output, r->waveform_data() );
    }
    else
    {
        unsigned scales = ((StftChunk*)chunk.get())->window_size();

        r.reset( new Signal::MonoBuffer(0, scales, fs ));

        fft->computeC2R( chunk->transform_data, r->waveform_data() );
    }

    unsigned original_sample_count = chunk->original_sample_rate/chunk->sample_rate + .5f;
    if ( r->number_of_samples() != original_sample_count )
        r = Signal::BufferSource(r).readFixedLength( Signal::Interval(0, original_sample_count ))->getChannel (0);

    r->set_sample_offset( chunk->getInterval().first );

    TIME_STFT ComputationSynchronize();

    return r;
}


unsigned Fft::
        lChunkSizeS(unsigned x, unsigned m)
{
    return fft->lChunkSizeS(x,m);
}

unsigned Fft::
        sChunkSizeG(unsigned x, unsigned m)
{
    return fft->sChunkSizeG(x,m);
}

/// STFT

std::vector<unsigned> Stft::_ok_chunk_sizes;


Stft::
        Stft(const StftParams& p)
    :
      p(p),
      fft( FftImplementation::newInstance () )
{
}


Stft::
        Stft(const Stft& s)
:
    Transform(s),
    p(s.params())
{
}


Tfr::pChunk Stft::
        operator() (Signal::pMonoBuffer b)
{
    TIME_STFT TaskTimer ti("Stft::operator, p.chunk_size() = %d, b = %s, computeredundant = %s",
                           p.chunk_size(), b->getInterval().toString().c_str(), p.compute_redundant()?"true":"false");

    DataStorage<float>::Ptr windowedInput = prepareWindow( b->waveform_data() );

    // @see compute_redundant()
    Tfr::pChunk chunk;
    if (p.compute_redundant())
        chunk = ChunkWithRedundant(windowedInput);
    else
        chunk = ComputeChunk(windowedInput);


    if (1 != p.averaging())
    {
        unsigned width = chunk->nScales();
        unsigned height = chunk->nSamples()/p.averaging();

        Tfr::ChunkData::Ptr averagedOutput(
                new Tfr::ChunkData( height*width ));

        stftAverage( chunk->transform_data, averagedOutput, width );

        chunk->transform_data = averagedOutput;
    }

    chunk->freqAxis = p.freqAxis( b->sample_rate() );
    double increment = p.increment();
    double alignment = p.chunk_size();

    //
    // see class Chunk::first_valid_sample for a definition of these
    //

    // sample_rate = original_sample_rate/(increment*averaging)
    chunk->original_sample_rate = b->sample_rate();
    chunk->sample_rate = chunk->original_sample_rate / (increment*p.averaging());
    chunk->chunk_offset = b->sample_offset() / (increment*p.averaging());
    //chunk->chunk_offset = (b->sample_offset() + alignment/2 - increment/2) / (increment*p.averaging());
    // (note that "first_valid_sample" only makes sense if the transform is invertible, which it isn't if averaging != 1)
    chunk->first_valid_sample = ceil((alignment - increment)/increment);
    int nSamples = chunk->nSamples();
    int last_valid_sample = floor((nSamples*increment - alignment + increment)/increment);
    if ( last_valid_sample >= chunk->first_valid_sample)
        chunk->n_valid_samples = last_valid_sample - chunk->first_valid_sample;
    else
        chunk->n_valid_samples = 0;

    TEST_FT_INVERSE
    {
        Signal::pMonoBuffer breal = b;
        Signal::pMonoBuffer binv = inverse( chunk );
        float* binv_p = binv->waveform_data()->getCpuMemory();
        float* breal_p = breal->waveform_data()->getCpuMemory();
        Signal::IntervalType breal_length = breal->number_of_samples();
        Signal::IntervalType binv_length = binv->number_of_samples();
        STFT_ASSERT( breal_length == binv_length );
        float maxd = 0;
        for(Signal::IntervalType i =0; i<breal_length; i++)
        {
            float d = breal_p[i]-binv_p[i];
            if (d*d > maxd)
                maxd = d*d;
        }

        TaskInfo("Difftest %s (value %g)", maxd<1e-9*p.chunk_size() ? "passed" : "failed", maxd);
    }

    TIME_STFT TaskInfo("Stft chunk %s, %s. (%u x %u)",
                       chunk->getInterval().toString().c_str(),
                       chunk->getCoveredInterval().toString().c_str(),
                       chunk->nSamples(),
                       chunk->nScales());

    return chunk;
}


Tfr::pChunk Stft::
        ComputeChunk(DataStorage<float>::Ptr inputbuffer)
{
    STFT_ASSERT( 0!=p.chunk_size() );

    DataStorageSize actualSize(
            p.chunk_size()/2 + 1,
            inputbuffer->size().width/p.chunk_size() );

    DataStorageSize n = actualSize.width * actualSize.height;

    STFT_ASSERT (0!=actualSize.height); // not enough data

    Tfr::pChunk chunk( new Tfr::StftChunk(p.chunk_size(), p.windowType(), p.increment(), false) );
    chunk->transform_data.reset( new Tfr::ChunkData( n ));

    fft->compute( inputbuffer, chunk->transform_data, DataStorageSize(p.chunk_size(), actualSize.height) );

    TIME_STFT ComputationSynchronize();

    return chunk;
}


Tfr::pChunk Stft::
        ChunkWithRedundant(DataStorage<float>::Ptr inputbuffer)
{
    Tfr::ChunkData::Ptr input( new Tfr::ChunkData( inputbuffer->size()));
    ::stftToComplex( inputbuffer, input );

    STFT_ASSERT( 0!=p.chunk_size() );

    DataStorageSize n(
            p.chunk_size(),
            inputbuffer->size().width/p.chunk_size() );

    STFT_ASSERT (0!=n.height); // not enough data

    if (32768<n.height)
    {
        TaskInfo("%s: Reducing n.height from %d to %d", __FUNCTION__, n.height, 32768);
        n.height = 32768;
    }

    Tfr::pChunk chunk( new Tfr::StftChunk(p.chunk_size(), p.windowType(), p.increment(), true) );

    chunk->transform_data.reset( new ChunkData( n.width*n.height ));

    fft->compute( input, chunk->transform_data, n, FftDirection_Forward );

    TIME_STFT ComputationSynchronize();

    return chunk;
}


Signal::pMonoBuffer Stft::
        inverse( pChunk chunk )
{
    STFT_ASSERT( p.averaging() == 1 );

    StftChunk* stftchunk = dynamic_cast<StftChunk*>(chunk.get());
    STFT_ASSERT( stftchunk );
    if (!(0<stftchunk->n_valid_samples))
    {
        STFT_ASSERT( 0<stftchunk->n_valid_samples );
    }
    if (stftchunk->redundant())
        return inverseWithRedundant( chunk );

    ComputationCheckError();
    STFT_ASSERT( chunk->nChannels() == 1 );

    const int chunk_window_size = stftchunk->window_size();
    const int actualSize = stftchunk->nActualScales();
    int nwindows = chunk->transform_data->numberOfElements() / actualSize;

    TIME_STFT ComputationSynchronize();

    TIME_STFT
            TaskTimer ti("Stft::inverse, chunk_window_size = %d, b = %s, nwindows=%d",
                         chunk_window_size, chunk->getInterval().toString().c_str(), nwindows);

    STFT_ASSERT( 0 != chunk_window_size );

    STFT_ASSERT (0!=nwindows); // not enough data

    if (32768<nwindows)
    {
        TaskInfo("%s: Reducing n.height from %d to %d", __FUNCTION__, nwindows, 32768);
        nwindows = 32768;
    }

    const DataStorageSize n(
            chunk_window_size,
            nwindows );

    DataStorage<float>::Ptr windowedOutput(new DataStorage<float>(nwindows*chunk_window_size));

    fft->inverse( chunk->transform_data, windowedOutput, n );

    TIME_STFT ComputationSynchronize();

    {
        TIME_STFT TaskTimer ti("normalizing %u elements", n.width);
        stftNormalizeInverse( windowedOutput, n.width );
        TIME_STFT ComputationSynchronize();
    }


    // TODO normalize while reducing
    // TODO reduce and prepare in kernel
    DataStorage<float>::Ptr signal = reduceWindow( windowedOutput, stftchunk );


    Signal::pMonoBuffer b(new Signal::MonoBuffer(stftchunk->getInterval().first, signal->numberOfElements(), chunk->original_sample_rate));
    *b->waveform_data() = *signal; // this will not copy any data thanks to COW optimizations


    return b;
}


Signal::pMonoBuffer Stft::
        inverseWithRedundant( pChunk chunk )
{
    STFT_ASSERT( chunk->nChannels() == 1 );
    int
            chunk_window_size = chunk->nScales(),
            nwindows = chunk->nSamples();

    TIME_STFT ComputationSynchronize();
    TIME_STFT TaskTimer ti("Stft::inverse, chunk_window_size = %d, b = %s", chunk_window_size, chunk->getInterval().toString().c_str());

    STFT_ASSERT( 0!= chunk_window_size );

    STFT_ASSERT (0!=nwindows); // not enough data

    if (32768<nwindows)
    {
        TaskInfo("%s: Reducing n.height from %d to %d", __FUNCTION__, nwindows, 32768);
        nwindows = 32768;
    }

    DataStorageSize n(
            chunk_window_size,
            nwindows );

    Tfr::ChunkData::Ptr complexWindowedOutput( new Tfr::ChunkData(nwindows*chunk_window_size));

    fft->compute( chunk->transform_data, complexWindowedOutput, n, FftDirection_Inverse );

    TIME_STFT ComputationSynchronize();

    DataStorage<float>::Ptr windowedOutput( new DataStorage<float>(nwindows*chunk_window_size));

    {
        TIME_STFT TaskTimer ti("normalizing %u elements", n.width);
        stftNormalizeInverse( complexWindowedOutput, windowedOutput, n.width );
        TIME_STFT ComputationSynchronize();
    }


    // TODO discard imaginary part while reducing
    StftChunk*stftchunk = dynamic_cast<StftChunk*>(chunk.get());
    DataStorage<float>::Ptr signal = reduceWindow( windowedOutput, stftchunk );


    Signal::pMonoBuffer b(new Signal::MonoBuffer(stftchunk->getInterval().first, signal->numberOfElements(), chunk->original_sample_rate));
    *b->waveform_data() = *signal; // this will not copy any data thanks to COW optimizations


    return b;
}


Tfr::ComplexBuffer::Ptr Stft::
        inverseKeepComplex( pChunk chunk )
{
    STFT_ASSERT( chunk->nChannels() == 1 );
    int
            chunk_window_size = chunk->nScales(),
            nwindows = chunk->nSamples();

    TIME_STFT ComputationSynchronize();
    TIME_STFT TaskTimer ti("Stft::inverse, chunk_window_size = %d, b = %s", chunk_window_size, chunk->getInterval().toString().c_str());

    STFT_ASSERT( 0!= chunk_window_size );

    STFT_ASSERT (0!=nwindows); // not enough data

    if (32768<nwindows)
    {
        TaskInfo("%s: Reducing n.height from %d to %d", __FUNCTION__, nwindows, 32768);
        nwindows = 32768;
    }

    DataStorageSize n(
            chunk_window_size,
            nwindows );

    Tfr::ChunkData::Ptr complexWindowedOutput( new Tfr::ChunkData(nwindows*chunk_window_size));

    fft->compute( chunk->transform_data, complexWindowedOutput, n, FftDirection_Inverse );

    TIME_STFT ComputationSynchronize();

    {
        TIME_STFT TaskTimer ti("normalizing %u elements", n.width);
        stftNormalizeInverse( complexWindowedOutput, n.width );
        TIME_STFT ComputationSynchronize();
    }


    // TODO discard imaginary part while reducing
    StftChunk*stftchunk = dynamic_cast<StftChunk*>(chunk.get());
    Tfr::ChunkData::Ptr signal = reduceWindow( complexWindowedOutput, stftchunk );


    Tfr::ComplexBuffer::Ptr b(new Tfr::ComplexBuffer(stftchunk->getInterval().first, signal->numberOfElements(), chunk->original_sample_rate));
    *b->complex_waveform_data() = *signal; // this will not copy any data thanks to COW optimizations


    return b;
}



//static unsigned absdiff(unsigned a, unsigned b)
//{
//    return a < b ? b - a : a - b;
//}





void Stft::
        compute( Tfr::ChunkData::Ptr input, Tfr::ChunkData::Ptr output, FftDirection direction )
{
    int window_size = p.chunk_size();
    DataStorageSize size( window_size, input->numberOfElements()/window_size);
    TIME_STFT TaskTimer ti("Stft::compute %s, size = %d, %d",
                           direction == FftDirection_Forward ? "forward" : "inverse",
                           size.width, size.height);
    fft->compute( input, output, size, direction );
}


template<> float Stft::computeWindowValue<StftParams::WindowType_Hann>( float p )         { return 1.f  + cos(M_PI*p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_Hamming>( float p )      { return 0.54f  + 0.46f*cos(M_PI*p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_Tukey>( float p )        { return std::fabs(p) < 0.5 ? 2.f : 1.f + cos(M_PI*(std::fabs(p)*2.f-1.f)); }
template<> float Stft::computeWindowValue<StftParams::WindowType_Cosine>( float p )       { return cos(M_PI*p*0.5f); }
template<> float Stft::computeWindowValue<StftParams::WindowType_Lanczos>( float p )      { return p==0?1.f:sin(M_PI*p)/(M_PI*p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_Triangular>( float p )   { return 1.f - fabs(p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_Gaussian>( float p )     { return exp2f(-6.492127684f*p*p); } // sigma = 1/3
template<> float Stft::computeWindowValue<StftParams::WindowType_BarlettHann>( float p )  { return 0.62f-0.24f*fabs(p)+0.38f*cos(M_PI*p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_Blackman>( float p )     { return 0.42f + 0.5f*cos(M_PI*p) + 0.08f*cos(2.f*M_PI*p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_Nuttail>( float p )      { return 0.355768f + 0.487396f*cos(M_PI*p) + 0.144232f*cos(2.f*M_PI*p) + 0.012604f*cos(3.f*M_PI*p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_BlackmanHarris>( float p )  { return 0.35875f + 0.48829*cos(M_PI*p) + 0.14128f*cos(2.f*M_PI*p) + 0.01168f*cos(3.f*M_PI*p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_BlackmanNuttail>( float p ) { return 0.3635819f + 0.4891775*cos(M_PI*p) + 0.1365995f*cos(2.f*M_PI*p) + 0.0106411f*cos(3.f*M_PI*p); }
template<> float Stft::computeWindowValue<StftParams::WindowType_FlatTop>( float p ) { return 1.f + 1.93f*cos(M_PI*p) + 1.29f*cos(2.f*M_PI*p) + 0.388f*cos(3.f*M_PI*p) + 0.032f*cos(4.f*M_PI*p); }
template<StftParams::WindowType> float Stft::computeWindowValue( float )                  { return 1.f; }


template<StftParams::WindowType Type>
void Stft::
        prepareWindowKernel( DataStorage<float>::Ptr source, DataStorage<float>::Ptr windowedData )
{
    unsigned increment = p.increment();
    int windowCount = windowedData->size().width/p.chunk_size();

    std::vector<float> windowfunction(p.chunk_size());
    float* window = &windowfunction[0];
    float norm = 0;
    int window_size = p.chunk_size();
    if (StftParams::applyWindowOnInverse(Type))
    {
        for (int x=0;x<window_size; ++x)
        {
            float p = 2.f*(x+1)/(window_size+1) - 1.f;
            float a = computeWindowValue<Type>(p);
            norm += a*a;
            window[x] = a;
        }
        norm = sqrt(p.chunk_size() / norm);
    }
    else
    {
        for (int x=0;x<window_size; ++x)
        {
            float p = 2.f*(x+1)/(window_size+1) - 1.f;
            float a = computeWindowValue<Type>(p);
            norm += a;
            window[x] = a;
        }
        norm = p.chunk_size() / norm;
    }

    CpuMemoryReadOnly<float, 3> in = CpuMemoryStorage::ReadOnly<3>(source);
    CpuMemoryWriteOnly<float, 3> out = CpuMemoryStorage::WriteAll<3>(windowedData);
    CpuMemoryWriteOnly<float, 3>::Position pos(0,0,0);

    STFT_ASSERT(1 == source->size().height);
    STFT_ASSERT(1 == source->size().depth);

    for (pos.z=0; pos.z<source->size().depth; ++pos.z)
    {
        for (pos.y=0; pos.y<source->size().height; ++pos.y)
        {
            if (pos.z + 1 == source->size().depth && pos.y + 1 == source->size().height)
            {
                long in0 = &in.ref(pos) - in.ptr();
                long in1 = &in.ref(pos) + windowCount*increment - in.ptr();
                long in2 = source->numberOfElements();
                if (!(&out.ref(pos) + windowCount*p.chunk_size() - out.ptr() == (long)windowedData->numberOfElements()) || !(in1 < in2))
                {
                    TaskInfo("in0 %dl", in0);
                    TaskInfo("in1 %dl", in1);
                    TaskInfo("in2 %dl", in2);
                    STFT_ASSERT(&out.ref(pos) + windowCount*p.chunk_size() - out.ptr() == (long)windowedData->numberOfElements());
                    STFT_ASSERT(in1 < in2);
                }
            }

#pragma omp parallel for
            for (int w=0; w<windowCount; ++w)
            {
                float *o = &out.ref(pos) + w*p.chunk_size();
                float *i = &in.ref(pos) + w*increment;

                for (int x=0; x<p.chunk_size(); ++x)
                    o[x] = window[x] * i[x] * norm;
            }
        }
    }
}


template<StftParams::WindowType Type, typename T>
void Stft::
        reduceWindowKernel( boost::shared_ptr<DataStorage<T> > windowedSignal, typename DataStorage<T>::Ptr signal, const StftChunk* c )
{
    int increment = c->increment();
    int window_size = c->window_size();
    int windowCount = windowedSignal->size().width/window_size;
    float normalizeOverlap = increment/(float)window_size;
    float normalizeFft = 1.f; // 1.f/window_size;, todo normalize here while going through the data anyways
    float normalize = normalizeFft*normalizeOverlap;

    CpuMemoryReadOnly<T, 3> in = CpuMemoryStorage::ReadOnly<3>(windowedSignal);
    CpuMemoryWriteOnly<T, 3> out = CpuMemoryStorage::WriteAll<3>(signal);

    typename CpuMemoryWriteOnly<T, 3>::Position pos(0,0,0);

    std::vector<float> windowfunction(window_size);
    float* window = &windowfunction[0];

    float norm = 0;

    if (StftParams::applyWindowOnInverse(Type))
    {
        for (int x=0;x<window_size; ++x)
        {
            float p = 2.f*(x+1)/(window_size+1) - 1.f;
            float a = computeWindowValue<Type>(p);
            norm += a*a;
            window[x] = normalize*a;
        }
        norm = sqrt(p.chunk_size() / norm);
    }
    else
    {
        for (int x=0;x<window_size; ++x)
            window[x] = normalize;

        norm = 1;
    }


    int out0 = c->first_valid_sample*increment;
    //int out0 = p.chunk_size()/2 - increment/2 + c->first_valid_sample*increment;
    int N = signal->size().width;

    TaskInfo("signal->size().width = %u", signal->size().width);

    STFT_ASSERT( c->n_valid_samples*increment == signal->size().width );

    for (pos.z=0; pos.z<windowedSignal->size().depth; ++pos.z)
    {
        for (pos.y=0; pos.y<windowedSignal->size().height; ++pos.y)
        {
            T *o = &out.ref(pos);
            for (int x=0; x<increment; ++x)
                if (x>=out0 && x<N+out0) o[x-out0] = 0;
            for (int x=0; x<signal->size().width; ++x)
                o[x] = 0;

// TODO figure out how to parallelize this... subsequent iterations of 'w' access overlapping regions of o which might work and might fail, depending on timing issues
//#pragma omp parallel for
            for (int w=0; w<windowCount; ++w)
            {
                T *o = &out.ref(pos);
                T *i = &in.ref(pos) + w*window_size;

                int x0 = w*increment;
                int x=0;
                for (; x<window_size; ++x)
                    //if (x0+x>=out0 && x0+x<N+out0) o[x0+x-out0] += i[x] * normalize;
                    if (x0+x>=out0 && x0+x<N+out0) o[x0+x-out0] += i[x] * (window[x] * norm);

                //for (; x<window_size-increment; ++x)
                    //if (x0+x>=out0 && x0+x<N+out0) o[x0+x-out0] += window[x] * i[x] * norm;
                //for (; x<window_size; ++x)
                    //if (x0+x>=out0 && x0+x<N+out0) o[x0+x-out0] = window[x] * i[x] * norm;
            }
        }
    }
}


DataStorage<float>::Ptr Stft::
        prepareWindow( DataStorage<float>::Ptr source )
{
    if (p.windowType() == StftParams::WindowType_Rectangular && p.overlap() == 0.f )
        return source;

    if (source->size().width < p.chunk_size())
        throw std::logic_error("Stft not enough data for window function");

    unsigned increment = p.increment();
    unsigned windowCount = 1 + (source->size().width-p.chunk_size()) / increment; // round down

    DataStorage<float>::Ptr windowedData(new DataStorage<float>(windowCount*p.chunk_size(), source->size().height, source->size().depth ));

    switch(p.windowType())
    {
    case StftParams::WindowType_Hann:
        prepareWindowKernel<StftParams::WindowType_Hann>(source, windowedData);
        break;
    case StftParams::WindowType_Hamming:
        prepareWindowKernel<StftParams::WindowType_Hamming>(source, windowedData);
        break;
    case StftParams::WindowType_Tukey:
        prepareWindowKernel<StftParams::WindowType_Tukey>(source, windowedData);
        break;
    case StftParams::WindowType_Cosine:
        prepareWindowKernel<StftParams::WindowType_Cosine>(source, windowedData);
        break;
    case StftParams::WindowType_Lanczos:
        prepareWindowKernel<StftParams::WindowType_Lanczos>(source, windowedData);
        break;
    case StftParams::WindowType_Triangular:
        prepareWindowKernel<StftParams::WindowType_Triangular>(source, windowedData);
        break;
    case StftParams::WindowType_Gaussian:
        prepareWindowKernel<StftParams::WindowType_Gaussian>(source, windowedData);
        break;
    case StftParams::WindowType_BarlettHann:
        prepareWindowKernel<StftParams::WindowType_BarlettHann>(source, windowedData);
        break;
    case StftParams::WindowType_Blackman:
        prepareWindowKernel<StftParams::WindowType_Blackman>(source, windowedData);
        break;
    case StftParams::WindowType_Nuttail:
        prepareWindowKernel<StftParams::WindowType_Nuttail>(source, windowedData);
        break;
    case StftParams::WindowType_BlackmanHarris:
        prepareWindowKernel<StftParams::WindowType_BlackmanHarris>(source, windowedData);
        break;
    case StftParams::WindowType_BlackmanNuttail:
        prepareWindowKernel<StftParams::WindowType_BlackmanNuttail>(source, windowedData);
        break;
    case StftParams::WindowType_FlatTop:
        prepareWindowKernel<StftParams::WindowType_FlatTop>(source, windowedData);
        break;
    default:
        prepareWindowKernel<StftParams::WindowType_Rectangular>(source, windowedData);
        break;
    }


    return windowedData;
}


template<typename T>
typename DataStorage<T>::Ptr Stft::
        reduceWindow( boost::shared_ptr<DataStorage<T> > windowedSignal, const StftChunk* c )
{
    if (c->window_type() == StftParams::WindowType_Rectangular && c->increment() == c->window_size() )
        return windowedSignal;

    unsigned increment = c->increment();
    unsigned window_size = c->window_size();
    unsigned windowCount = windowedSignal->size().width / window_size;
    STFT_ASSERT( int(windowCount*c->window_size()) == windowedSignal->size().width );


    unsigned L = c->n_valid_samples*increment;
    typename DataStorage<T>::Ptr signal(new DataStorage<T>( L ));

    switch(c->window_type())
    {
    case StftParams::WindowType_Hann:
        reduceWindowKernel<StftParams::WindowType_Hann>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_Hamming:
        reduceWindowKernel<StftParams::WindowType_Hamming>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_Tukey:
        reduceWindowKernel<StftParams::WindowType_Tukey>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_Cosine:
        reduceWindowKernel<StftParams::WindowType_Cosine>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_Lanczos:
        reduceWindowKernel<StftParams::WindowType_Lanczos>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_Triangular:
        reduceWindowKernel<StftParams::WindowType_Triangular>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_Gaussian:
        reduceWindowKernel<StftParams::WindowType_Gaussian>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_BarlettHann:
        reduceWindowKernel<StftParams::WindowType_BarlettHann>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_Blackman:
        reduceWindowKernel<StftParams::WindowType_Blackman>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_Nuttail:
        reduceWindowKernel<StftParams::WindowType_Nuttail>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_BlackmanHarris:
        reduceWindowKernel<StftParams::WindowType_BlackmanHarris>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_BlackmanNuttail:
        reduceWindowKernel<StftParams::WindowType_BlackmanNuttail>(windowedSignal, signal, c);
        break;
    case StftParams::WindowType_FlatTop:
        reduceWindowKernel<StftParams::WindowType_FlatTop>(windowedSignal, signal, c);
        break;
    default:
        reduceWindowKernel<StftParams::WindowType_Rectangular>(windowedSignal, signal, c);
        break;
    }

    return signal;
}


unsigned Stft::
        build_performance_statistics(bool writeOutput, float size_of_test_signal_in_seconds)
{
    _ok_chunk_sizes.clear();
    scoped_ptr<TaskTimer> tt;
    Tfr::StftParams p;
#ifdef USE_CUFFT
    if(writeOutput) tt.reset( new TaskTimer("Building STFT performance statistics for %s", CudaProperties::getCudaDeviceProp().name));
#else
    if(writeOutput) tt.reset( new TaskTimer("Building STFT performance statistics for %s", "Cpu"));
#endif
    Signal::pMonoBuffer B = boost::shared_ptr<Signal::MonoBuffer>( new Signal::MonoBuffer( 0, 44100*size_of_test_signal_in_seconds, 44100 ) );
    {
        scoped_ptr<TaskTimer> tt;
        if(writeOutput) tt.reset( new TaskTimer("Filling test buffer with random data (%.1f kB or %.1f s with fs=44100)", B->number_of_samples()*sizeof(float)/1024.f, size_of_test_signal_in_seconds));

        float* p = B->waveform_data()->getCpuMemory();
        for (int i = 0; i < B->number_of_samples(); i++)
            p[i] = rand() / (float)RAND_MAX;
    }

    time_duration fastest_time;
    unsigned fastest_size = 0;
    unsigned ok_size = 0;
    Tfr::pChunk C;
    time_duration latest_time[4];
    unsigned max_base = 3;
    //double base[] = {2,3,5,7};
    double base[] = {2};
    for (int n = 128; n < B->number_of_samples(); n++ )
    {
        unsigned N = -1;
        unsigned selectedBase = 0;

        for (unsigned b=0; b<sizeof(base)/sizeof(base[0]) && b<=max_base; b++)
        {
            double x = ceil(log((double)n)/log(base[b]));
            unsigned N2 = pow(base[b], x);

            if (N2<N)
            {
                selectedBase = b;
                N = N2;
            }
        }

        n = N;

        p.set_exact_chunk_size( N );
        Tfr::Stft S(p);

        {
            scoped_ptr<TaskTimer> tt;
            if(writeOutput) tt.reset( new TaskTimer( "n=%u, _chunk_size = %u = %g ^ %g",
                                                     n, S.p.chunk_size(),
                                                     base[selectedBase],
                                                     log2f((float)S.p.chunk_size())/log2f(base[selectedBase])));

            ptime startTime = microsec_clock::local_time();

            C = S( B );

            if (!C)
                continue;

            time_duration diff = microsec_clock::local_time() - startTime;

            if (0<selectedBase)
                if (diff.total_microseconds() > latest_time[0].total_microseconds()*1.5)
                {
                    max_base = selectedBase-1;
                    if(tt) tt->info("max_base = %u", max_base);
                }

            latest_time[selectedBase] = diff;

            if (diff.total_milliseconds() < fastest_time.total_milliseconds()*1.2)
                ok_size = S.p.chunk_size();

            if (diff < fastest_time || 0==fastest_size)
            {
                max_base = sizeof(base)/sizeof(base[0]) - 1;
                fastest_time = diff;
                fastest_size = S.p.chunk_size();
            }

            _ok_chunk_sizes.push_back( S.p.chunk_size() );
        }
        C.reset();

        if (S.p.chunk_size() > B->number_of_samples())
            break;
    }

    if(writeOutput) TaskInfo("Fastest size = %u", fastest_size);
    return fastest_size;
}


StftChunk::
        StftChunk(unsigned window_size, StftParams::WindowType window_type, unsigned increment, bool redundant)
            :
            Chunk( Order_column_major ),
            _window_type( window_type ),
            _increment( increment ),
            _halfs_n( 0 ),
            _window_size( window_size ),
            _redundant( redundant )
{}


void StftChunk::
        setHalfs( unsigned n )
{
    sample_rate /= 1<<_halfs_n;

    _halfs_n = n;

    sample_rate *= 1<<_halfs_n;
}


unsigned StftChunk::
        halfs( )
{
    return _halfs_n;
}


unsigned StftChunk::
        nActualScales() const
{
    if (_redundant)
        return _window_size;
    return _window_size/2 + 1;
}


unsigned StftChunk::
        nSamples() const
{
    return transform_data->size().width / nActualScales();
}


unsigned StftChunk::
        nScales() const
{
    unsigned s = transformSize();
    if (_redundant)
        return s;
    else
        return s/2 + 1;
}


unsigned StftChunk::
        transformSize() const
{
    return _window_size >> _halfs_n;
}


Signal::Interval StftChunk::
        getCoveredInterval() const
{
    double scale = original_sample_rate/sample_rate;
    unsigned n = nSamples();

    Signal::Interval I(
            std::floor(chunk_offset.asFloat() * scale + _window_size/2.0 + 0.5),
            std::floor((chunk_offset + n - 1.0).asFloat() * scale + _window_size/2.0 + 0.5)
    );
    if (I.first == I.last)
        ++I.last;

    return I;
}


} // namespace Tfr
