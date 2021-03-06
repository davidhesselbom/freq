#ifndef HEIGHTMAP_UPDATE_OPENGL_FBO2BLOCK_H
#define HEIGHTMAP_UPDATE_OPENGL_FBO2BLOCK_H

#include "heightmap/block.h"
#include "glframebuffer.h"

namespace Heightmap {
namespace Update {
namespace OpenGL {

class Fbo2Block {
public:
    typedef ReleaseAfterContext<Fbo2Block> ScopeBinding;

    Fbo2Block ();
    Fbo2Block (Fbo2Block&&) = default;
    Fbo2Block (const Fbo2Block&) = delete;
    Fbo2Block& operator=(const Fbo2Block&) = delete;
    ~Fbo2Block();

    ScopeBinding begin (pBlock block);

private:
    void end();

    pBlock block;
    Block::pGlBlock glblock;
    std::unique_ptr<GlFrameBuffer> fbo;
    unsigned copyfbo;
};


} // namespace OpenGL
} // namespace Update
} // namespace Heightmap

#endif // HEIGHTMAP_UPDATE_OPENGL_FBO2BLOCK_H
