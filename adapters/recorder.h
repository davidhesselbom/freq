#ifndef RECORDER_H
#define RECORDER_H

#include <QMutex>

#include "signal/sinksourcechannels.h"
#include "signal/postsink.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Adapters {

class Recorder: public Signal::FinalSource
{
public:
    Recorder();
    ~Recorder();

    virtual void startRecording() = 0;
    virtual void stopRecording() = 0;
    virtual bool isStopped() = 0;
    virtual bool canRecord() = 0;

    float time_since_last_update();
    Signal::PostSink* getPostSink() { return &_postsink; }
    Signal::SinkSourceChannels& data() { return _data; }

    // virtual from Signal::FinalSource
    // virtual std::string name() = 0;
    // virtual float sample_rate() = 0;

    // overloaded from Signal::FinalSource
    virtual Signal::pBuffer read( const Signal::Interval& I );
    virtual long unsigned number_of_samples();
    virtual unsigned num_channels();
    virtual void set_channel(unsigned c);
    virtual unsigned get_channel();
    virtual float length();

protected:
    QMutex _data_lock;
    Signal::SinkSourceChannels _data;
    Signal::PostSink _postsink;
    float _offset;
    boost::posix_time::ptime _start_recording, _last_update;
    long unsigned actual_number_of_samples();

private:
    float time();
};

}

#endif // RECORDER_H
