#ifndef PLAYBACKMODEL_H
#define PLAYBACKMODEL_H

#include "signal/operation.h"
#include "signal/processing/targetmarker.h"
#include "signal/sink.h"

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

//Use Signal::Processing namespace
//        Signal::pTarget playbackTarget;
        Signal::Processing::TargetMarker::ptr target_marker;

        std::string selection_filename;

        SelectionModel* selection;
        PlaybackMarkersModel* markers;

        Signal::OperationDesc::ptr adapter_playback;

        Signal::Operation::ptr playback();
    };
} // namespace Tools
#endif // PLAYBACKMODEL_H
