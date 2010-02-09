#include "GL/glew.h"
#include <stdio.h>
#include "spectrogram-renderer.h"
#include "spectrogram-vbo.h"
#include <list>
#include <GlException.h>
#include <CudaException.h>
#include <boost/array.hpp>
#include <tmatrix.h>

using namespace std;

static bool g_invalidFrustum = true;

SpectrogramRenderer::SpectrogramRenderer( pSpectrogram spectrogram )
:   _spectrogram(spectrogram),
    _mesh_index_buffer(0),
    _mesh_width(0),
    _mesh_height(0),
    _initialized(false),
    _redundancy(2) // 1 means every pixel gets its own vertex, 10 means every 10th pixel gets its own vertex
{
}

void SpectrogramRenderer::setSize( unsigned w, unsigned h)
{
    if (w == _mesh_width && h ==_mesh_height)
        return;

    createMeshIndexBuffer(w, h);
    createMeshPositionVBO(w, h);
}

// create index buffer for rendering quad mesh
void SpectrogramRenderer::createMeshIndexBuffer(unsigned w, unsigned h)
{
    // create index buffer
    if (_mesh_index_buffer)
        glDeleteBuffersARB(1, &_mesh_index_buffer);

    _mesh_width = w;
    _mesh_height = h;

    int size = ((w*2)+2)*(h-1)*sizeof(GLuint);
    glGenBuffersARB(1, &_mesh_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh_index_buffer);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);

    // fill with indices for rendering mesh as triangle strips
    GLuint *indices = (GLuint *) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (!indices) {
        return;
    }

    for(unsigned y=0; y<h-1; y++) {
        for(unsigned x=0; x<w; x++) {
            *indices++ = y*w+x;
            *indices++ = (y+1)*w+x;
        }
        // start new strip with degenerate triangle
        *indices++ = (y+1)*w+(w-1);
        *indices++ = (y+1)*w;
    }

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// create fixed vertex buffer to store mesh vertices
void SpectrogramRenderer::createMeshPositionVBO(unsigned w, unsigned h)
{
    _mesh_position.reset( new Vbo( w*h*4*sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, *_mesh_position);
    float *pos = (float *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (!pos) {
        return;
    }

    for(unsigned y=0; y<h; y++) {
        for(unsigned x=0; x<w; x++) {
            float u = x / (float) (w-1);
            float v = y / (float) (h-1);
            *pos++ = u;
            *pos++ = 0.0f;
            *pos++ = v;
            *pos++ = 1.0f;
        }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


typedef tvector<3,GLdouble> GLvector;
typedef tvector<4,GLdouble> GLvector4;
typedef tmatrix<4,GLdouble> GLmatrix;

static GLvector4 to4(const GLvector& a) { return GLvector4(a[0], a[1], a[2], 1);}
static GLvector to3(const GLvector4& a) { return GLvector(a[0], a[1], a[2]);}

template<typename f>
GLvector gluProject(tvector<3,f> obj, const GLdouble* model, const GLdouble* proj, const GLint *view, bool *r=0) {
    GLvector win;
    bool s = (GLU_TRUE == gluProject(obj[0], obj[1], obj[2], model, proj, view, &win[0], &win[1], &win[2]));
    if(r) *r=s;
    return win;
}

template<typename f>
GLvector gluUnProject(tvector<3,f> win, const GLdouble* model, const GLdouble* proj, const GLint *view, bool *r=0) {
    GLvector obj;
    bool s = (GLU_TRUE == gluUnProject(win[0], win[1], win[2], model, proj, view, &obj[0], &obj[1], &obj[2]));
    if(r) *r=s;
    return obj;
}

template<typename f>
GLvector gluProject(tvector<3,f> obj, bool *r=0) {
    GLdouble model[16], proj[16];
    GLint view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    return gluProject(obj, model, proj, view, r);
}

template<typename f>
GLvector gluProject2(tvector<3,f> obj, bool *r=0) {
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    GLvector4 eye = applyProjectionMatrix(applyModelMatrix(to4(obj)));
    eye[0]/=eye[3];
    eye[1]/=eye[3];
    eye[2]/=eye[3];

    GLvector screen(view[0] + (eye[0]+1)*view[2]/2, view[1] + (eye[1]+1)*view[3]/2, eye[2]);
    float dummy=43*23;
    return screen;
}

template<typename f>
GLvector gluUnProject(tvector<3,f> win, bool *r=0) {
    GLdouble model[16], proj[16];
    GLint view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    return gluUnProject(win, model, proj, view, r);
}

static bool validWindowPos(GLvector win) {
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);

    return win[0]>view[0] && win[1]>view[1]
            && win[0]<view[0]+view[2]
            && win[1]<view[1]+view[3]
            && win[2]>=0.1 && win[2]<=100;
}

static GLvector4 applyModelMatrix(GLvector4 obj) {
    GLdouble m[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, m);

    GLvector4 eye = GLmatrix(m) * obj;
    return eye;
}

static GLvector4 applyProjectionMatrix(GLvector4 eye) {
    GLdouble p[16];
    glGetDoublev(GL_PROJECTION_MATRIX, p);

    GLvector4 clip = GLmatrix(p) * eye;
    return clip;
}

static bool inFrontOfCamera2( GLvector obj ) {
    GLvector4 eye = applyModelMatrix(to4(obj));
    GLvector4 clip = applyProjectionMatrix(eye);

    return clip[2] > 0.1;
}

static bool inFrontOfPlane( GLvector obj, const GLvector& plane, const GLvector& normal ) {
    return (plane-obj)%normal < 0;
}

static bool inFrontOfCamera( GLvector obj ) {
    // need to comply exactly with cameraIntersection
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    GLvector projectionPlane = gluUnProject( GLvector( view[0] + view[2]/2, view[1] + view[3]/2, .2) );
    GLvector projectionNormal = (gluUnProject( GLvector( view[0] + view[2]/2, view[1] + view[3]/2, 1) ) - projectionPlane).Normalize();

    return inFrontOfPlane( obj, projectionPlane, projectionNormal);
}

class glPushMatrixContext
{
public:
    glPushMatrixContext() { glPushMatrix(); }
    ~glPushMatrixContext() { glPopMatrix(); }
};

void SpectrogramRenderer::init()
{
    // initialize necessary OpenGL extensions
    GlException_CHECK_ERROR_MSG("1");

    int cudaDevices;
    CudaException_SAFE_CALL( cudaGetDeviceCount( &cudaDevices) );
    if (0 == cudaDevices ) {
        fprintf(stderr, "ERROR: Couldn't find any \"cuda capable\" device.");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    if (0 != glewInit() ) {
        fprintf(stderr, "ERROR: Couldn't initialize \"glew\".");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    if (! glewIsSupported("GL_VERSION_2_0" )) {
        fprintf(stderr, "ERROR: Support for necessary OpenGL extensions missing.");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    if (!glewIsSupported( "GL_VERSION_1_5 GL_ARB_vertex_buffer_object GL_ARB_pixel_buffer_object" )) {
            fprintf(stderr, "Error: failed to get minimal extensions\n");
            fprintf(stderr, "Sonic AWE requires:\n");
            fprintf(stderr, "  OpenGL version 1.5\n");
            fprintf(stderr, "  GL_ARB_vertex_buffer_object\n");
            fprintf(stderr, "  GL_ARB_pixel_buffer_object\n");
            fflush(stderr);
            exit(-1);
    }

    // load shader
    _shader_prog = loadGLSLProgram("spectrogram.vert", "spectrogram.frag");

    setSize( _spectrogram->samples_per_block(), _spectrogram->scales_per_block() );

    _initialized=true;

    GlException_CHECK_ERROR_MSG("2");
}

void SpectrogramRenderer::draw()
{
    if (!_initialized) init();

    g_invalidFrustum = true;

    glPushMatrixContext();

    glScalef( 10, 1, 5 );

//    boost::shared_ptr<Spectrogram_chunk> transform = _spectrogram->getWavelettTransform();

    // find camera position
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);

    GLvector wCam = gluUnProject(GLvector(view[2]/2, view[3]/2, -1e100) ); // approx==camera position

    // as we
    // 1) never tilt the camera around the z-axes
    // 2) x-rotation is performed before y-rotation
    // 3) never rotate more than 90 degrees around x
    // => the closest point in the plane from the camera is on the line: (view[2]/2, view[3])-(view[2]/2, view[3]/2)
    // Check if the point right beneath the camera is visible

    // find units per pixel right beneath camera
    // wC is the point on screen that renders the closest pixel of the plane

    GLvector wStart = GLvector(wCam[0],0,wCam[2]);
    float length = _spectrogram->transform()->original_waveform()->length();
    if (wStart[0]<0) wStart[0]=0;
    if (wStart[2]<0) wStart[2]=0;
    if (wStart[0]>length) wStart[0]=length;
    if (wStart[2]>1) wStart[2]=1;

    GLvector sBase      = gluProject(wStart);
    sBase[2] = 0.1;

    if (!validWindowPos(sBase)) {
        // point not visible => the pixel closest to the plane is somewhere along the border of the screen
        // take bottom of screen instead
        if (wCam[1] > 0)
            sBase = GLvector(view[0]+view[2]/2, view[1], 0.1);
        else // or top of screen if beneath
            sBase = GLvector(view[0]+view[2]/2, view[1]+view[3], 0.1);

/*        GLvector wBase = gluUnProject( sBase ),
        if (wBase[0]<0 || )
                        sBase = GLvector(view[0]+view[2]/2, view[1]+view[3], 0.1);
wStart[0]=0;
        if (wStart[2]<0) wStart[2]=0;
        if (wStart[0]>length) wStart[0]=length;
        if (wStart[2]>1) wStart[2]=1;*/
    }

    // find world coordinates of projection surface
    GLvector
            wBase = gluUnProject( sBase ),
            w1 = gluUnProject(sBase + GLvector(1,0,0) ),
            w2 = gluUnProject(sBase + GLvector(0,1,0) );

    // directions
    GLvector
            dirBase = wBase-wCam,
            dir1 = w1-wCam,
            dir2 = w2-wCam;

    // valid projection on xz-plane exists if dir?[1]<0 wBase[1]<0
    GLvector
            xzBase = wCam - dirBase*(wCam[1]/dirBase[1]),
            xz1 = wCam - dir1*(wCam[1]/dir1[1]),
            xz2 = wCam - dir2*(wCam[1]/dir2[1]);

    // compute {units in xz-plane} per {screen pixel}, that determines the required resolution
    GLdouble
            timePerPixel = 0,
            freqPerPixel = 0;

    if (dir1[1] != 0 && dir2[1] != 0) {
        timePerPixel = max(timePerPixel, fabs(xz1[0]-xz2[0]));
        freqPerPixel = max(freqPerPixel, fabs(xz1[2]-xz2[2]));
    }
    if (dir1[1] != 0 && dirBase[1] != 0) {
        timePerPixel = max(timePerPixel, fabs(xz1[0]-xzBase[0]));
        freqPerPixel = max(freqPerPixel, fabs(xz1[2]-xzBase[2]));
    }
    if (dir2[1] != 0 && dirBase[1] != 0) {
        timePerPixel = max(timePerPixel, fabs(xz2[0]-xzBase[0]));
        freqPerPixel = max(freqPerPixel, fabs(xz2[2]-xzBase[2]));
    }

    if (0 == timePerPixel)
        timePerPixel = max(fabs(w1[0]-wBase[0]), fabs(w2[0]-wBase[0]));
    if (0 == freqPerPixel)
        freqPerPixel = max(fabs(w1[2]-wBase[2]), fabs(w2[2]-wBase[2]));

    if (0==freqPerPixel) freqPerPixel=timePerPixel;
    if (0==timePerPixel) timePerPixel=freqPerPixel;
    timePerPixel*=_redundancy;
    freqPerPixel*=_redundancy;

    Spectrogram::Reference ref = _spectrogram->findReference(Spectrogram::Position(wBase[0], wBase[2]), Spectrogram::Position(timePerPixel, freqPerPixel));

    Spectrogram::Position mss = _spectrogram->max_sample_size();
    ref = _spectrogram->findReference(Spectrogram::Position(0,0), mss);

    beginVboRendering();

    renderChildrenSpectrogramRef(ref);

/*    // This is the starting point for rendering the dataset
    while (false==renderChildrenSpectrogramRef(ref) && !ref.parent().toLarge() )
        ref = ref.parent();



    // Render its parent and parent and parent until a the largest section is found or a section is found that covers the entire viewed area
    renderParentSpectrogramRef( ref );
*/
    endVboRendering();

    GlException_CHECK_ERROR();
}

void SpectrogramRenderer::beginVboRendering()
{
    unsigned meshW = _spectrogram->samples_per_block();
    unsigned meshH = _spectrogram->scales_per_block();

    glUseProgram(_shader_prog);

    // Set default uniform variables parameters for the vertex shader
    GLuint uniHeightScale, uniChopiness, uniSize;

    uniHeightScale = glGetUniformLocation(_shader_prog, "heightScale");
    glUniform1f(uniHeightScale, 0.5f);

    uniChopiness   = glGetUniformLocation(_shader_prog, "chopiness");
    glUniform1f(uniChopiness, 1.0f);

    uniSize        = glGetUniformLocation(_shader_prog, "size");
    glUniform2f(uniSize, meshW, meshH);

    // Set default uniform variables parameters for the pixel shader
    GLuint uniDeepColor, uniShallowColor, uniSkyColor, uniLightDir;

    uniDeepColor = glGetUniformLocation(_shader_prog, "deepColor");
    glUniform4f(uniDeepColor, 0.0f, 0.0f, 0.1f, 1.0f);

    uniShallowColor = glGetUniformLocation(_shader_prog, "shallowColor");
    glUniform4f(uniShallowColor, 0.1f, 0.4f, 0.3f, 1.0f);

    uniSkyColor = glGetUniformLocation(_shader_prog, "skyColor");
    glUniform4f(uniSkyColor, 0.5f, 0.5f, 0.5f, 1.0f);

    uniLightDir = glGetUniformLocation(_shader_prog, "lightDir");
    glUniform3f(uniLightDir, 0.0f, 1.0f, 0.0f);
    // end of uniform settings

    glBindBuffer(GL_ARRAY_BUFFER, *_mesh_position);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh_index_buffer);
}

void SpectrogramRenderer::endVboRendering() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glUseProgram(0);
}

bool SpectrogramRenderer::renderSpectrogramRef( Spectrogram::Reference ref )
{
    if (!ref.containsSpectrogram())
        return false;

    Spectrogram::Position a, b;
    ref.getArea( a, b );
    glPushMatrix();
    if (1) { // matrix push/pop
        glTranslatef(a.time, 0, a.scale);
        glScalef(b.time-a.time, 1, b.scale-a.scale);

        Spectrogram::pBlock block = _spectrogram->getBlock( ref );
        if (0!=block.get()) {
            if (0 /* direct rendering */ )
                block->vbo->draw_directMode();
            else if (1 /* vbo */ )
                block->vbo->draw( this );

        } else {
            // getBlock would try to find something else if the requested block wasn't readily available.
            // If getBlock fails, we're out of memory. Indicate by not drawing the surface but only a wireframe

            glBegin(GL_LINE_LOOP );
                glVertex3f( 0, 0, 0 );
                glVertex3f( 0, 0, 1 );
                glVertex3f( 1, 0, 0 );
                glVertex3f( 1, 0, 1 );
            glEnd();
        }
    }
    glPopMatrix();

    return true;
}

bool SpectrogramRenderer::renderChildrenSpectrogramRef( Spectrogram::Reference ref )
{
    if (!ref.containsSpectrogram())
        return false;

    float timePixels, scalePixels;
    if (!computePixelsPerUnit( ref, timePixels, scalePixels))
        return false;

    fprintf(stdout, "Ref (%d,%d)\t%g\t%g\n", ref.chunk_index[0], ref.chunk_index[1], timePixels,scalePixels);
    fflush(stdout);

    GLdouble needBetterF, needBetterT;

    if (0==scalePixels)
        needBetterF = 1.01;
    else
        needBetterF = scalePixels / (_redundancy*_spectrogram->scales_per_block());
    if (0==timePixels)
        needBetterT = 1.01;
    else
        needBetterT = timePixels / (_redundancy*_spectrogram->samples_per_block());

    if ( needBetterF > needBetterT && needBetterF > 1 && ref.top().containsSpectrogram() ) {
        renderChildrenSpectrogramRef( ref.top() );
        renderChildrenSpectrogramRef( ref.bottom() );
    }
    else if ( needBetterT > 1 && ref.left().containsSpectrogram() ) {
        renderChildrenSpectrogramRef( ref.left() );
        renderChildrenSpectrogramRef( ref.right() );
    }
    else {
        renderSpectrogramRef( ref );
    }

    return true;
}

void SpectrogramRenderer::renderParentSpectrogramRef( Spectrogram::Reference ref )
{
    // Assume that ref has already been drawn, draw sibblings, and call renderParent again
    renderChildrenSpectrogramRef( ref.sibbling1() );
    renderChildrenSpectrogramRef( ref.sibbling2() );
    renderChildrenSpectrogramRef( ref.sibbling3() );

    if (!ref.parent().toLarge() )
        renderParentSpectrogramRef( ref.parent() );
}

// the normal does not need to be normalized, and back/front doesn't matter
static float linePlane( GLvector planeNormal, GLvector pt, GLvector dir ) {
    return -(pt % planeNormal)/ (dir % planeNormal);
}

static GLvector planeIntersection( GLvector planeNormal, GLvector pt, GLvector dir ) {
    return pt + dir*linePlane(planeNormal, pt, dir);
}

static GLvector planeIntersection( GLvector pt1, GLvector pt2, float &s, const GLvector& plane, const GLvector& normal ) {
    GLvector dir = pt2-pt1;

    s = ((plane-pt1)%normal)/(dir % normal);
    GLvector p = pt1 + dir * s;

//    float v = (p-plane ) % normal;
//    fprintf(stdout,"p[2] = %g, v = %g\n", p[2], v);
//    fflush(stdout);
    return p;
}

GLvector xzIntersection( GLvector pt1, GLvector pt2 ) {
    return planeIntersection( GLvector(0,1,0), pt1, pt2-pt1 );
}

static GLvector cameraIntersection( GLvector pt1, GLvector pt2, float &s ) {
    /*
      This is possible to compute without glUnProject, but not as straight-forward

    GLvector4 s1 = applyProjectionMatrix(applyModelMatrix(to4(pt1)));
    GLvector4 s2 = applyProjectionMatrix(applyModelMatrix(to4(pt2)));

    s = (0.3-s1[2]) / (s2[2]-s1[2]);
    return pt1 + (pt2-pt1)*s;

    GLvector4 pp = applyProjectionMatrix(applyModelMatrix(to4(projectionPlane)));
    //GLvector4 pp = applyModelMatrix(to4(projectionPlane));
    float v = (p-projectionPlane) % projectionNormal;

    float dummy = pp[2]/pp[3];
    float dummy2= 342*34;
*/

    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    GLvector projectionPlane = gluUnProject( GLvector( view[0] + view[2]/2, view[1] + view[3]/2, .2) );
    GLvector projectionNormal = (gluUnProject( GLvector( view[0] + view[2]/2, view[1] + view[3]/2, 1) ) - projectionPlane).Normalize();

    return planeIntersection(pt1, pt2, s, projectionPlane, projectionNormal);
}


GLvector xzIntersection2( GLvector pt1, GLvector pt2, float &s ) {
    s = -pt1[1] / (pt2[1]-pt1[1]);
    return pt1 + (pt2-pt1)*s;
}

void updateBounds(float bounds[4], GLvector p) {
    if (bounds[0] > p[0]) bounds[0] = p[0];
    if (bounds[2] < p[0]) bounds[2] = p[0];
    if (bounds[1] > p[1]) bounds[1] = p[1];
    if (bounds[3] < p[1]) bounds[3] = p[1];
}

std::vector<GLvector> clipPlane( std::vector<GLvector> p, GLvector p0, GLvector n ) {
    std::vector<GLvector> r;

    bool valid[4], visible = false;
    for (unsigned i=0; i<p.size(); i++)
        visible |= (valid[i] = inFrontOfPlane( p[i], p0, n ));

    // Clipp ref square with projection plane
    for (unsigned i=0; i<p.size(); i++) {
        int nexti=(i+1)%p.size();
        if (valid[i])
            r.push_back( p[i] );
        if (valid[i] !=  valid[nexti]) {
            float s=NAN;
            GLvector xy = planeIntersection( p[i], p[nexti], s, p0, n );
            if (!isnan(s) && 0 < s && s < 1)
                r.push_back( xy );
        }
    }

    return r;
}

void printl(const char* str, const std::vector<GLvector>& l) {
    fprintf(stdout,"%s\n",str);
    for (unsigned i=0; i<l.size(); i++) {
        fprintf(stdout,"  %g\t%g\t%g\n",l[i][0],l[i][1],l[i][2]);
    }
    fflush(stdout);
}


std::vector<GLvector> clipFrustum( std::vector<GLvector> l ) {

    static GLvector projectionPlane, projectionNormal,
        rightPlane, rightNormal,
        leftPlane, leftNormal,
        topPlane, topNormal,
        bottomPlane, bottomNormal;

    if (g_invalidFrustum) {
        GLint view[4];
        glGetIntegerv(GL_VIEWPORT, view);
        float z0 = .1, z1=1;
        g_invalidFrustum = false;

        projectionPlane = gluUnProject( GLvector( view[0] + view[2]/2, view[1] + view[3]/2, z0) );
        projectionNormal = (gluUnProject( GLvector( view[0] + view[2]/2, view[1] + view[3]/2, z1) ) - projectionPlane).Normalize();

        rightPlane = gluUnProject( GLvector( view[0] + view[2], view[1] + view[3]/2, z0) );
        GLvector rightZ = gluUnProject( GLvector( view[0] + view[2], view[1] + view[3]/2, z1) );
        GLvector rightY = gluUnProject( GLvector( view[0] + view[2], view[1] + view[3]/2+1, z0) );
        rightNormal = ((rightY-rightPlane)^(rightZ-rightPlane)).Normalize();

        leftPlane = gluUnProject( GLvector( view[0], view[1] + view[3]/2, z0) );
        GLvector leftZ = gluUnProject( GLvector( view[0], view[1] + view[3]/2, z1) );
        GLvector leftY = gluUnProject( GLvector( view[0], view[1] + view[3]/2+1, z0) );
        leftNormal = ((leftZ-leftPlane)^(leftY-leftPlane)).Normalize();

        topPlane = gluUnProject( GLvector( view[0] + view[2]/2, view[1] + view[3], z0) );
        GLvector topZ = gluUnProject( GLvector( view[0] + view[2]/2, view[1] + view[3], z1) );
        GLvector topX = gluUnProject( GLvector( view[0] + view[2]/2+1, view[1] + view[3], z0) );
        topNormal = ((topZ-topPlane)^(topX-topPlane)).Normalize();

        bottomPlane = gluUnProject( GLvector( view[0] + view[2]/2, view[1], z0) );
        GLvector bottomZ = gluUnProject( GLvector( view[0] + view[2]/2, view[1], z1) );
        GLvector bottomX = gluUnProject( GLvector( view[0] + view[2]/2+1, view[1], z0) );
        bottomNormal = ((bottomX-bottomPlane)^(bottomZ-bottomPlane)).Normalize();
    }

    //printl("Start",l);
    l = clipPlane(l, projectionPlane, projectionNormal);
    //printl("Projectionclipped",l);
    l = clipPlane(l, rightPlane, rightNormal);
    //printl("Right", l);
    l = clipPlane(l, leftPlane, leftNormal);
    //printl("Left", l);
    l = clipPlane(l, topPlane, topNormal);
    //printl("Top",l);
    l = clipPlane(l, bottomPlane, bottomNormal);
    //printl("Bottom",l);
    //printl("Clipped polygon",l);

    return l;
}

std::vector<GLvector> clipFrustum( GLvector corner[4] ) {
    std::vector<GLvector> l;
    for (unsigned i=0; i<4; i++)
        l.push_back( corner[i] );
    return clipFrustum(l);
}

static GLvector findSquare( const std::vector<GLvector>& l, const GLvector& a, unsigned i) {
    GLvector b=a;
    bool done[2]={false,false};
    // find vertical and horisontal intersection
    unsigned j, nexti = (i+1)%l.size();
    for (j=nexti; j!=i; j=(j+1)%l.size()) {
        const GLvector& p1 = l[j];
        const GLvector& p2 = l[(j+1)%l.size()];
        for (unsigned v=0; v<=2; v+=2) {
            unsigned u=2*(!v);
            float f = (a[u]-p1[u])/(p2[u]-p1[u]);
            if (0 <= f && f <= 1 && p2[u]!=p1[u]) {
                float s = p1[v] + (p2[v]-p1[v])*f;
                if (!done[0!=v]) {
                    b[v]=s;
                    done[0!=v] = true;
                }
            }
        }
    }

    if (!done[0] || !done[1])
        return a;

    // validate that the fourth point lies on the inside
    for (j=nexti; j!=i; j=(j+1)%l.size()) {
        const GLvector& p1 = l[j];
        const GLvector& p2 = l[(j+1)%l.size()];
        for (unsigned v=0; v<=2; v+=2) {
            unsigned u=2*(!v);
            float f = (b[u]-p1[u])/(p2[u]-p1[u]);
            if (0 <= f && f <= 1 && p2[u]!=p1[u]) {
                float s = p1[v] + (p2[v]-p1[v])*f;
                float g = (s-a[u])/(b[u]-a[u]);
                if (0 < g && g < 1 && b[u]!=a[u])
                    b[v]=s;
            }
        }
    }

    return b;
}

static std::vector<GLvector> getLargestSquare( const std::vector<GLvector>& l) {
    // find the largest square enclosed by convex polygon 'l'

    // check that the polygon isn't already square
    if (4==l.size()) {
        GLvector a = l[0],b = l[0];
        for (unsigned i=1; i<l.size(); i++) {
            for (unsigned c=0; c<3; c++) {
                if (l[i][c]<a[c]) a[c]=l[i][c];
                if (l[i][c]>b[c]) b[c]=l[i][c];
            }
        }
        bool square = true;
        for (unsigned i=1; i<l.size(); i++) {
            for (unsigned c=0; c<3; c++) {
                if (l[i][c] != a[c] && l[i][c] != b[c])
                    square = false;
            }
        }
        if (square)
            return l;
    }

    // the largest square will tangent the polygon at three points, for which the remaining point lie within the polygon
    GLvector a,b;
    float maxp=-1;
    for (unsigned i=0; i<l.size(); i++) {
        unsigned nexti = (i+1)%l.size();
        for (unsigned m=0; m<2; m++) {
            GLvector sa = (l[i]+l[nexti])*(.5f*m),
                     sb = findSquare(l, sa, i);

            if (fabs((a[0]-b[0])*(a[2]-b[2])) < fabs((sa[0]-sb[0])*(sa[2]-sb[2]))) {
                a = sa, b = sb;
                maxp = i + .5f*m;
            }
        }
    }

    if (maxp<0)
        return std::vector<GLvector>();

    float sz = .25f;
    while (sz>.01) { // accuracy
        unsigned maxi = (unsigned)floor(maxp);
        GLvector sa = (l[maxi]+l[(maxi+1)%l.size()])*(maxp+sz-maxi);
        GLvector sb = findSquare(l, sa, maxi);

        if (fabs((a[0]-b[0])*(a[2]-b[2])) < fabs((sa[0]-sb[0])*(sa[2]-sb[2]))) {
            a = sa, b = sb;
            maxp = maxp+sz;
        }

        maxi = (((unsigned)ceil(maxp))+l.size()-1)%l.size();
        sa = (l[maxi]+l[(maxi+1)%l.size()])*(maxp-sz-maxi);
        sb = findSquare(l, sa, maxi);

        if (fabs((a[0]-b[0])*(a[2]-b[2])) < fabs((sa[0]-sb[0])*(sa[2]-sb[2]))) {
            a = sa, b = sb;
            maxp = maxp+sz;
        }

        sz /= 2;
    }

    std::vector<GLvector> r;
    r.push_back( a );
    r.push_back( GLvector( a[0], 0, b[2] ) );
    r.push_back( GLvector( b[0], 0, b[2] ) );
    r.push_back( GLvector( b[0], 0, a[2] ) );
    return r;
}

/**
  @arg timePixels Estimated longest line of pixels along t-axis within ref measured in pixels
  @arg scalePixels Estimated longest line of pixels along t-axis within ref measured in pixels
  */
bool SpectrogramRenderer::computePixelsPerUnit( Spectrogram::Reference ref, float& timePixels, float& scalePixels )
{
    Spectrogram::Position p[2];
    ref.getArea( p[0], p[1] );

    GLvector corner[4]=
    {
        GLvector( p[0].time, 0, p[0].scale),
        GLvector( p[0].time, 0, p[1].scale),
        GLvector( p[1].time, 0, p[1].scale),
        GLvector( p[1].time, 0, p[0].scale)
    };

    std::vector<GLvector> clippedCorners = clipFrustum(corner);
    // clippedCorners = getLargestSquare(clippedCorners);
    printl("Largest Square",clippedCorners);

    if (0==clippedCorners.size())
        return false;

    // Find projection of object coordinates in corner[i] onto window coordinates
    // Compare length in pixels of each side
    // Check if projection is visible
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    bool above=true, under=true, right=true, left=true;
    timePixels = scalePixels = 0;
    for (unsigned i=0; i<clippedCorners.size(); i++)
    for (unsigned nexti=0; nexti<clippedCorners.size(); nexti++)
    {
        //unsigned nexti=(i+1)%clippedCorners.size();
        if (nexti==i)
            continue;

        GLvector screen[2];

        GLvector c1 = clippedCorners[i];
        GLvector c2 = clippedCorners[nexti];

        GLvector a[] = {
            GLvector( min(c1[0], c2[0]), 0, min(c1[2], c2[2])),
            GLvector( min(c1[0], c2[0]), 0, max(c1[2], c2[2])),
            GLvector( max(c1[0], c2[0]), 0, max(c1[2], c2[2])),
            GLvector( max(c1[0], c2[0]), 0, min(c1[2], c2[2]))
        };

        unsigned alen = sizeof(a)/sizeof(a[0]);
        if ( a[0] == a[1] ) {
            a[1] = a[2];
            alen = 2;
        }
        if ( a[0] == a[3] ) {
            alen = 2;
        }

        for (unsigned b=0; b<(2==alen?1:alen); b++) {
            std::vector<GLvector> l;
            l.push_back( a[b] );
            l.push_back( a[(b+1)%alen] );
            l = clipFrustum( l );
            if (2 > l.size())
                continue;
            if (l[0] == l[1])
                continue;

            screen[0] = gluProject2( l[0] );
            screen[1] = gluProject2( l[1] );

            GLvector screenBorder = screen[0] - screen[1];
            GLvector worldBorder = l[0] - l[1];

            bool labove=true, lunder=true, lright=true, lleft=true;
            for (unsigned j=0; j<2; j++) {
                if (screen[j][0] > view[0]) lleft=false;
                if (screen[j][0] < view[0]+view[2]) lright=false;
                if (screen[j][1] > view[1]) lunder=false;
                if (screen[j][1] < view[1]+view[3]) labove=false;
            }

            above &= labove;
            under &= lunder;
            right &= lright;
            left &= lleft;
            // If everything is
            //if (labove || lunder || lright || lleft)
            //    continue;

            screenBorder[2] = 0;
            float ltimePixels = screenBorder.length() * (p[1].time-p[0].time)/worldBorder[0];
            float lscalePixels = screenBorder.length() * (p[1].scale-p[0].scale)/worldBorder[2];
            timePixels = max(timePixels, (0==worldBorder[0])?0:ltimePixels);
            scalePixels = max(scalePixels, (0==worldBorder[2])?0:lscalePixels);
        }
    }

    // If everything is
    if (above || under || right || left)
        return false;

    return true;
}
