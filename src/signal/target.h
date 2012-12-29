#ifndef LAYERS_H
#define LAYERS_H

#include "chain.h"
#include "reroutechannels.h"

#include <boost/serialization/set.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace Sawe { class Project; }

namespace Signal {

/**
  Collection of all chains.
  */
class Layers
{
public:
    Layers(Sawe::Project* project);
    ~Layers();

    Sawe::Project* project();

    const std::set<pChain>& layers();

    void addLayer(pChain);
    void removeLayer(pChain);

    bool isInSet(pChain) const;

    std::string toString() const;

private:
    std::set<pChain> layers_;
    Sawe::Project* project_;

    friend class boost::serialization::access;
    Layers() { EXCEPTION_ASSERT(false); } // required for serialization to compile, is never called
    template<class Archive> void serialize(Archive& ar, const unsigned int /*version*/) {
        TaskInfo ti("Layers::serialize");
        ar & BOOST_SERIALIZATION_NVP( layers_);
    }
};


/**
  post_sink() is given to worker.
  post_sink can be asked for invalid samples so that worker knows what to work on.
  worker still needs to be told if worker should start from 0 or start at the middle.

  chain has a post_sink in which targets register themselves as sinks so that
  invalidate samples can propagate through.

  when worker reads from post_sink post_sink has its source setup so that it
  will merge the results from all chains using OperationSuperposition.

  it could also make a setup so that it puts the results from each layer in its
  own channel.

  the OperationSuperposition structure is rebuilt whenever addLayer, removeLayer
  or addAsChannels is called.

  Default allow_cheat_resolution = false
  Default autocreate_chainheads = true
  */
class Target {
public:
    Target(Layers* all_layers, std::string name, bool allow_cheat_resolution, bool autocreate_chainheads);
    ~Target();

    /**
     * @brief Returns the project that owns this Target.
     * TODO Target should preferably be independent of project.
     * @return
     */
    Sawe::Project* project();

    /**
      //It is an error to add a layer that is not in 'all_layers_'
      */
    void addLayerHead(pChainHead);

    /**
      It is an error to remove a layer that is not in 'layerHeads'
      */
    void removeLayerHead(pChainHead);

    /**
      Returns the chain head that corresponds to a chain, if there is one.
      */
    pChainHead findHead( pChain );

    /**
      layer merging doesn't have to be done by superpositioning, it could also
      make a setup so that it puts the results from each layer in its
      own channel
      */
    void addAsChannels(bool add_as_channels);

    /**
      Add sinks to this target group through this PostSink
      */
    PostSink*   post_sink() const;

    /**
      Select which channels that gets to propagate through with anything else
      than silence.
      */
    RerouteChannels* channels() const;

    /**
      Will process all active channels (as wired by channels()) and return the
      buffer from the last channel.
      */
    pBuffer read( const Interval& I );

    /**
      The name that was given when the target was created.
      */
    std::string name();

    /**
      */
    pOperation source() const;

    /**
      */
    bool allow_cheat_resolution() const;

    /**
      */
    void allow_cheat_resolution(bool);

    /**
      */
    unsigned next_good_size( unsigned current_valid_samples_per_chunk );

    /**
      */
    unsigned prev_good_size( unsigned current_valid_samples_per_chunk );

    /**
     */
    pChainHead main_chain_head();

private:
    void rebuildSource();

    std::string name_;
    Signal::pOperation post_sink_;
    Signal::pOperation reroute_channels_;
    Signal::pOperation update_view_;
    Signal::pOperation cache_vars_;
    Signal::pOperation read_;
    bool add_as_channels_;
    bool allow_cheat_resolution_;
    Layers* all_layers_;
    std::set<pChainHead> layerHeads;

    bool isInSet(pChain) const;
};
typedef boost::shared_ptr<Target> pTarget;


/**
  * Not used
  */
class OperationTarget: public Target {
public:
    /**
      * @param all_layers Pass &Sawe::Projecct::layers
      */
    OperationTarget(Layers* all_layers, pOperation operation, std::string name, bool allow_cheat_resolution);

private:
    void addOperation(pOperation operation);
};

} // namespace Signal

#endif // LAYERS_H
