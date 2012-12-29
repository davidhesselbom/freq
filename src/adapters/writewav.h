#ifndef ADAPTERS_WRITEWAV_H
#define ADAPTERS_WRITEWAV_H

#include "signal/sink.h"

class SndfileHandle;

namespace Adapters {

class SaweDll WriteWav: public Signal::Sink
{
public:
    WriteWav( std::string filename );
    ~WriteWav();

    // Overloaded from Sink
    virtual void put( Signal::pBuffer );
    virtual void invalidate_samples( const Signal::Intervals& s );
    virtual Signal::Intervals invalid_samples();
    virtual bool deleteMe();

    void reset();
    bool normalize() { return _normalize; }
    void normalize(bool v);

    static void writeToDisk(std::string filename, Signal::pBuffer b, bool normalize = true);

    /// @overload Operation::read()
    virtual Signal::pBuffer read(const Signal::Interval& I);

private:
    std::string _filename;
    bool _normalize;
    boost::shared_ptr<SndfileHandle> _sndfile;

    Signal::Intervals _invalid_samples;
    long double _sum;
    float _high, _low;
    Signal::IntervalType _sumsamples;
    Signal::IntervalType _offset;

    void appendBuffer(Signal::pBuffer b);
    void rewriteNormalized();
};

} // namespace Adapters

#endif // ADAPTERS_WRITEWAV_H
