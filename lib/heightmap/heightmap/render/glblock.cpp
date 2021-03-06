#include "gl.h"

#include "glblock.h"

// gpumisc
#include "vbo.h"
#include "demangle.h"
#include "GlException.h"
#include "cpumemorystorage.h"
#include "computationkernel.h"
#include "tasktimer.h"

// std
#include <stdio.h>


//#define INFO
#define INFO if(0)


using namespace std;


namespace Heightmap {
namespace Render {

GlBlock::
GlBlock( GlTexture::ptr tex )
:   tex_( tex ),
    _tex_height( tex->getOpenGlTextureId () )
{
    INFO TaskTimer tt("GlBlock()");

    // TODO read up on OpenGL interop in CUDA 3.0, cudaGLRegisterBufferObject is old, like CUDA 1.0 or something ;)
}


GlTexture::ptr GlBlock::
        glTexture()
{
    return tex_;
}


void GlBlock::
        updateTexture( float*p, int n )
{
    static bool hasTextureFloat = 0 != strstr( (const char*)glGetString(GL_EXTENSIONS), "GL_ARB_texture_float" );

    int w = tex_->getWidth ();
    int h = tex_->getHeight ();

    EXCEPTION_ASSERT_EQUALS(n, w*h);

    if (!hasTextureFloat)
        glPixelTransferf( GL_RED_SCALE, 0.1f );

    auto t = tex_->getScopeBinding ();
    glBindTexture(GL_TEXTURE_2D, _tex_height);
    GlException_SAFE_CALL( glTexSubImage2D(GL_TEXTURE_2D,0,0,0, w, h, hasTextureFloat?GL_LUMINANCE:GL_RED, GL_FLOAT, p) );
    glBindTexture(GL_TEXTURE_2D, 0);

    if (!hasTextureFloat)
        glPixelTransferf( GL_RED_SCALE, 1.0f );
}


bool GlBlock::
        has_texture() const
{
    return _tex_height;
}


void GlBlock::
        draw( unsigned vbo_size )
{
    if (!has_texture ())
        return;

    GlException_CHECK_ERROR();

    glBindTexture(GL_TEXTURE_2D, _tex_height);

    const bool wireFrame = false;
    const bool drawPoints = false;

    if (drawPoints) {
        glDrawArrays(GL_POINTS, 0, vbo_size);
    } else if (wireFrame) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
            glDrawElements(GL_TRIANGLE_STRIP, vbo_size, BLOCK_INDEX_TYPE, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        glDrawElements(GL_TRIANGLE_STRIP, vbo_size, BLOCK_INDEX_TYPE, 0);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    GlException_CHECK_ERROR();
}


unsigned GlBlock::
        allocated_bytes_per_element() const
{
    return 4*sizeof(float); // OpenGL texture
}

} // namespace Render
} // namespace Heightmap
