#include "collection.h"
#include "render/glblock.h"
#include "blockmanagement/blockfactory.h"
#include "blockmanagement/blockinitializer.h"
#include "blockquery.h"
#include "blockcacheinfo.h"
#include "reference_hash.h"
#include "blockmanagement/clearinterval.h"
#include "blockmanagement/garbagecollector.h"

// Gpumisc
//#include "GlException.h"
#include "neat_math.h"
#include "computationkernel.h"
#include "tasktimer.h"
#include "log.h"

// boost
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/unordered_set.hpp>

// std
#include <string>

// MSVC-GCC-compatibility workarounds
#include "msc_stdc.h"


//#define INFO_COLLECTION
#define INFO_COLLECTION if(0)

//#define VERBOSE_EACH_FRAME_COLLECTION
#define VERBOSE_EACH_FRAME_COLLECTION if(0)

using namespace Signal;
using namespace boost;

namespace Heightmap {


Collection::
        Collection( BlockLayout block_layout, VisualizationParams::const_ptr visualization_params)
:   block_layout_( 2, 2, FLT_MAX ),
    visualization_params_(),
    cache_( new BlockCache ),
    block_factory_(new BlockManagement::BlockFactory(block_layout, visualization_params)),
    block_initializer_(new BlockManagement::BlockInitializer(block_layout, visualization_params, cache_)),
    block_textures_(new Render::BlockTextures(block_layout)),
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
    BlockCache::cache_t C = cache_->clear ();
    INFO_COLLECTION {
        TaskInfo ti("Collection::Reset, cache count = %u, size = %s", C.size(), DataStorageVoid::getMemorySizeText( BlockCacheInfo::cacheByteSize (C) ).c_str() );
        RegionFactory rr(block_layout_);
        for (const BlockCache::cache_t::value_type& b : C)
        {
            TaskInfo(format("%s") % rr(b.first));
        }

        TaskInfo("of which recent count %u", C.size ());
    }

    failed_allocation_ = false;
}


void Collection::
        next_frame()
{
    BlockCache::cache_t cache = cache_->clone ();

    VERBOSE_EACH_FRAME_COLLECTION TaskTimer tt(boost::format("%s(), %u")
            % __FUNCTION__ % cache.size ());

    block_factory_->next_frame();

    boost::unordered_set<Reference> blocksToPoke;

    for (const BlockCache::cache_t::value_type& b : cache)
    {
        Block* block = b.second.get();
        if (block->frame_number_last_used == _frame_counter)
        {
            // Mark these blocks and surrounding blocks as in-use
            blocksToPoke.insert (block->reference ());
        }
        else if (block->glblock && block->glblock->has_texture ())
        {
            // This block isn't used but it has allocated a texture in OpenGL
            // memory that can easily recreate as soon as it is needed.
//            INFO_COLLECTION TaskTimer tt(boost::format("Deleting texture for block %s") % block->getRegion ());
            //block->glblock->delete_texture ();
        }
    }

    boost::unordered_set<Reference> blocksToPoke2;

    for (const Reference& r : blocksToPoke)
    {
        // poke blocks that are likely to be needed soon

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
    b->frame_number_last_used = _frame_counter;
}


Reference Collection::
        entireHeightmap() const
{
    Reference r;
    r.log2_samples_size = Reference::Scale( floor_log2( _max_sample_size.time ), floor_log2( _max_sample_size.scale ));
    r.block_index = Reference::Index(0,0);
    return r;
}


pBlock Collection::
        getBlock( const Reference& ref )
{
    pBlock block = cache_->find( ref );
    if (block)
        return block;

    createMissingBlocks(Render::RenderSet::references_t{ref});

    return cache_->find( ref );
}


void Collection::
         createMissingBlocks(const Render::RenderSet::references_t& R)
{
    Render::RenderSet::references_t missing;

    {
        BlockCache::cache_t cache = this->cache ()->clone ();
        for (const Reference& r : R)
            if (cache.find(r) == cache.end())
                missing.insert (r);
    }

    if (missing.empty ())
    {
        // Nothing to do
        return;
    }

    VERBOSE_EACH_FRAME_COLLECTION TaskTimer tt("Collection::createMissingBlocks %d new", missing.size());

    auto w = block_textures_.write ();
    std::vector<GlTexture::ptr> missing_textures = w->getUnusedTextures(missing.size());
    int need_more = missing.size() - missing_textures.size ();
    if (need_more)
    {
        // Only grow capacity here, possibly shrink when running garbage collection after each frame
        w->setCapacityHint (w->getCapacity () + need_more);
    }

    auto T = w->getUnusedTextures(need_more);
    w.unlock ();
    for (GlTexture::ptr& t : T)
        missing_textures.push_back (t);

    std::vector<pBlock> blocks_to_init;
    blocks_to_init.reserve (missing.size());

    for (const Reference& ref : missing )
    {
        if (missing_textures.empty ()) {
            failed_allocation_ = true;
            return;
        }

        pBlock block = block_factory_->createBlock (ref, missing_textures.back ());
        if (block)
        {
            missing_textures.pop_back ();
            blocks_to_init.push_back (block);
        }
    }

    block_initializer_->initBlocks(blocks_to_init);

    for (const pBlock& block : blocks_to_init)
    {
        block->frame_number_last_used = _frame_counter;
        cache_->insert (block);
    }
}


int Collection::
        runGarbageCollection(bool aggressive)
{
    {
        // Make sure this global block covering everything is available to provide a background color
        pBlock b = getBlock(entireHeightmap ());
        b->frame_number_last_used = _frame_counter;
    }

    Blocks::GarbageCollector gc(cache_);
    unsigned F = gc.countBlocksUsedThisFrame(_frame_counter);
    block_textures_->setCapacityHint( F );
    unsigned max_cache_size = 4*F;

    if (cache_->size() <= max_cache_size)
        return 0;

    unsigned n_to_release = cache_->size() - max_cache_size;
    int i = 0;
    for (pBlock b : gc.releaseNOldest( _frame_counter, n_to_release))
    {
        removeBlock (b);
        ++i;
    }

    if (aggressive)
    {
        for (pBlock b : Blocks::GarbageCollector(cache_).releaseAllNotUsedInThisFrame (_frame_counter))
        {
            removeBlock (b);
            ++i;
        }
    }
    else
    {
        for (pBlock b : Blocks::GarbageCollector(cache_).runUntilComplete (_frame_counter))
        {
            removeBlock (b);
            ++i;
        }
//        while( pBlock released = Blocks::GarbageCollector(cache_).runOnce (_frame_counter))
//        {
//            removeBlock (released);
//            ++i;
//        }
    }
    return i;
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
        cache() const
{
    return cache_;
}


void Collection::
        discardOutside(Signal::Interval I)
{
    std::list<pBlock> discarded = Blocks::ClearInterval(cache_).discardOutside (I);
    for (pBlock b : discarded) {
        removeBlock (b);
    }
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
        length(float length)
{
    _max_sample_size.time = 2.f*std::max(1.f, length)/block_layout_.texels_per_row ();

    // If the signal has gotten shorter, make sure to discard all blocks that
    // go outside the new shorter interval
    if (_prev_length > length)
        discardOutside( Signal::Interval(0, length*block_layout_.targetSampleRate ()) );

    _prev_length = length;
}


void Collection::
        block_layout(BlockLayout v)
{
    if (block_layout_ == v)
        return;

    block_layout_ = v;

    if (visualization_params_)
    {
        block_factory_.reset(new BlockManagement::BlockFactory(block_layout_, visualization_params_));
        block_initializer_.reset(new BlockManagement::BlockInitializer(block_layout_, visualization_params_, cache_));
    }
    block_textures_.reset(new Render::BlockTextures(block_layout_));

    _max_sample_size.scale = 1.f/block_layout_.texels_per_column ();
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

    block_factory_.reset(new BlockManagement::BlockFactory(block_layout_, visualization_params_));
    block_initializer_.reset(new BlockManagement::BlockInitializer(block_layout_, visualization_params_, cache_));

    clear();
}


Intervals Collection::
        needed_samples(UnsignedIntervalType& smallest_length)
{
    Intervals r;

    if (!_is_visible)
        return r;

    for ( auto a : cache_->clone () )
    {
        const Block& b = *a.second;
        unsigned framediff = _frame_counter - b.frame_number_last_used;
        if (1 == framediff || 0 == framediff) // this block was used last frame or this frame
        {
            Interval i = b.getInterval();
            r |= i;
            if (i.count () < smallest_length)
                smallest_length = i.count ();
        }
    }

    return r;
}

////// private


Signal::Intervals Collection::
        recently_created()
{
    return block_factory_->recently_created();
}


void Collection::
        removeBlock (pBlock b)
{
    cache_->erase(b->reference());
}

} // namespace Heightmap
