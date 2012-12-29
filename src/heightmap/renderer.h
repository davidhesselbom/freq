#ifndef HEIGHTMAPRENDERER_H
#define HEIGHTMAPRENDERER_H

// Heightmap namespace
#include "reference.h"

// gpumisc
#include <tmatrix.h>

// std
#include <vector>

// boost
#include <boost/shared_ptr.hpp>

typedef tvector<3,double> GLvector;
class GlTexture;

class Vbo;
typedef boost::shared_ptr<Vbo> pVbo;

namespace Heightmap {

    GLvector gluProject(GLvector obj, const double* model, const double* proj, const int *view, bool *r=0);
    GLvector gluUnProject(GLvector win, const double* model, const double* proj, const int *view, bool *r=0);

    class Collection;

class Renderer
{
public:
    enum ColorMode {
        ColorMode_Rainbow,
        ColorMode_Grayscale,
        ColorMode_BlackGrayscale,
        ColorMode_FixedColor,
        ColorMode_GreenRed,
        ColorMode_GreenWhite,
        ColorMode_Green
    };

    Renderer( Collection* collection );

    Reference findRefAtCurrentZoomLevel( Heightmap::Position p );
    Collection* collection;

    void draw( float scaley );
    void drawAxes( float T );
    void drawFrustum();

    bool draw_piano;
    bool draw_hz;
    bool draw_t;
    bool draw_cursor_marker;
    int draw_axis_at0;
    GLvector camera, cameraRotation;
    GLvector cursor;

    bool draw_contour_plot;
    ColorMode color_mode;
    tvector<4, float> fixed_color;
    tvector<4, float> clear_color;
    float y_scale;
    float last_ysize;
    float last_axes_length;
    unsigned drawn_blocks;
    bool left_handed_axes;
    bool vertex_texture;

    void setFractionSize( unsigned divW=1, unsigned divH=1);
    bool fullMeshResolution();
    unsigned trianglesPerBlock();
    bool isInitialized();
    void init();

    double modelview_matrix[16], projection_matrix[16];
    int viewport_matrix[4];

    GLvector gluProject(GLvector obj, bool *r=0);
    GLvector gluUnProject(GLvector win, bool *r=0);

    void frustumMinMaxT( float& min_t, float& max_t);

    float redundancy();
    void redundancy(float value);

    void clearCaches();
private:
    enum LevelOfDetal {
        Lod_NeedBetterF,
        Lod_NeedBetterT,
        Lod_Ok,
        Lod_Invalid
    };

    enum InitializedLevel {
        NotInitialized,
        Initialized,
        InitializationFailed
    };

    std::vector<GLvector> clippedFrustum;
    unsigned _mesh_index_buffer;
    unsigned _mesh_width;
    unsigned _mesh_height;
    unsigned _mesh_fraction_width;
    unsigned _mesh_fraction_height;
    unsigned _vbo_size;
    pVbo _mesh_position;
    unsigned _shader_prog;
    InitializedLevel _initialized;
    bool _draw_flat;
    float _redundancy;
    bool _invalid_frustum;
    bool _drawcrosseswhen0;
    GLvector projectionPlane, projectionNormal, // for clipFrustum
        rightPlane, rightNormal,
        leftPlane, leftNormal,
        topPlane, topNormal,
        bottomPlane, bottomNormal;

    ColorMode _color_texture_colors;
    boost::shared_ptr<GlTexture> colorTexture;

    void setSize( unsigned w, unsigned h);
    void createMeshIndexBuffer(int w, int h);
    void createMeshPositionVBO(int w, int h);
    void createColorTexture(unsigned N);

    void beginVboRendering();
    void endVboRendering();

    void renderSpectrogramRef( Reference ref );
    LevelOfDetal testLod( Reference ref );
    bool renderChildrenSpectrogramRef( Reference ref );
    void renderParentSpectrogramRef( Reference ref );
    bool computePixelsPerUnit( Reference ref, float& timePixels, float& scalePixels );
    void computeUnitsPerPixel( GLvector p, GLvector::T& timePerPixel, GLvector::T& scalePerPixel );

    std::vector<GLvector> clipFrustum( GLvector corner[4], GLvector &closest_i, float w=0, float h=0 );
    std::vector<GLvector> clipFrustum( std::vector<GLvector> l, GLvector &closest_i, float w=0, float h=0 );
};
typedef boost::shared_ptr<Renderer> pRenderer;

} // namespace Heightmap

#endif // HEIGHTMAPRENDERER_H
