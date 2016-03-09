/**
See class comment #GlTexture.
*/

#pragma once

#include "releaseaftercontext.h"
#include <memory>

// It's not necessary to include the whole glew.h here just to get 
// the constant value of GL_RGBA, which is used as the default value
// for some arguments in GlTexture.
#ifndef GL_RGBA 
#define GL_RGBA 0x1908
#else
#if GL_RGBA != 0x1908
#error GL_RGBA is different from 0x1908
#endif
#endif

#ifndef GL_UNSIGNED_BYTE
#define GL_UNSIGNED_BYTE 0x1401
#else
#if GL_UNSIGNED_BYTE != 0x1401
#error GL_UNSIGNED_BYTE is different from 0x1401
#endif
#endif


/**
GlTexture is a wrapper for an OpenGL texture (refered to as just 
"texture") to manage the texture in an object oriented manner (mainly
construction and destruction and keeping track of texture id).

@author johan.b.gustafsson@gmail.com
*/
class GlTexture: public boost::noncopyable {
public:
    typedef std::shared_ptr<GlTexture> ptr;

	/**
	Creates a new OpenGL texture and allocates memory for a given
	width and height using a given pixelFormat.

	@param width Requested width of the new texture.
	@param height Requested height of the new texture.
	@param pixelFormat Requested pixel format for the new texture.

	@throws GlException If OpenGL encountered an error.
	*/
    GlTexture();
    GlTexture(unsigned short width, unsigned short height);
    GlTexture(unsigned short width, unsigned short height, unsigned int pixelFormat, unsigned int internalFormat, unsigned type, void* data = 0);

    /**
     * @brief GlTexture maps an existing gl texture
     * @param textureId
     */
    GlTexture(unsigned int textureId, int width, int height, bool adopt=false);

    void reset(unsigned short width, unsigned short height, unsigned int pixelFormat=GL_RGBA, unsigned int internalFormat=GL_RGBA, unsigned type=GL_UNSIGNED_BYTE, void* data = 0);

	/**
	Releases the texture object.
	*/
	~GlTexture();

    /**
    Binds this texture with glBindTexture to GL_TEXTURE_2D.
    */
    void bindTexture();

	/**
	Returns the OpenGL texture id for this texture. To be used with
	OpenGL functions such as glBindTexture().
	*/
    unsigned int getOpenGlTextureId() const { return textureId; }

	/**
	Returns the width of the texture.
	*/
    unsigned short getWidth() const { return width; }

	/**
	Returns the height of the texture.
	*/
    unsigned short getHeight() const { return height; }

    /**
    getMinFilter returns the min_filter previously stored by storeMinFilter.
    This is used to check if the mipmap levels have been enabled for this texture.
    The texture must be bound when calling setMinFilter.
     */
    unsigned int getMinFilter() const { return min_filter; }
    void setMinFilter(unsigned int f);

private:
    /**
	Requested texture width of the texture.
	*/
	unsigned short width;

	/**
	Requested texture height of the texture.
	*/
	unsigned short height;

	/**
	OpenGL texture id for the texture.
	*/
    unsigned int textureId;
    unsigned int ownTextureId;

    unsigned int min_filter=0;
};
