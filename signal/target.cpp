#include "target.h"

#include "operation-basic.h"

#include "sawe/project.h"
#include "tools/renderview.h"

#include <boost/foreach.hpp>

#define DEBUG_Target if(0)
//#define DEBUG_Target

namespace Signal {

class OperationAddChannels: public Operation
{
public:
    OperationAddChannels( pOperation source, pOperation source2 )
        :
        Operation(source),
        source2_(source2),
        current_channel_(0)
    {
    }

    virtual pBuffer read( const Interval& I )
    {
        if (current_channel_< source()->num_channels())
            return source()->read( I );
        else
            return source2_->read( I );
    }

    virtual pOperation source2() const { return source2_; }

    virtual unsigned num_channels() { return source()->num_channels() + source2_->num_channels(); }
    virtual void set_channel(unsigned c) {
        BOOST_ASSERT( c < num_channels() );
        if (c < source()->num_channels())
            source()->set_channel(c);
        else
            source2_->set_channel(c - source()->num_channels());
        current_channel_ = c;
    }
    virtual unsigned get_channel() { return current_channel_; }

private:
    pOperation source2_;
    unsigned current_channel_;
};


class ForAllChannelsOperation: public Operation
{
public:
    ForAllChannelsOperation
        (
            Signal::pOperation o
        )
            :
        Operation(o)
    {
    }


    virtual pBuffer read( const Interval& I )
    {
        unsigned N = num_channels();
        Signal::pBuffer r;
        for (unsigned i=0; i<N; ++i)
        {
            set_channel( i );
            r = Signal::Operation::read( I );
        }

        return r;
    }


    virtual void invalidate_samples(const Intervals& I)
    {
        unsigned N = num_channels();
        if (0 < N)
        {
            if (get_channel() >= N)
                set_channel(N - 1);
        }

        Operation::invalidate_samples( I );
    }
};


class UpdateView: public Operation
{
public:
    UpdateView(Sawe::Project* project, std::string targetname)
        :
        Operation(pOperation()),
        project_(project),
        targetname_(targetname)
    {
    }


    virtual std::string name()
    {
        return Operation::name() + " for '" + targetname_ + "' in '" + project_->project_title() + "'";
    }


    virtual void invalidate_samples(const Intervals& I)
    {
        if (project_->areToolsInitialized())
            project_->tools().render_view()->userinput_update( false );

        Operation::invalidate_samples(I);
    }


private:
    Sawe::Project* project_;
    std::string targetname_;
};


Layers::
        Layers(Sawe::Project* project)
            :
            project_(project)
{
}


Layers::
        ~Layers()
{
    TaskInfo ti("~Layers");
    layers_.clear();
}


Sawe::Project* Layers::
        project()
{
    return project_;
}


std::set<pChain> Layers::
        layers()
{
    return layers_;
}


void Layers::
        addLayer(pChain p )
{
    BOOST_ASSERT( !isInSet(p) );
    layers_.insert( p );
}


void Layers::
        removeLayer(pChain p)
{
    BOOST_ASSERT( isInSet(p) );
    layers_.erase( p );
}


bool Layers::
        isInSet(pChain p) const
{
    return layers_.find( p ) != layers_.end();
}


std::string Layers::
        toString() const
{
    std::string s;
    for (std::set<pChain>::iterator itr = layers_.begin(); itr != layers_.end(); ++itr)
    {
        s += (*itr)->tip_source()->toString();
        s += "\n\n";
    }
    return s;
}


Target::
        Target(Layers* all_layers, std::string name)
            :
            name_( name ),
            post_sink_( new PostSink ),
            rewire_channels_( new RewireChannels(pOperation()) ),
            forall_channels_( new ForAllChannelsOperation(pOperation()) ),
            update_view_( new UpdateView( all_layers->project(), name )),
            add_as_channels_(false),
            all_layers_(all_layers)
{
    post_sink_->source( rewire_channels_ );
    forall_channels_->source( post_sink_ );
    update_view_->source( forall_channels_ );
    read_ = update_view_;

    BOOST_FOREACH( pChain c, all_layers_->layers() )
    {
        /*if (all_layers->project()->head->chain() == c)
            addLayerHead(all_layers->project()->head);
        else*/
            addLayerHead( pChainHead(new ChainHead(c)));
    }
}


void Target::
        addLayerHead(pChainHead p)
{
    BOOST_ASSERT( all_layers_ );
    BOOST_ASSERT( !isInSet(p->chain()) );
    BOOST_ASSERT( all_layers_->isInSet(p->chain()) );

    Signal::Intervals was_zero = read_->zeroed_samples_recursive();

    layerHeads.insert( p );
    rebuildSource();

    Signal::Intervals is_zero = read_->zeroed_samples_recursive();
    Signal::Intervals need_update = Signal::Intervals::Intervals_ALL - (was_zero&is_zero);

    post_sink()->invalidate_samples( need_update );
}


void Target::
        removeLayerHead(pChainHead p)
{
    BOOST_ASSERT( all_layers_ );
    BOOST_ASSERT( isInSet(p->chain()) );
    BOOST_ASSERT( all_layers_->isInSet(p->chain()) );

    Signal::Intervals was_zero = read_->zeroed_samples_recursive();

    layerHeads.erase( p );
    rebuildSource();

    Signal::Intervals is_zero = read_->zeroed_samples_recursive();
    Signal::Intervals need_update = Signal::Intervals::Intervals_ALL - (was_zero&is_zero);

    post_sink()->invalidate_samples( need_update );
}


pChainHead Target::
        findHead( pChain c )
{
    BOOST_FOREACH( pChainHead p, layerHeads )
    {
        if (c == p->chain())
            return p;
    }

    return pChainHead();
}


void Target::
        addAsChannels(bool add_as_channels)
{
    if (add_as_channels_ == add_as_channels)
        return;

    add_as_channels_ = add_as_channels;

    rebuildSource();

    post_sink()->invalidate_samples( ~post_sink()->zeroed_samples_recursive() );
}


PostSink* Target::
        post_sink() const
{
    return dynamic_cast<PostSink*>(post_sink_.get());
}


RewireChannels* Target::
        channels() const
{
    return dynamic_cast<RewireChannels*>(rewire_channels_.get());
}


pBuffer Target::
        read( const Interval& I )
{
    DEBUG_Target TaskInfo("Target \"%s\", reading\n%s", name_.c_str(), read_->toString().c_str());
    return read_->read( I );
}


std::string Target::
        name()
{
    return name_;
}


pOperation Target::
        source() const
{
    return read_;
}


void Target::
        rebuildSource()
{
    pOperation s;

    BOOST_FOREACH( pChainHead p, layerHeads )
    {
        if (!s)
        {
            s = p->head_source_ref();
        }
        else
        {
            if (add_as_channels_)
                s.reset(new OperationAddChannels(s, p->head_source_ref()));
            else
                s.reset(new OperationSuperposition(s, p->head_source_ref()));
        }
    }

    rewire_channels_->source( s );

    DEBUG_Target TaskInfo("Target::rebuildSource created\n%s", read_->toString().c_str());
}


bool Target::
        isInSet(pChain c) const
{
    BOOST_FOREACH( pChainHead p, layerHeads )
    {
        if (p->chain() == c)
            return true;
    }
    return false;
}

} // namespace Signal