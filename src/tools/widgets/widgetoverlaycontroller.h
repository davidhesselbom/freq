#ifndef TOOLS_WIDGETS_WIDGETOVERLAYCONTROLLER_H
#define TOOLS_WIDGETS_WIDGETOVERLAYCONTROLLER_H

#include "overlaywidget.h"

#include <QList>
#include <QMouseEvent>

namespace Tools {
namespace Widgets {

class WidgetOverlayController: public OverlayWidget
{
public:
    WidgetOverlayController(RenderView* view);
    ~WidgetOverlayController();

    void enterEvent ( QEvent * );
    void leaveEvent ( QEvent * );
    void keyPressEvent ( QKeyEvent * );
    void keyReleaseEvent ( QKeyEvent * );
    void mouseMoveEvent ( QMouseEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void mouseReleaseEvent ( QMouseEvent * event );

protected:
    void updatePosition();

private:
    void setupLayout();
    void setupLayoutCenter();
    void setupLayoutRightAndBottom();

    bool updateFocusWidget(QKeyEvent *e);
    void sendMouseProxyEvent( QMouseEvent * event );

    QWidget *pan_, *rescale_, *rotate_, *proxy_mousepress_;
    RenderView* view_;
    QMouseEvent child_event_;
    QPoint lastMousePos_;
};

} // namespace Widgets
} // namespace Tools

#endif // TOOLS_WIDGETS_WIDGETOVERLAYCONTROLLER_H
