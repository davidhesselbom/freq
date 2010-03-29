#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "signal-source.h"
#include "signal-sink.h"
#include "transform-inverse.h"
#include <boost/shared_ptr.hpp>
#include <map>

typedef unsigned int cufftHandle; /* from cufft.h */

typedef boost::shared_ptr<class Transform> pTransform;

class Transform
{
public:
    typedef boost::shared_ptr<class Inverse> pInverse;

    typedef unsigned ChunkIndex;

    Transform( Signal::pSource source,
               unsigned channel,
               unsigned samples_per_chunk,
               unsigned scales_per_octave,
               float wavelet_std_t );
    ~Transform();

    ChunkIndex             getChunkIndex( unsigned including_sample );
    pTransform_chunk       getChunk( ChunkIndex n, cudaStream_t stream=0 );

    /* discard cached data, releases all GPU memory */
    void     gc();

    /* properties, altering ANY of these is equivalent to creating a new
       instance of class Transform */
    unsigned  channel() const { return _channel; }
    void      channel( unsigned );
    unsigned  scales_per_octave() const { return _scales_per_octave; }
    void      scales_per_octave( unsigned );
    unsigned  samples_per_chunk() const { return _samples_per_chunk; }
    void      samples_per_chunk( unsigned );
    float     wavelet_std_t() const { return _wavelet_std_samples/(float)_original_waveform->sample_rate(); }
    void      wavelet_std_t( float );
    Signal::pSource original_waveform() { return _original_waveform; }
    void      original_waveform( Signal::pSource );
    float     number_of_octaves() const;
    unsigned  nScales() { return number_of_octaves() * scales_per_octave(); }
    float     min_hz() const { return _min_hz; }
    void      min_hz(float f);
    float     max_hz() const { return _max_hz; }
    void      max_hz(float f);
    pTransform_chunk previous_chunk( unsigned &out_chunk_index );

    boost::shared_ptr<Transform_inverse> inverse();

private:
#ifdef _USE_CHUNK_CACHE
    pTransform_chunk allocateChunk( ChunkIndex n );
    pTransform_chunk releaseChunkFurthestAwayFrom( ChunkIndex n );
    void             clampTransform( pTransform_chunk out_chunk, pTransform_chunk in_transform, cudaStream_t stream );
#endif // #ifdef _USE_CHUNK_CACHE
    pTransform_chunk computeTransform( Signal::pBuffer chunk, cudaStream_t stream );

    /* caches */
#ifdef _USE_CHUNK_CACHE
    typedef std::map<ChunkIndex, pTransform_chunk> ChunkMap;
    ChunkMap                                _oldChunks;
#endif // #ifdef _USE_CHUNK_CACHE
    pTransform_chunk                        _intermediate_wt;
    boost::shared_ptr<GpuCpuData<float2> >  _intermediate_ft;

    /* property values */
    Signal::pSource _original_waveform;
    unsigned  _channel;
    unsigned  _scales_per_octave;
    unsigned  _samples_per_chunk;
    unsigned  _wavelet_std_samples;
    float     _min_hz;
    float     _max_hz;
    cufftHandle _fft_many;
    cufftHandle _fft_single;
    unsigned _fft_width;
};

#endif // TRANSFORM_H
