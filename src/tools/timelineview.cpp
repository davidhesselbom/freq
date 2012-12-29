#include "timelineview.h"

// Sonic AWE tools
#include "toolfactory.h"
#include "ui/mainwindow.h"
#include "rendercontroller.h"

// Sonic AWE lib
#include "sawe/application.h"
#include "heightmap/renderer.h"

// gpumisc
#include <computationkernel.h>
#include <GlException.h>
#include <glPushContext.h>
#include <glframebuffer.h>

// boost
#include <boost/assert.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// qt
#include <QMouseEvent>
#include <QDockWidget>
#include <QTimer>
#include <QErrorMessage>
#include <QBoxLayout>

#undef max

//#define TIME_PAINTGL
#define TIME_PAINTGL if(0)

using namespace Signal;

namespace Tools {

TimelineView::
        TimelineView( Sawe::Project* p, RenderView* render_view)
:   QGLWidget( 0, render_view->glwidget, Qt::WindowFlags(0) ),
    tool_selector( 0 ),
    _xscale( 1 ),
    _xoffs( 0 ),
    _barHeight( 0.1f ),
    _length( 0 ),
    _width( 0 ),
    _height( 0 ),
    _project( p ),
    _render_view( render_view ),
    _except_count( 0 ),
    _vertical( true )
{
    EXCEPTION_ASSERT( _render_view );

    if (!context() || !context()->isSharing())
    {
        throw std::invalid_argument("Failed to open a second OpenGL window. Couldn't find a valid rendering context to share.");
    }
}


TimelineView::
        ~TimelineView()
{
    TaskInfo ti("~TimelineView");
    makeCurrent();
    _timeline_fbo.reset();
    _timeline_bar_fbo.reset();
}


Heightmap::Position TimelineView::
        getSpacePos( QPointF pos, bool* success )
{
    pos.setY( tool_selector->parentTool()->geometry().height() - 1 - pos.y() );

    GLvector win_coord( pos.x(), pos.y(), 0.1);

    GLvector world_coord = Heightmap::gluUnProject(
            win_coord,
            modelview_matrix,
            projection_matrix,
            viewport_matrix,
            success);

    return Heightmap::Position( world_coord[0], world_coord[2] );
}


void TimelineView::
        userinput_update()
{
    // Never update only the timeline as renderview is responsible for invoking
    // workOne to update textures as needed. TimelineView::update is connected
    // to RenderView::postPaint.
    _render_view->userinput_update();

    //_project->worker.requested_fps(30);
    //update();
}


void TimelineView::
        paintInGraphicsView()
{
    initializeTimeline();

    EXCEPTION_ASSERT( tool_selector );
    QRect rect = tool_selector->parentTool()->geometry();
    resizeGL( 0, 0, rect.width(), rect.height() );
    paintEventTime = boost::posix_time::microsec_clock::local_time();
    paintGL();
}


void TimelineView::
        layoutChanged( QBoxLayout::Direction direction )
{
    switch (direction)
    {
    case QBoxLayout::TopToBottom:
    case QBoxLayout::BottomToTop:
        _vertical = true;
        tool_selector->parentTool()->setMaximumSize( 524287, 100 );
        break;
    case QBoxLayout::LeftToRight:
    case QBoxLayout::RightToLeft:
        _vertical = false;
        tool_selector->parentTool()->setMaximumSize( 100, 524287 );
        break;
    }
}


void TimelineView::
        initializeGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    {   // Antialiasing
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glDisable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable(GL_BLEND);
    }

    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    initializeTimeline();
}


void TimelineView::
        initializeTimeline()
{
    if (!_timeline_bar_fbo) _timeline_bar_fbo.reset( new GlFrameBuffer );
    if (!_timeline_fbo) _timeline_fbo.reset( new GlFrameBuffer );
}


void TimelineView::
        resizeGL( int width, int height )
{
    resizeGL( 0, 0, width, height );

    userinput_update();
}


void TimelineView::
        resizeGL( int x, int y, int width, int height )
{
    height = height?height:1;

    glViewport( x, y, _width = width, _height = height );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,1,0,1, -10,10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void TimelineView::
        paintGL()
{
    TIME_PAINTGL TaskTimer tt("TimelineView::paintGL");

    boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - paintEventTime;
    if (diff.total_milliseconds() > 50)
    {
        QErrorMessage::qtHandler()->showMessage("Seems like the timeline doesn't work properly on your computer, so it has been removed. Otherwise the program should behave correctly. If you want to help out you can send an error report from the Help menu.");
        emit hideMe();
    }

    _length = std::max( 1.f, _render_view->model->renderer->last_axes_length );
    if (_length < 60*10)
        _barHeight = 0;
    else
        _barHeight = 20.f/(_vertical?_height:_width);

    _except_count = 0;
    try {
        GlException_CHECK_ERROR();
        ComputationCheckError();

        if (!tool_selector)
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        glPushMatrixContext mc(GL_MODELVIEW);

        { // Render
            // Set up camera position
            if (_xscale<1) _xscale = 1;
            if (_xoffs<0) _xoffs = 0;
            if (_xoffs>_length-_length/_xscale) _xoffs = _length-_length/_xscale;

            if (_render_view->model->renderer->left_handed_axes)
            {
                glViewport( 0, _height*_barHeight, _width, _height*(1-_barHeight) );
            }
            else
            {
                glViewport( _width*_barHeight, 0, _width*(1-_barHeight), _height );
            }
            setupCamera( false );

            glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
            glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
            glGetIntegerv(GL_VIEWPORT, viewport_matrix);

            {
                glPushMatrixContext mc(GL_MODELVIEW);

                _render_view->drawCollections( _timeline_fbo.get(), 0 );

                // TODO what should be rendered in the timelineview?
                // Not arbitrary tools but
                // _project->tools().selection_view.drawSelection();
                _render_view->model->renderer->drawFrustum();

                emit painting();
            }
        }

        if (_barHeight>0)
        {
            // Draw little bar for entire signal at the bottom of the timeline
            //glPushMatrixContext mc(GL_MODELVIEW);

            if (_render_view->model->renderer->left_handed_axes)
            {
                glViewport( 0, 0, (GLint)_width, (GLint)_height*_barHeight );
            }
            else
            {
                glViewport( 0, 0, (GLint)_width*_barHeight, (GLint)_height );
            }
            setupCamera( true );

            _render_view->drawCollections( _timeline_bar_fbo.get(), 0 );

            glViewport( 0, 0, (GLint)_width, (GLint)_height );
            setupCamera( true );
            glScalef(1,1,_barHeight);
            //glTranslatef(0,0,-1);

            glColor4f( 0.75, 0.75,0.75, .5);
            glLineWidth(2);
            glBegin(GL_LINES);
                glVertex3f(0,1,1);
                glVertex3f(_length,1,1);
            glEnd();

            float x1 = _xoffs;
            float x4 = _xoffs+_length/_xscale;
            float x2 = x1*.9 + x4*.1;
            float x3 = x1*.1 + x4*.9;
            glBegin( GL_TRIANGLE_STRIP );
                glColor4f(0,0,0,.5);
                glVertex3f(x1,1,0);
                glVertex3f(x1,1,1);
                glColor4f(0,0,0,.25);
                glVertex3f(x2,1,0);
                glVertex3f(x2,1,1);
                glVertex3f(x3,1,0);
                glVertex3f(x3,1,1);
                glColor4f(0,0,0,.5);
                glVertex3f(x4,1,0);
                glVertex3f(x4,1,1);
            glEnd();

            _render_view->model->renderer->drawFrustum();
        }

        GlException_CHECK_ERROR();
        ComputationCheckError();

        _except_count = 0;
#ifdef USE_CUDA
    } catch (const CudaException &x) {
        if (1<++_except_count) throw;

        TaskTimer("TimelineView::paintGL SWALLOWED CUDAEXCEPTION\n%s", x.what()).suppressTiming();;
        Sawe::Application::global_ptr()->clearCaches();
#endif
    } catch (const GlException &x) {
        if (1<++_except_count) throw;

        TaskTimer("TimelineView::paintGL SWALLOWED GLEXCEPTION\n%s", x.what()).suppressTiming();
        Sawe::Application::global_ptr()->clearCaches();
    }
}


void TimelineView::
        paintEvent ( QPaintEvent * event )
{
    paintEventTime = boost::posix_time::microsec_clock::local_time();
    QGLWidget::paintEvent ( event );
}


void TimelineView::
        setupCamera( bool staticTimeLine )
{
    // Make sure that the camera focus point is within the timeline
    {
        float t = _render_view->model->renderer->camera[0];
        float new_t = -1;

        switch(0) // Both 1 and 2 might feel annoying, don't do them :)
        {
        case 1: // Clamp the timeline, prevent moving to much.
                // This might be both annoying and confusing
            if (t < _xoffs) _xoffs = t;
            if (t > _xoffs + _length/_xscale ) _xoffs = t - _length/_xscale;
            break;

        case 2: // Clamp the render view
                // This might be just annoying
            if (t < _xoffs) new_t = _xoffs;
            if (t > _xoffs + _length/_xscale ) new_t = _xoffs + _length/_xscale;

            if (0<=new_t)
            {
                float f = _render_view->model->renderer->camera[2];
                _render_view->setPosition( Heightmap::Position( new_t, f) );
            }
            break;
        }
    }

    glLoadIdentity();

    glRotatef( 90, 1, 0, 0 );
    glRotatef( 180, 0, 1, 0 );

    if (!_render_view->model->renderer->left_handed_axes)
    {
        glTranslatef(-0.5f,0,0);
        glScalef(-1,1,1);
        glTranslatef(-0.5f,0,0);
        glRotatef(90,0,1,0);
    }

    glScalef(-1/_length, 1, 1);

    if (!staticTimeLine) {
        glScalef(_xscale, 1, 1);
        glTranslatef(-_xoffs, 0, 0);
    }
}


} // namespace Tools
