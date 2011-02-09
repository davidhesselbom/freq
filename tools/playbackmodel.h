#ifndef PLAYBACKMODEL_H
#define PLAYBACKMODEL_H

#include "signal/operation.h"
#include "signal/postsink.h"
#include "signal/worker.h"

#include <string>

namespace Sawe { class Project; }
namespace Adapters { class Playback; }

namespace Tools
{
    class SelectionModel;
    class PlaybackMarkersModel;

    class PlaybackModel
    {
    public:
        PlaybackModel(Sawe::Project* project);

        Signal::PostSink* getPostSink();
        Signal::pWorkerCallback postsinkCallback;

        unsigned playback_device;
        std::string selection_filename;

        SelectionModel* selection;
        PlaybackMarkersModel* markers;

        Signal::pOperation adapter_playback;

        Adapters::Playback* playback();
    };
} // namespace Tools
#endif // PLAYBACKMODEL_H