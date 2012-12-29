#ifndef TFRCWT_H
#define TFRCWT_H

#include "transform.h"
#include "signal/buffer.h"
#include "fftimplementation.h"

#include <map>

namespace Tfr {

class CwtChunk;
class CwtChunkPart;

class Cwt:public Transform, public TransformParams
{
public:
    /**
      TODO: Update and clarify
    Cwt initializes asynchronous computation if 0!=stream (inherited
    behaviour from cuda). Also note, from NVIDIA CUDA Programming Guide section
    3.2.6.1, v2.3 Stream:

    "Two commands from different streams cannot run concurrently if either a page-
    locked host memory allocation, a device memory allocation, a device memory set,
    a device ↔ device memory copy, or any CUDA command to stream 0 is called in-
    between them by the host thread."

    That is, a call to Cwt shall never result in any of the operations
    mentioned above, which basically leaves kernel execution and page-locked
    host ↔ device memory copy. For this purpose, a page-locked chunk of memory
    could be allocated beforehand and reserved for the Signal::pBuffer. Previous
    copies with the page-locked chunk is synchronized at beforehand.
    */
    Cwt( float scales_per_octave=20, float wavelet_time_suppport=3 );

    virtual pChunk operator()( Signal::pMonoBuffer );
    virtual Signal::pMonoBuffer inverse( pChunk );
    virtual const TransformParams* transformParams() const { return this; }

    virtual pTransform createTransform() const;
    virtual float displayedTimeResolution( float FS, float hz ) const;
    virtual FreqAxis freqAxis( float FS ) const;
    //virtual Signal::Interval validLength(Signal::pBuffer buffer);
    virtual bool operator==(const TransformParams& b) const;


    float     get_min_hz(float fs) const;
    float     wanted_min_hz() const;
    void      set_wanted_min_hz(float f);
    /// returns the nyquist frequency
    float     get_max_hz(float sample_rate) const { return sample_rate/2.f; }
    unsigned  nScales(float FS) const;
    unsigned  nBins(float fs) const;
    float     scales_per_octave() const { return _scales_per_octave; }
    void      scales_per_octave( float, float fs=0 );
    float     tf_resolution() const { return _tf_resolution; }
    void      tf_resolution( float );
    float     sigma() const;

    /**
      wavelet_time_support is the size of overlapping required in the windowed
      cwt, measured in number of wavelet sigmas. How many samples this
      corresponds to can be computed by calling wavelet_time_support_samples.
      @def 3
      */
    float     wavelet_default_time_support() const { return _wavelet_def_time_suppport; }
    float     wavelet_time_support() const { return _wavelet_time_suppport; }
    void      wavelet_time_support( float value ) { _wavelet_time_suppport = value; _wavelet_def_time_suppport = value; }
    void      wavelet_fast_time_support( float value ) { _wavelet_time_suppport = value; }
    float     wavelet_scale_support() const { return _wavelet_scale_suppport; }
    void      wavelet_scale_support( float value ) { _wavelet_scale_suppport = value; }

    /**
      Computes the standard deviation in time and frequency using the tf_resolution value. For a given frequency.
      */
    float     morlet_sigma_samples( float sample_rate, float hz ) const;
    float     morlet_sigma_f( float hz ) const;

    /**
      Provided so that clients can compute 'good' overlapping sizes. This gives the number of samples that
      wavelet_std_t corresponds to with a given sample rate, aligned to a multiple of 32 to provide for
      faster inverse calculations later on.
      */
    unsigned  wavelet_time_support_samples( float sample_rate ) const;
    unsigned  wavelet_time_support_samples( float sample_rate, float hz ) const;

    /**
      The Cwt will be computed in chunks who are powers of two. Given sample rate and wavelet_std_t,
      compute a good number of valid samples per chunk.
      */
    virtual unsigned  next_good_size( unsigned current_valid_samples_per_chunk, float sample_rate ) const;
    unsigned  prev_good_size_gold( unsigned current_valid_samples_per_chunk, float sample_rate );
    virtual unsigned  prev_good_size( unsigned current_valid_samples_per_chunk, float sample_rate ) const;
    virtual std::string toString() const;

    void      largest_scales_per_octave( float fs, float scales, float last_ok = 0 );
    bool      is_small_enough( float fs ) const;
    size_t    required_gpu_bytes(unsigned valid_samples_per_chunk, float sample_rate) const;

    unsigned        chunk_alignment(float fs) const;
private:
    pChunk          computeChunkPart( pChunk ft, unsigned first_scale, unsigned n_scales );

    unsigned        find_bin( unsigned j ) const;
    unsigned        time_support_bin0( float fs ) const;
    float           j_to_hz( float sample_rate, unsigned j ) const;
    unsigned        hz_to_j( float sample_rate, float hz ) const;
    unsigned        required_length( unsigned current_valid_samples_per_chunk, float fs, unsigned&r ) const;
    void            scales_per_octave_internal( float );
    unsigned        chunkpart_alignment(unsigned c) const;

    Signal::pMonoBuffer inverse( Tfr::CwtChunk* );
    Signal::pMonoBuffer inverse( Tfr::CwtChunkPart* );

    float           _min_hz;
    float           _scales_per_octave;
    float           _tf_resolution;
    float           _least_meaningful_fraction_of_r;
    unsigned        _least_meaningful_samples_per_chunk;

    std::map<int, Tfr::FftImplementation::Ptr> fft_instances;
    std::set<int> fft_usage; // remove any that wasn't recently used
    Tfr::FftImplementation::Ptr fft() const;
    Tfr::FftImplementation::Ptr fft(int width); // width=0 -> don't care
    void clearFft();

    /**
      Default value: _wavelet_time_suppport=3.
      @see wavelet_time_suppport
      */
    float  _wavelet_time_suppport;
    float  _wavelet_def_time_suppport;
    float _wavelet_scale_suppport;
    float _jibberish_normalization;
};

} // namespace Tfr

#endif // TFRCWT_H
