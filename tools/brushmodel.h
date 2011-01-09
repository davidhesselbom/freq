#pragma once

// other tools
#include "rendermodel.h"

// tool support
#include "support/brushfilter.h"
#include "support/brushpaint.cu.h"

// Sonic AWE
#include "heightmap/reference.h"
#include "tfr/filter.h"


namespace Tools
{

/*
 A continous brush stroke results in an image. This image is then applied upon
 the data through a filter which interprets what to do with the image data.
 */
class BrushModel
{
public:
    BrushModel( Sawe::Project* project, RenderModel* render_model );

    /**
      Get the BrushFilter.
      */
    Support::BrushFilter* filter();

    float brush_factor;

    /**
      Defaults to 1
      */
    float std_t;

    Signal::Interval paint( Heightmap::Reference ref, Heightmap::Position pos );

    Gauss getGauss( Heightmap::Reference ref, Heightmap::Position pos );
private:
    RenderModel* render_model_;
    Signal::pOperation filter_;

    Signal::Interval addGauss( Heightmap::Reference ref, Gauss gauss );
};


} // namespace Tools
