#include "playback.h"

#include "cpumemorystorage.h"
#include "tasktimer.h"

#include <iostream>
#include <stdexcept>
#include <QMessageBox>

//#define TIME_PLAYBACK
#define TIME_PLAYBACK if(0)

using namespace std;
using namespace boost::posix_time;
using namespace boost;

namespace Adapters {

bool Playback_logging = true;

Playback::
        Playback( int outputDevice )
:   _data(),
    _first_buffer_size(0),
    _playback_itr(0),
    _output_device(-1),
    _output_channels(0),
    _is_interleaved(false)
{
    portaudio::AutoSystem autoSys;
    portaudio::System &sys = portaudio::System::instance();

    static bool first = true;
    boost::shared_ptr<TaskInfo> ti;
    if (first) {
        if (Playback_logging) {
            ti.reset (new TaskInfo(format("Creating audio Playback. Requested device: %d") % outputDevice));
            list_devices();
        }

        bool _has_output_device = false;
        for (int i=0; i < sys.deviceCount(); ++i)
        {
            if (!sys.deviceByIndex(i).isInputOnlyDevice ())
                _has_output_device = true;
        }
        if (!_has_output_device) {
            TaskInfo("System didn't report any output devices. Can't play sound.");
            // leave _output_device as -1
            return;
        }
    }


    if (0>outputDevice) {
        _output_device = sys.defaultOutputDevice().index();
    } else if (outputDevice >= sys.deviceCount ()) {
        _output_device = sys.defaultOutputDevice().index();
        if (Playback_logging)
            TaskInfo(format("Highest valid device id is %d. Reverting to default output device") % (sys.deviceCount() - 1));
    } else if ( sys.deviceByIndex(outputDevice).isInputOnlyDevice() ) {
        _output_device = sys.defaultOutputDevice().index();
        if (Playback_logging)
            TaskInfo(format("Requested audio device (%d) '%s' can only be used for input. Reverting to default output device")
                         % outputDevice % sys.deviceByIndex(outputDevice).name());
    } else {
        _output_device = outputDevice;
    }

    if(first)
    {
        if (Playback_logging)
            TaskInfo tt(format("Using device '%s' (%d) for audio output")
                           % sys.deviceByIndex(_output_device).name()
                           % _output_device);
    }

    reset();
    // first = false;
}


Playback::
        ~Playback()
{
    if (streamPlayback)
    {
        if (!streamPlayback->isStopped())
            streamPlayback->stop();
        if (streamPlayback->isOpen())
            streamPlayback->close();
    }
}


/*static*/ void Playback::
        list_devices()
{
    portaudio::AutoSystem autoSys;
    portaudio::System &sys = portaudio::System::instance();

    int 	iNumDevices 		= sys.deviceCount();
    int 	iIndex 				= 0;
    string	strDetails			= "";

    TaskTimer tt("Enumerating sound devices (count %d)", iNumDevices);
    for (portaudio::System::DeviceIterator i = sys.devicesBegin(); i != sys.devicesEnd(); ++i)
    {
        strDetails = "";
        if ((*i).isSystemDefaultInputDevice())
                strDetails += ", default input";
        if ((*i).isSystemDefaultOutputDevice())
                strDetails += ", default output";

        tt.info( "%d: %s, in=%d out=%d, %s%s",
                 (*i).index(), (*i).name(),
                 (*i).maxInputChannels(),
                 (*i).maxOutputChannels(),
                 (*i).hostApi().name(),
                 strDetails.c_str());

        iIndex++;
    }
}


/*static*/ std::list<Playback::DeviceInfo> Playback::
        get_devices()
{
    // initialize enumeration of devices if it hasn't been done already
    portaudio::AutoSystem autoSys;

    portaudio::System &sys = portaudio::System::instance();

    std::list<DeviceInfo> devices;

    for (portaudio::System::DeviceIterator i = sys.devicesBegin(); i != sys.devicesEnd(); ++i)
    {
        DeviceInfo d;
        d.name = (*i).name();
        d.name2 = (*i).hostApi().name();
        d.inputChannels = (*i).maxInputChannels();
        d.outputChannels = (*i).maxOutputChannels();
        d.isDefaultIn = (*i).isSystemDefaultInputDevice();
        d.isDefaultOut = (*i).isSystemDefaultOutputDevice();
        d.index = (*i).index();

        std::string strDetails = (*i).hostApi().name();
        if ((*i).isSystemDefaultInputDevice())
               strDetails += ", default input";
        if ((*i).isSystemDefaultOutputDevice())
               strDetails += ", default output";
        d.name2 = strDetails;

        devices.push_back( d );
    }

    return devices;
}


unsigned Playback::
        playback_itr()
{
    return _playback_itr;
}


float Playback::
        time()
{
    if (_data.empty())
        return 0.f;

    // if !isPaused() the stream has started, but it hasn't read anything yet if this holds, and _startPlay_timestamp is not set
    if (isPaused() || _playback_itr == _data.spannedInterval ().first )
    {
        return _playback_itr / sample_rate();
    }

    // streamPlayback->time() doesn't seem to work (ubuntu 10.04)
    // float dt = streamPlayback?streamPlayback->time():0;
    
    time_duration d = microsec_clock::local_time() - _startPlay_timestamp;
    float dt = d.total_milliseconds()*0.001f;
    float t = dt;
    t += _data.spannedInterval ().first / sample_rate();

#ifdef _WIN32
// TODO deal with output latency some other way. Such as adjusting 'dt' and keep updating in playbackview.
//    t -= outputLatency();
#endif

    return std::max(0.f, t);
}


float Playback::
        outputLatency()
{
    return streamPlayback?streamPlayback->outputLatency():0;
}


void Playback::
        put( Signal::pBuffer buffer )
{
    TIME_PLAYBACK TaskTimer tt("Playback::put %s", buffer->getInterval().toString().c_str());
    _last_timestamp = microsec_clock::local_time();

    if (_data.empty())
    {
        const Signal::Interval I = buffer->getInterval();
        _first_timestamp = _last_timestamp;
        _first_buffer_size = I.count();

        // Discard zeros in the beginning of the signal
/*        float* p = buffer->waveform_data()->getCpuMemory();
        Signal::IntervalType i;
        for (i=0; i<I.count(); ++i)
        {
            if (fabsf(p[i]) > 0.001)
                break;
        }

        if (i>0)
        {
            Signal::Intervals is = _data.invalid_samples();
            _data.clear();
            _data.invalidate_samples( is - Signal::Interval(I.first, I.first + i) );

            Signal::Interval rI( I.first + i, I.last );

            if (0==rI.count())
                return;

            buffer = Signal::BufferSource( buffer ).readFixedLength( rI );
        }*/
	}

    // Make sure the buffer is moved over to CPU memory and that GPU memory is released
    // (because the audio stream callback is executed from a different thread
    // it can't access the GPU memory)
    buffer->release_extra_resources ();

    _data.put( buffer );
    _output_channels = buffer->number_of_channels ();

    if (streamPlayback)
    {
        if (streamPlayback->isActive()) {
            TIME_PLAYBACK TaskInfo("Is playing");
        } else {
            TIME_PLAYBACK TaskInfo("Is paused");
        }
        return;
    }

    onFinished();
}


void Playback::
        setExpectedSamples(const Signal::Interval &I, int C)
{
    _expected = I;
    invalidate_samples (_expected, C);
}


void Playback::
        stop()
{
    if (streamPlayback)
    {
        // streamPlayback->stop will invoke a join with readBuffer
        if (!streamPlayback->isStopped())
            streamPlayback->stop();
    }

    _playback_itr = _data.spannedInterval ().last;
    _max_found = 1;
    _min_found = -1;
}


void Playback::
        reset()
{
    stop();

    if (streamPlayback)
        streamPlayback.reset();

    _playback_itr = 0;

    _data.clear();
}


bool Playback::
        deleteMe()
{
    // Keep cache
    return false;

    // Discard cache
    //return _data.deleteMe() && isStopped();
}


void Playback::
        invalidate_samples( const Signal::Intervals& s, int C )
{
    // Don't bother recomputing stuff we've already played
    Signal::Interval whatsLeft(
                        _playback_itr,
                        Signal::Interval::IntervalType_MAX);

    if (C != _data.num_channels ())
        _data.clear ();

    _data.invalidate_samples( s & whatsLeft );
}


unsigned Playback::
        num_channels()
{
    return _data.num_channels();
}


Signal::Intervals Playback::
        invalid_samples()
{
    return ~_data.samplesDesc() & _expected;
}


void Playback::
        onFinished()
{
    if (isUnderfed() )
    {
        TIME_PLAYBACK TaskTimer("Waiting for more data");
        return;
    }

    try
    {

        // Be nice, don't just destroy the previous one but ask it to stop first
        if (streamPlayback && !streamPlayback->isStopped())
        {
            streamPlayback->stop();
        }
        streamPlayback.reset();

        portaudio::System &sys = portaudio::System::instance();

        if (_output_device < 0) {
            TaskInfo("No output device, can't start playing");
            return;
        }

        TIME_PLAYBACK TaskInfo("Start playing on: %s", sys.deviceByIndex(_output_device).name() );

        unsigned requested_number_of_channels = num_channels();
        unsigned available_channels = sys.deviceByIndex(_output_device).maxOutputChannels();

        if (available_channels < requested_number_of_channels)
        {
            requested_number_of_channels = available_channels;
            _output_channels = requested_number_of_channels;
        }

        portaudio::Device& device = sys.deviceByIndex(_output_device);
        for (int interleaved=0; interleaved<2; ++interleaved)
        {
            _is_interleaved = interleaved!=0;

            // Set up the parameters required to open a (Callback)Stream:
            portaudio::DirectionSpecificStreamParameters outParamsPlayback(
                    device,
                    requested_number_of_channels,
                    portaudio::FLOAT32,
                    _is_interleaved,
                    device.defaultLowOutputLatency(),
                    //sys.deviceByIndex(_output_device).defaultHighOutputLatency(),
                    NULL);

            PaError err = Pa_IsFormatSupported(0, outParamsPlayback.paStreamParameters(), sample_rate());
            bool fmtok = err==paFormatIsSupported;
            if (!fmtok)
                continue;

            portaudio::StreamParameters paramsPlayback(
                    portaudio::DirectionSpecificStreamParameters::null(),
                    outParamsPlayback,
                    sample_rate(),
                    0,
                    paNoFlag);//paClipOff);


            // Create (and (re)open) a new Stream:
            streamPlayback.reset( new portaudio::MemFunCallbackStream<Playback>(
                    paramsPlayback,
                    *this,
                    &Playback::readBuffer) );

            _playback_itr = _data.spannedInterval ().first;

            streamPlayback->start();
            break;
        }

    } catch (const portaudio::PaException& x) {
        QMessageBox::warning( 0,
                     "Can't play sound",
                     x.what() );
        _data.clear();
    }
}


bool Playback::
        isStopped()
{
    //return streamPlayback ? !streamPlayback->isActive() || streamPlayback->isStopped():true;
    bool isActive = streamPlayback ? streamPlayback->isActive() : false;
    //bool isStopped = streamPlayback ? streamPlayback->isStopped() : true;
    // isActive and isStopped might both be false at the same time
    bool paused = isPaused();
    return streamPlayback ? !isActive && !paused : true;
}


bool Playback::
        isPaused()
{
    bool isStopped = streamPlayback ? streamPlayback->isStopped() : true;

    bool read_past_end = _data.empty() ? true : _playback_itr >= _data.spannedInterval ().last;

    return isStopped && !read_past_end;
}


bool Playback::
        hasReachedEnd()
{
    if (_data.empty())
        return false;

    if (invalid_samples())
        return false;

    return time()*_data.sample_rate() > _data.spannedInterval ().last;
}


bool Playback::
        isUnderfed()
{
    unsigned nAccumulated_samples = _data.spannedInterval ().count ();

    if (!_data.empty() && !invalid_samples()) {
        TIME_PLAYBACK TaskInfo("Not underfed");
        return false; // No more expected samples, not underfed
    }

    if (nAccumulated_samples < 0.1f*_data.sample_rate() || nAccumulated_samples < 3*_first_buffer_size ) {
        TIME_PLAYBACK TaskInfo("Underfed %d", nAccumulated_samples);
        return true; // Haven't received much data, wait to do a better estimate
    }

    time_duration diff = _last_timestamp - _first_timestamp;
    float accumulation_time = diff.total_milliseconds() * 0.001f;

    // _first_timestamp is taken after the first buffer,
    // _last_timestamp is taken after the last buffer,
    // that means that accumulation_time is the time it took to accumulate all buffers except
    // the first buffer.
    float incoming_samples_per_sec = (nAccumulated_samples - _first_buffer_size) / accumulation_time;

    Signal::IntervalType marker = _playback_itr;
    if (0==marker)
        marker = _data.spannedInterval ().first;

    Signal::Interval cov = invalid_samples().spannedInterval();
    float time_left =
            (cov.last - marker) / _data.sample_rate();

    float estimated_time_required = cov.count() / incoming_samples_per_sec;

    // Add small margin
    estimated_time_required *= 1.11f;

    // Return if the estimated time to receive all expected samples is greater than
    // the time it would take to play the remaining part of the data.
    // If it is, the sink is underfed.
    TIME_PLAYBACK TaskInfo("Time left %g %s %g estimated time required. %s underfed", time_left, time_left < estimated_time_required?"<":">=", estimated_time_required, time_left < estimated_time_required?"Is":"Not");
    bool underfed = false;
    underfed |= time_left < estimated_time_required;

    // Also, check that we keep a margin of 3 buffers
    underfed |= marker + 3*_first_buffer_size > cov.first;

    return underfed;
}


void Playback::
        pausePlayback(bool pause)
{
    if (!streamPlayback)
        return;

    if (pause)
    {
        _playback_itr = time()*sample_rate();

        if (streamPlayback->isActive() && !streamPlayback->isStopped())
            streamPlayback->stop();
    }
    else
    {
        if (!isPaused() || _data.empty())
            return;

        _startPlay_timestamp = microsec_clock::local_time();
        _startPlay_timestamp -= time_duration(0, 0, 0, (_playback_itr - _data.spannedInterval ().first)/sample_rate()*time_duration::ticks_per_second() );

        streamPlayback->start();
    }
}


void Playback::
        restart_playback()
{
    if (streamPlayback)
    {
        TIME_PLAYBACK TaskTimer tt("Restaring playback");

        _playback_itr = 0;
        _max_found = 1;
        _min_found = -1;

        onFinished();
    }
}


void Playback::
        normalize( float* p, unsigned N )
{
    for (unsigned j=0; j<N; ++j)
    {
        if (p[j] > _max_found) _max_found = p[j];
        if (p[j] < _min_found) _min_found = p[j];

        p[j] = (p[j] - _min_found)/(_max_found - _min_found)*2-1;
    }
}


int Playback::
        readBuffer(const void * /*inputBuffer*/,
                 void *outputBuffer,
                 unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo * /*timeInfo*/,
                 PaStreamCallbackFlags /*statusFlags*/)
{
    float FS;
    TIME_PLAYBACK FS = _data.sample_rate();
    TIME_PLAYBACK TaskTimer("Playback::readBuffer Reading [%d, %d)%u# from %d. [%g, %g)%g s",
                           (int)_playback_itr, (int)(_playback_itr+framesPerBuffer),
                           (unsigned)framesPerBuffer, (int)_data.spannedInterval ().count (),
                           _playback_itr/ FS, (_playback_itr + framesPerBuffer)/ FS,
                           framesPerBuffer/ FS);

    if (!_data.empty() && _playback_itr == _data.spannedInterval ().first) {
        _startPlay_timestamp = microsec_clock::local_time();
    }

    unsigned nchannels = _output_channels;
    Signal::pBuffer mb = _data.read( Signal::Interval(_playback_itr, _playback_itr+framesPerBuffer) );
    for (unsigned c=0; c<nchannels; ++c)
    {
        Signal::pMonoBuffer b = mb->getChannel (c);
        float *p = CpuMemoryStorage::ReadOnly<1>( b->waveform_data() ).ptr();
        if (_is_interleaved)
        {
            float *out = (float *)outputBuffer;
            float *buffer = out;
            for (unsigned j=0; j<framesPerBuffer; ++j)
                buffer[j*nchannels + c] = p[j];
        }
        else
        {
            float **out = static_cast<float **>(outputBuffer);
            float *buffer = out[c];
            ::memcpy( buffer, p, framesPerBuffer*sizeof(float) );
            normalize( buffer, framesPerBuffer );
        }
    }

    if (_is_interleaved)
        normalize( (float *)outputBuffer, framesPerBuffer*nchannels );

    _playback_itr += framesPerBuffer;

    int ret = paContinue;
    if (_data.spannedInterval ().last + (Signal::IntervalType)framesPerBuffer < _playback_itr ) {
        TaskInfo("DONE");
        ret = paComplete;
    } else {
        if (_data.spannedInterval ().last < _playback_itr ) {
            TaskInfo("PAST END");
            // TODO if !_data.invalid_samples().empty() should pause playback here and continue when data is made available
        } else {
        }
    }

    return ret;
}


void Playback::
    test()
{
    Playback_logging = false;

    // It should not throw any exceptions when requesting an invalid device
    {
        portaudio::AutoSystem autoSys;
        portaudio::System &sys = portaudio::System::instance();

        // Don't throw any exceptions
        for (int i=-10; i<sys.deviceCount ()*2+10; ++i) {
            Playback a(i);
            int od = a.output_device();
            EXCEPTION_ASSERTX ( 0 <= od && od < sys.deviceCount (), (format("od=%1% not in [0, %2%). i=%3%") % od % sys.deviceCount () % i));
            EXCEPTION_ASSERT_LESS ( od, sys.deviceCount () );
        }
    }

    // It should be stopped and pause should be disabled when there's not put data
    {
        Playback pb(-1);
        EXCEPTION_ASSERT (pb.isStopped () && !pb.isPaused ());
        EXCEPTION_ASSERT (pb.isStopped () && !pb.isPaused ());
        pb.setExpectedSamples (Signal::Interval(10,20), 1);
        EXCEPTION_ASSERT (pb.isStopped () && !pb.isPaused ());
        pb.pausePlayback (true);
        EXCEPTION_ASSERT (pb.isStopped () && !pb.isPaused ());
        pb.pausePlayback (false);
        EXCEPTION_ASSERT (pb.isStopped () && !pb.isPaused ());
    }

    // It should start playing when feeded with data
    {
        Playback pb(-1);
        pb.setExpectedSamples (Signal::Interval(10,20), 1);
        EXCEPTION_ASSERT (pb.isStopped ());
        EXCEPTION_ASSERT (!pb.isPaused ());
        pb.put (Signal::pBuffer(new Signal::Buffer(Signal::Interval(10,20), 44100, 1)));
        EXCEPTION_ASSERT (!pb.isStopped () && !pb.isPaused ());
        pb.pausePlayback (true);
        EXCEPTION_ASSERT (!pb.isStopped () && pb.isPaused ());
        pb.pausePlayback (false);
        EXCEPTION_ASSERT (!pb.isStopped () && !pb.isPaused ());
    }

    Playback_logging = true;
}

} // namespace Adapters
