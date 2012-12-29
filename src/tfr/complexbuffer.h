#ifndef TFR_COMPLEXBUFFER_H
#define TFR_COMPLEXBUFFER_H

#include <complex>

#include "signal/source.h"

#include <boost/scoped_ptr.hpp>

namespace Tfr
{
class ComplexBuffer
{
public:
    typedef boost::shared_ptr<ComplexBuffer> Ptr;


    ComplexBuffer(UnsignedF firstSample,
           unsigned long numberOfSamples,
           float FS,
           unsigned numberOfChannels=1);

    /**
        Create a complex waveform out of a real waveform.
    */
    ComplexBuffer(const Signal::MonoBuffer& b);

    ComplexBuffer(DataStorage<float>::Ptr inputbuffer);


    /**
        Really inefficient, don't do this. Will recompute get_real for each
        call. Instead, call get_real(), store that pBuffer and then call
        waveform_data().

        The pointer is valid for the lifetime of this class, or as long as the
        pBuffer returned from get_real() isn't deleted.
    */
    DataStorage<float>::Ptr waveform_data();


    /**
        Overloaded from buffer
    */
    int number_of_samples() const { return _complex_waveform_data->size().width; }


    UnsignedF       sample_offset;
    float           sample_rate;

    /**
        Used to convert back to real data, will discard imaginary part.
    */
    Signal::pMonoBuffer get_real();


    /**
        Access the complex waveform
    */
    DataStorage<std::complex<float> >::Ptr complex_waveform_data() const {
        return _complex_waveform_data;
    }

protected:
    Signal::pMonoBuffer _my_real;

    void setData(DataStorage<float>::Ptr inputbuffer);
    DataStorage<std::complex<float> >::Ptr _complex_waveform_data;
};

} // namespace Tfr
#endif // TFR_COMPLEXBUFFER_H
