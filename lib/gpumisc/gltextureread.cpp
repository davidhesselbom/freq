#include "gltextureread.h"
#include "gl.h"
#include "GlTexture.h"
#include "log.h"
#include "GlException.h"
#include "glframebuffer.h"

#include <QApplication>
#include <QGLWidget>

// https://www.opengl.org/sdk/docs/man/xhtml/glGetTexImage.xml


GlTextureRead::
        GlTextureRead(int texture)
    :
      texture(texture)
{
    EXCEPTION_ASSERT(texture);
}


DataStorage<float>::ptr GlTextureRead::
        readFloat(int level, int format)
{
    // assumes GL_PIXEL_PACK_BUFFER_BINDING is 0
    GLint width=0, height=0, pack_alignment=0;

    GlException_SAFE_CALL( glBindTexture (GL_TEXTURE_2D, texture) );
    GlException_SAFE_CALL( glGetTexLevelParameteriv (GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width) );
    GlException_SAFE_CALL( glGetTexLevelParameteriv (GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &height) );
    GlException_SAFE_CALL( glGetIntegerv (GL_PACK_ALIGNMENT, &pack_alignment) );
    GlException_SAFE_CALL( glPixelStorei (GL_PACK_ALIGNMENT, 4) );

    EXCEPTION_ASSERT_LESS( 0, width );
    EXCEPTION_ASSERT_LESS( 0, height );

    int number_of_components = 0;
    switch(format) {
    case GL_RGBA: number_of_components = 4; break;
    case GL_RED: number_of_components = 1; break;
    default: EXCEPTION_ASSERTX(false, "Unsupported format"); break;
    }

    DataStorage<float>::ptr data(new DataStorage<float>(width*number_of_components, height));


    // Straightforward, but unstable
    //GlException_SAFE_CALL( glGetTexImage(GL_TEXTURE_2D, level, format, GL_FLOAT, data->getCpuMemory()) );


    // Read through FBO instead
    GlFrameBuffer fb(texture);

    GlFrameBuffer::ScopeBinding fbobinding = fb.getScopeBinding();
    unsigned pbo=0;
    glGenBuffers (1, &pbo);
    glBindBuffer (GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData (GL_PIXEL_PACK_BUFFER, data->numberOfBytes (), NULL, GL_STREAM_READ);

    glReadPixels (0, 0, width, height, format, GL_FLOAT, 0);
    glBindBuffer (GL_PIXEL_PACK_BUFFER, 0);

    glBindBuffer (GL_PIXEL_PACK_BUFFER, pbo);
    float *src = (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    memcpy(data->getCpuMemory(), src, data->numberOfBytes ());
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
    glBindBuffer (GL_PIXEL_PACK_BUFFER, 0);
    glDeleteBuffers (1, &pbo);


    // restore
    GlException_SAFE_CALL( glPixelStorei (GL_PACK_ALIGNMENT, pack_alignment) );
    GlException_SAFE_CALL( glBindTexture (GL_TEXTURE_2D, 0) );

    return data;
}


DataStorage<unsigned char>::ptr GlTextureRead::
        readByte(int level, int format)
{
    // assumes GL_PIXEL_PACK_BUFFER_BINDING is 0
    GLint width=0, height=0, pack_alignment=0;

    GlException_SAFE_CALL( glBindTexture (GL_TEXTURE_2D, texture) );
    GlException_SAFE_CALL( glGetTexLevelParameteriv (GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width) );
    GlException_SAFE_CALL( glGetTexLevelParameteriv (GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &height) );
    GlException_SAFE_CALL( glGetIntegerv (GL_PACK_ALIGNMENT, &pack_alignment) );
    GlException_SAFE_CALL( glPixelStorei (GL_PACK_ALIGNMENT, 1) );

    EXCEPTION_ASSERT_LESS( 0, width );
    EXCEPTION_ASSERT_LESS( 0, height );

    int number_of_components = 0;
    switch(format) {
    case GL_RGBA: number_of_components = 4; break;
    case GL_RED: number_of_components = 1; break;
    default: EXCEPTION_ASSERTX(false, "Unsupported format"); break;
    }

    DataStorage<unsigned char>::ptr data(new DataStorage<unsigned char>(width*number_of_components, height));


    // Straightforward, but unstable
    //GlException_SAFE_CALL( glGetTexImage(GL_TEXTURE_2D, level, format, GL_UNSIGNED_BYTE, data->getCpuMemory()) );


    // Read through FBO instead
    GlFrameBuffer fb(texture);

    GlFrameBuffer::ScopeBinding fbobinding = fb.getScopeBinding();
    unsigned pbo=0;
    glGenBuffers (1, &pbo);
    glBindBuffer (GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData (GL_PIXEL_PACK_BUFFER, data->numberOfBytes (), NULL, GL_STREAM_READ);
    glReadPixels (0, 0, width, height, format, GL_UNSIGNED_BYTE, 0);
    glBindBuffer (GL_PIXEL_PACK_BUFFER, 0);

    glBindBuffer (GL_PIXEL_PACK_BUFFER, pbo);
    float *src = (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    memcpy(data->getCpuMemory(), src, data->numberOfBytes ());
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
    glBindBuffer (GL_PIXEL_PACK_BUFFER, 0);
    glDeleteBuffers (1, &pbo);



    // restore
    GlException_SAFE_CALL( glPixelStorei (GL_PACK_ALIGNMENT, pack_alignment) );
    GlException_SAFE_CALL( glBindTexture (GL_TEXTURE_2D, 0) );

    return data;
}


template<typename T>
void compare(T* expected, size_t sizeof_expected, typename DataStorage<T>::ptr data)
{
    EXCEPTION_ASSERT_EQUALS(sizeof_expected, data->numberOfBytes ());

    T *p = data->getCpuMemory ();

    if (0 != memcmp(p, expected, sizeof_expected))
    {
        Log("%s") % data->numberOfElements ();
        for (size_t i=0; i<data->numberOfElements (); i++)
            Log("%s: %s\t%s\t%s") % i % ((double)p[i]) % ((double)expected[i]) % (double)(p[i] - expected[i]);

        EXCEPTION_ASSERT_EQUALS(0, memcmp(p, expected, sizeof_expected));
    }
}


void GlTextureRead::
        test()
{
    std::string name = "GlTextureRead";
    int argc = 1;
    char * argv = &name[0];
    QApplication a(argc,&argv);
    QGLWidget w;
    w.makeCurrent ();

    // It should read the contents of an OpenGL texture.
    {
        // There must be a current OpenGL context
        EXCEPTION_ASSERT(QGLContext::currentContext ());

        const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
        EXCEPTION_ASSERT(extensions);

        bool hasTextureFloat = 0 != strstr( extensions, "GL_ARB_texture_float" );
        EXCEPTION_ASSERT(hasTextureFloat);

        float srcdata[]={.1,.2,.3,.4,
                         .2,.3,.4,.5,
                         .3,.4,.5,.6,
                         .4,.5,.6,2.7};

        unsigned char bytes[]={ 26, 51, 77, 102,
                                51, 77, 102, 128,
                                77, 102, 128, 153,
                                102, 128, 153, 255 };

        unsigned char bytesRgba[]={ 26,  0,0,255, 51,  0,0,255, 77,  0,0,255, 102, 0,0,255,
                                    51,  0,0,255, 77,  0,0,255, 102, 0,0,255, 128, 0,0,255,
                                    77,  0,0,255, 102, 0,0,255, 128, 0,0,255, 153, 0,0,255,
                                    102, 0,0,255, 128, 0,0,255, 153, 0,0,255, 255, 0,0,255 };

        GlTexture src(4, 4, GL_RED, GL_RGBA, GL_FLOAT, srcdata);

        DataStorage<float>::ptr data = GlTextureRead(src.getOpenGlTextureId ()).readFloat (0,GL_RED);
        compare(srcdata, sizeof(srcdata), data);

        DataStorage<unsigned char>::ptr databyte = GlTextureRead(src.getOpenGlTextureId ()).readByte (0,GL_RED);
        compare(bytes, sizeof(bytes), databyte);

        databyte = GlTextureRead(src.getOpenGlTextureId ()).readByte ();
        compare(bytesRgba, sizeof(bytesRgba), databyte);
    }
}
