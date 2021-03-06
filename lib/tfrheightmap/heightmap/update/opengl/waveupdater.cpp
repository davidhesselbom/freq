#include "waveupdater.h"
#include "heightmap/update/waveformblockupdater.h"
#include "fbo2block.h"
#include "wave2fbo.h"
#include "lazy.h"
#include "log.h"

#include <unordered_map>

//#define INFO
#define INFO if(0)

using namespace std;
using namespace JustMisc;

namespace std {
    template<typename T>
    struct hash<boost::shared_ptr<T>>
    {
        size_t operator()(boost::shared_ptr<T> const& p) const
        {
            return hash<T*>()(p.get ());
        }
    };
}


namespace Heightmap {
namespace Update {
namespace OpenGL {

class WaveUpdaterPrivate
{
public:
    Fbo2Block fbo2block;
};


WaveUpdater::
        WaveUpdater()
    :
      p(new WaveUpdaterPrivate)
{
}


WaveUpdater::
        ~WaveUpdater()
{
    delete p;
}


void WaveUpdater::
        processJobs( queue<UpdateQueue::Job>& jobs )
{
    // Select subset to work on, must consume jobs in order
    vector<UpdateQueue::Job> myjobs;
    while (!jobs.empty ())
    {
        UpdateQueue::Job& j = jobs.front ();
        if (dynamic_cast<const WaveformBlockUpdater::Job*>(j.updatejob.get ()))
        {
            myjobs.push_back (move(j)); // Steal it
            jobs.pop ();
        }
        else
            break;
    }

    // Prepare Wave2Fbo
    unordered_map<Signal::pMonoBuffer,lazy<Wave2Fbo>> wave2fbo;
    for (const UpdateQueue::Job& j : myjobs)
    {
        auto job = dynamic_cast<const WaveformBlockUpdater::Job*>(j.updatejob.get ());
        wave2fbo[job->b] = Wave2Fbo(job->b);
    }

    // Remap block -> buffer (instead of buffer -> blocks) because we want to draw all
    // buffers to each block, instead of each buffer to all blocks.
    //
    // The chunks must be drawn in order, thus a "vector<Tfr::pMonoBuffer>" is required
    // to preserve ordering.
    unordered_map<pBlock, vector<Signal::pMonoBuffer>> buffers_per_block;
    for (const UpdateQueue::Job& j : myjobs)
    {
        auto job = dynamic_cast<const WaveformBlockUpdater::Job*>(j.updatejob.get ());

        for (pBlock block : j.intersecting_blocks)
            buffers_per_block[block].push_back(job->b);
    }

    // Draw from all chunks to each block
    for (auto& f : buffers_per_block)
    {
        const pBlock& block = f.first;
        auto fbo_mapping = p->fbo2block.begin (block);

        for (auto& b : f.second)
            wave2fbo[b]->draw();

        // suppress warning caused by RAII
        (void)fbo_mapping;
    }

    for (UpdateQueue::Job& j : myjobs) {
        INFO {
            auto job = dynamic_cast<const WaveformBlockUpdater::Job*>(j.updatejob.get ());
            Log("WaveUpdater finished %s") % job->b->getInterval();
        }

        j.promise.set_value ();
    }
}

} // namespace OpenGL
} // namespace Update
} // namespace Heightmap
