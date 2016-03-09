#include "collection.h"
#include "blockmanagement/blockfactory.h"
#include "blockmanagement/blockinitializer.h"
#include "blockquery.h"
#include "blockcacheinfo.h"
#include "reference_hash.h"
#include "blockmanagement/clearinterval.h"
#include "blockmanagement/garbagecollector.h"

// Gpumisc
#include "neat_math.h"
#include "computationkernel.h"
#include "tasktimer.h"
#include "log.h"

// boost
#include <boost/date_time/posix_time/posix_time.hpp>

// std
#include <string>

// MSVC-GCC-compatibility workarounds
#include "msc_stdc.h"


//#define INFO_COLLECTION
#define INFO_COLLECTION if(0)

//#define VERBOSE_EACH_FRAME_COLLECTION
#define VERBOSE_EACH_FRAME_COLLECTION if(0)

//#define LOG_BLOCK_USAGE_WHEN_DISCARDING_BLOCKS
#define LOG_BLOCK_USAGE_WHEN_DISCARDING_BLOCKS if(0)

//#define DEBUG_FRAME_ON_FIRST_MISSING_BLOCK

using namespace Signal;
using namespace boost;

namespace Heightmap {


Collection::
        Collection( BlockLayout block_layout, VisualizationParams::const_ptr visualization_params)
:   block_layout_( 2, 2, FLT_MAX ),
    visualization_params_(),
    cache_( new BlockCache ),
    block_factory_(new BlockManagement::BlockFactory),
    block_initializer_(),
    _is_visible( true ),
    _frame_counter(0),
    _prev_length(.0f)
{
    // set _max_sample_size
    this->block_layout (block_layout);
    this->visualization_params (visualization_params);
}


Collection::
        ~Collection()
{
    INFO_COLLECTION TaskInfo ti("~Collection");
    clear();
}


void Collection::
        clear()
{
    auto C = cache_->clear ();
    block_factory_->updater()->clearQueue();

    // merge
    for (auto const r: to_remove_)
        C[r->reference()] = r;
    to_remove_.clear ();

    // rebuild to_remove_ if any blocks are still being used, such as if a worker is actively processing data for a block
    for (auto const& v : C)
    {
        pBlock const& b = v.second;
        if (!b.unique())
            to_remove_.insert (b);
    }

    INFO_COLLECTION {
        TaskInfo ti("Collection::Reset, cache count = %u, size = %s", C.size(), DataStorageVoid::getMemorySizeText( BlockCacheInfo::cacheByteSize (C) ).c_str() );
        RegionFactory rr(block_layout_);
        for (const BlockCache::cache_t::value_type& b : C)
        {
            TaskInfo(format("%s") % rr.getVisible (b.first));
        }

        TaskInfo("of which recent count %u, %d not removed", C.size (), to_remove_.size ());
    }

    C.clear ();
    Render::BlockTextures::gc (true);
    INFO_COLLECTION Log ("Render::BlockTextures:gc left %d textures") % Render::BlockTextures::getCapacity ();

    failed_allocation_ = false;

    if (visualization_params_)
    {
        // Don't delete block_factory_ as it would delete BlockUpdater which might be currently in use by a worker.
        block_factory_->reset(block_layout_, visualization_params_);
        block_initializer_.reset(new BlockManagement::BlockInitializer(block_layout_, visualization_params_, cache_));
    }
}


void Collection::
        frame_begin()
{
    std::set<pBlock> to_keep;
    for (auto const& b : to_remove_)
    {
        if (!b.unique())
            to_keep.insert (b);
    }
    to_keep.swap (to_remove_);
    to_keep.clear ();

    BlockCache::cache_t cache = cache_->clone ();

    VERBOSE_EACH_FRAME_COLLECTION TaskTimer tt(boost::format("%s(), %u")
            % __FUNCTION__ % cache.size ());

    missing_data_.swap (missing_data_next_);
    missing_data_next_.clear ();

    boost::unordered_set<Reference> blocksToPoke;

#ifndef PAINT_BLOCKS_FROM_UPDATE_THREAD
    block_factory_->updater()->processUpdates (true);
#endif

    for (const BlockCache::cache_t::value_type& b : cache)
    {
        Block* block = b.second.get();

        if (block->frame_number_last_used == _frame_counter)
        {
            // Mark these blocks and surrounding blocks as in-use
            blocksToPoke.insert (block->reference ());
        }
    }

    boost::unordered_set<Reference> blocksToPoke2;

    for (const Reference& r : blocksToPoke)
    {
        // poke blocks that are still allocated and likely to be needed again soon

        // poke children
        blocksToPoke2.insert (r.left());
        blocksToPoke2.insert (r.right());
        blocksToPoke2.insert (r.top());
        blocksToPoke2.insert (r.bottom());

        // poke parent
        blocksToPoke2.insert (r.parent());
        blocksToPoke2.insert (r.parentHorizontal());
        blocksToPoke2.insert (r.parentVertical());

        // poke surrounding sibblings, and this
        Reference q = r;
        for (q.block_index[0] = std::max(1u, r.block_index[0]) - 1;
             q.block_index[0] <= r.block_index[0] + 1;
             q.block_index[0]++)
        {
            for (q.block_index[1] = std::max(1u, r.block_index[1]) - 1;
                 q.block_index[1] <= r.block_index[1] + 1;
                 q.block_index[1]++)
            {
                blocksToPoke2.insert (q);
            }
        }
    }


    for (const Reference& r : blocksToPoke2)
    {
        auto i = cache.find (r);
        if (i != cache.end ())
            poke(i->second);
    }


    runGarbageCollection(false);

    _frame_counter++;
}


unsigned Collection::
        frame_number() const
{
    return _frame_counter;
}


void Collection::
        poke(pBlock b)
{
    if (b->frame_number_last_used != _frame_counter)
        b->frame_number_last_used = _frame_counter - 1;
}


Reference Collection::
        entireHeightmap() const
{
    Reference r;
    r.log2_samples_size = Reference::Scale( ceil(log2(_max_sample_size.time )), ceil(log2( _max_sample_size.scale )));
    r.block_index = Reference::Index(0,0);
    return r;
}


pBlock Collection::
        getBlock( const Reference& ref )
{
    Log("collection: using deprecated getBlock");

    pBlock block = cache_->find( ref );
    if (block)
        return block;

    createMissingBlocks({ref});

    return cache_->find( ref );
}


void Collection::
         prepareBlocks(const Render::RenderSet::references_t& R)
{
    std::unordered_set<Reference> missing;

    BlockCache::cache_t cache = cache_->clone ();

    // entireHeightmap should always be included so that it can be used to fill new blocks
    Reference entire = entireHeightmap();
    auto i = cache.find (entire);
    if (i == cache.end())
        missing.insert (entire);
    else
        i->second->frame_number_last_used = _frame_counter;

    for (const auto& r : R)
    {
        auto i = cache.find(r.first);
        if (i == cache.end())
            missing.insert (r.first);
        else
            i->second->frame_number_last_used = _frame_counter;
    }

    if (missing.empty ())
    {
        // Nothing to do
        return;
    }

    createMissingBlocks(missing);
}


void Collection::
         createMissingBlocks(const std::unordered_set<Reference>& missing)
{
    VERBOSE_EACH_FRAME_COLLECTION TaskTimer tt("Collection::createMissingBlocks %d new", missing.size());

    std::vector<pBlock> blocks_to_init;
    blocks_to_init.reserve (missing.size());

    for (const auto& ref : missing )
    {
        pBlock block = block_factory_->createBlock (ref);
        if (block)
        {
            blocks_to_init.push_back (block);
            recently_created_ |= block->getInterval ();
        }
    }

    for (const pBlock& block : blocks_to_init)
        block->frame_number_last_used = _frame_counter;

    missing_data_next_ |= block_initializer_->initBlocks(blocks_to_init);

#ifdef DEBUG_FRAME_ON_FIRST_MISSING_BLOCK
    static int misscount = 0;
    if (!missing.empty ())
        misscount++;
    if (misscount==10 && !missing.empty ())
        glInsertEventMarkerEXT(0, "com.apple.GPUTools.event.debug-frame");
#endif

    for (const pBlock& block : blocks_to_init)
    {
        // Make this block available for rendering
        cache_->insert (block);
    }
}


int Collection::
        runGarbageCollection(bool aggressive)
{
    // When using the block textures (OpenGL resources) from a separate update
    // thread the accesses needs to be synchronized. The update thread is
    // responsible for reference counting the texture until it has been glFlushed
    // onto the OpenGL driver. Likewise, texture releases we don't need are kept
    // accross a glFlush (swap between frames) by putting them in 'to_remove_'
    // which a list of blocks to be removed on 'frame_begin'.

    Blocks::GarbageCollector gc(cache_);
    unsigned F = gc.countBlocksUsedThisFrame(_frame_counter);
    unsigned max_cache_size = 2*F;
    unsigned n_to_release = cache_->size() <= max_cache_size ? 0 : cache_->size() - max_cache_size;

    int discarded_blocks = 0;
    if (n_to_release) for (const pBlock& b : gc.getNOldest( _frame_counter, n_to_release))
    {
        removeBlock (b);
        ++discarded_blocks;
    }

    if (aggressive)
    {
        for (const pBlock& b : gc.getAllNotUsedInThisFrame (_frame_counter))
        {
            removeBlock (b);
            ++discarded_blocks;
        }
    }
    else
    {
//        if (const pBlock& released = gc.getOldestBlock (_frame_counter))
//        {
//            removeBlock (released);
//            ++i;
//        }
    }

    Render::BlockTextures::gc(aggressive);
    LOG_BLOCK_USAGE_WHEN_DISCARDING_BLOCKS if (0<discarded_blocks)
        Log("collection: discarded_blocks %d, has %d blocks in cache of which %d were used/poked. %d of %d textures used")
                % discarded_blocks
                % cache_->size()
                % F
                % Render::BlockTextures::getUseCount ()
                % Render::BlockTextures::getCapacity ();

    return discarded_blocks;
}


unsigned long Collection::
        cacheByteSize() const
{
    return BlockCacheInfo::cacheByteSize (cache_->clone ());
}


unsigned Collection::
        cacheCount() const
{
    return cache_->size();
}


void Collection::
        printCacheSize() const
{
    BlockCacheInfo::printCacheSize(cache_->clone ());
}


BlockCache::ptr Collection::
        cache(Collection::ptr C)
{
    return C.raw ()->cache_;
}


void Collection::
        discardOutside(Signal::Interval I)
{
    std::list<pBlock> discarded = Blocks::ClearInterval(cache_).discardOutside (I);
    for (pBlock b : discarded)
        removeBlock (b);
}


bool Collection::
        failed_allocation()
{
    return failed_allocation_;
}


bool Collection::
        isVisible() const
{
    return _is_visible;
}


void Collection::
        setVisible(bool v)
{
    _is_visible = v;
}


BlockLayout Collection::
        block_layout() const
{
    return block_layout_;
}


VisualizationParams::const_ptr Collection::
        visualization_params() const
{
    return visualization_params_;
}


void Collection::
        length(double length)
{
    double m = block_layout_.margin () / double(block_layout_.texels_per_row ());
    _max_sample_size.time = std::max(1., length) * (1+m);

    // If the signal has gotten shorter, make sure to discard all blocks that
    // go outside the new shorter interval
    if (_prev_length > length)
    {
        Log ("collection: _prev_length %g > length %g") % _prev_length % length;
        discardOutside( Signal::Interval(0, length*block_layout_.targetSampleRate ()) );
    }

    _prev_length = length;
}


void Collection::
        block_layout(BlockLayout v)
{
    if (block_layout_ == v)
        return;

    block_layout_ = v;

    _max_sample_size.scale = 1.f;
    length(_prev_length);

    clear();
}


void Collection::
        visualization_params(VisualizationParams::const_ptr v)
{
    EXCEPTION_ASSERT( v );
    if (visualization_params_ == v)
        return;

    visualization_params_ = v;

    clear();
}


Intervals Collection::
        needed_samples() const
{
    Intervals r;

    if (!_is_visible)
        return r;

    for ( auto const& a : cache_->clone () )
    {
        Block const& b = *a.second;
        unsigned framediff = _frame_counter - b.frame_number_last_used;
        if (1 == framediff || 0 == framediff) // this block was used last frame or this frame
            r |= b.getInterval();
    }

    return r;
}

////// private


Signal::Intervals Collection::
        recently_created()
{
    Signal::Intervals I;
    recently_created_.swap (I);
    return I;
}


Signal::Intervals Collection::
        missing_data()
{
    Signal::Intervals I;
    missing_data_.swap (I);
    return I;
}


void Collection::
        removeBlock (pBlock b)
{
    to_remove_.insert (b);
    cache_->erase(b->reference());
}

} // namespace Heightmap
