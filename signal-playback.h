#ifndef SIGNALPLAYBACK_H
#define SIGNALPLAYBACK_H

#include "signal-sink.h"
#include "signal-samplesintervaldescriptor.h"
#include <vector>
#include <time.h>
#include <QMutex>
#include <portaudiocpp/PortAudioCpp.hxx>
#include <boost/scoped_ptr.hpp>

namespace Signal {

class Playback: virtual public Sink
{
public:
    Playback( int outputDevice/* = -1 */);
    ~Playback();

    virtual void put( pBuffer );
    virtual void reset();

    SamplesIntervalDescriptor getMissingSamples();
    static void list_devices();
    unsigned    playback_itr();
    float       time();
    float       outputLatency();
    pBuffer     first_buffer();
    unsigned    output_device() { return _output_device; }
    bool        isStopped();
    bool        isUnderfed();
    void        preparePlayback( unsigned firstSample, unsigned number_of_samples );

private:
    QMutex _cache_lock;

    struct BufferSlot {
        pBuffer buffer;
        clock_t timestamp;
    };

    int readBuffer(const void */*inputBuffer*/,
                     void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo *timeInfo,
                     PaStreamCallbackFlags statusFlags);

    portaudio::AutoSystem _autoSys;
    boost::scoped_ptr<portaudio::MemFunCallbackStream<Playback> > streamPlayback;

    std::vector<BufferSlot> _cache;
    unsigned _playback_itr;
    unsigned _first_invalid_sample;
    int _output_device;

    unsigned nAccumulatedSamples();
};

} // namespace Signal

#endif // SIGNALPLAYBACK_H
