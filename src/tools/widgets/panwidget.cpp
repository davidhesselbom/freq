#include "panwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QApplication>

#include "hudglwidget.h"
#include "sawe/project.h"
#include "tools/renderview.h"
#include "tools/commands/movecameracommand.h"

namespace Tools {
namespace Widgets {


PanWidget::PanWidget(RenderView *view) :
    view_(view)
{
    setMinimumSize(70,70);
    setCursor(Qt::OpenHandCursor);
    setToolTip("Click and drag to pan [ctrl]");
}


void PanWidget::
        leaveEvent ( QEvent * )
{
    QApplication::restoreOverrideCursor();
}


void PanWidget::
        mouseMoveEvent ( QMouseEvent * event )
{
    bool success1, success2;

    Heightmap::Position last = view_->getPlanePos( dragSource_, &success1);
    Heightmap::Position current = view_->getPlanePos( event->posF(), &success2);

    if (success1 && success2)
    {
        float dt = current.time - last.time;
        float ds = current.scale - last.scale;

        Tools::Commands::pCommand cmd( new Tools::Commands::MoveCameraCommand(view_->model, -dt, -ds ));
        view_->model->project()->commandInvoker()->invokeCommand( cmd );
    }

    dragSource_ = event->pos();
}


void PanWidget::
        mousePressEvent ( QMouseEvent * event )
{
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    dragSource_ = event->pos();
}


void PanWidget::
        mouseReleaseEvent ( QMouseEvent * event )
{
    leaveEvent(event);
}


void PanWidget::
        paintEvent ( QPaintEvent * event )
{
    QPainter painter (this);
    painter.beginNativePainting ();
    painter.setRenderHints (QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    painter.fillPath (path_, QColor(220,220,220,200));
    painter.strokePath (path_, QPen(
                            QColor(100,100,100,200),
                            hasFocus () ? 1.6 : .8,
                            Qt::SolidLine,
                            Qt::RoundCap,
                            Qt::RoundJoin));

    QWidget::paintEvent(event);
}



void PanWidget::
        resizeEvent ( QResizeEvent *)
{
    recreatePolygon();
}


void PanWidget::
        recreatePolygon ()
{
    QPoint o = rect().center();
    QPoint x = (rect().topRight() - rect().topLeft())/4;
    QPoint y = (rect().bottomLeft() - rect().topLeft())/4;

    QPolygon poly;
    float R = 1.5;
    float r = 0.25;
    poly.push_back( o - R*y );
    poly.push_back( o + r*x - r*y);
    poly.push_back( o + R*x );
    poly.push_back( o + r*x + r*y);
    poly.push_back( o + R*y );
    poly.push_back( o - r*x + r*y);
    poly.push_back( o - R*x );
    poly.push_back( o - r*x - r*y);

    path_ = QPainterPath();
    path_.addPolygon(poly);

    setMask(HudGlWidget::growRegion(poly));
    update();
}


} // namespace Widgets
} // namespace Tools
