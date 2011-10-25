#ifndef ADAPTERS_WRITEWAV_H
#define ADAPTERS_WRITEWAV_H

#include "signal/sinksourcechannels.h"

namespace Adapters {

class WriteWav: public Signal::Sink
{
public:
    WriteWav( std::string filename );
    ~WriteWav();

    // Overloaded from Sink
    virtual void put( Signal::pBuffer b, Signal::pOperation ) { put (b); }
    virtual void reset();
    virtual void invalidate_samples( const Signal::Intervals& s );
    virtual void set_channel(unsigned c);
    virtual Signal::Intervals invalid_samples();
    virtual bool deleteMe();

    bool normalize() { return _normalize; }
    void normalize(bool v);

    static void writeToDisk(std::string filename, Signal::pBuffer b, bool normalize = true);

    void put( Signal::pBuffer );

    static Signal::pBuffer crop(Signal::pBuffer b);

private:
    Signal::SinkSourceChannels _data;
    std::string _filename;
    bool _normalize;

    void writeToDisk();
};

} // namespace Adapters

#endif // ADAPTERS_WRITEWAV_H
