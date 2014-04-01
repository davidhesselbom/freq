#ifndef HEIGHTMAP_TFRMAPPING_H
#define HEIGHTMAP_TFRMAPPING_H

#include "blocklayout.h"
#include "shared_state.h"
#include "visualizationparams.h"

#include <vector>

namespace Heightmap {
class Collection;
typedef int ChannelCount;

class TfrMapping {
public:
    typedef shared_state<TfrMapping> ptr;
    typedef shared_state<const TfrMapping> const_ptr;

    TfrMapping(BlockLayout, ChannelCount channels);
    ~TfrMapping();

    BlockLayout block_layout() const;
    void block_layout(BlockLayout bs);

    Tfr::FreqAxis display_scale() const;
    AmplitudeAxis amplitude_axis() const;
    void display_scale(Tfr::FreqAxis);
    void amplitude_axis(AmplitudeAxis);

    float targetSampleRate() const;
    void targetSampleRate(float);

    Tfr::TransformDesc::ptr transform_desc() const;
    void transform_desc(Tfr::TransformDesc::ptr);

    float length() const;
    void length(float L);

    int channels() const;
    void channels(int value);

    typedef shared_state<Heightmap::Collection> pCollection;
    typedef std::vector<pCollection> Collections;
    Collections collections() const;

private:
    void updateCollections();

    Collections                 collections_;
    BlockLayout                 block_layout_;
    VisualizationParams::ptr    visualization_params_;
    float                       length_;

public:
    static void test();
    static TfrMapping::ptr testInstance();
};

} // namespace Heightmap

#endif // HEIGHTMAP_TFRMAPPING_H
