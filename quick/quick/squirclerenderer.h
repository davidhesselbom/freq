#ifndef SQUIRCLERENDERER_H
#define SQUIRCLERENDERER_H

#include <QObject>
#include <QOpenGLShaderProgram>
#include "tools/rendermodel.h"
#include "tools/renderview.h"

/**
 * @brief The SquircleRenderer is created when there is an OpenGL context and performs rendering.
 * The SquircleRenderer is destroyed before the OpenGL context is destroyed.
 */
class SquircleRenderer : public QObject {
    Q_OBJECT
public:
    SquircleRenderer(Tools::RenderModel* render_model);
    ~SquircleRenderer();

    void setT(qreal t) { m_t = t; }
    void setViewportSize(const QSize &size);

signals:
    void redrawSignal();

public slots:
    void paint2();
    void paint();

private:
    Tools::RenderView render_view;

    QSize m_viewportSize;
    qreal m_t;
    QOpenGLShaderProgram *m_program;
};

#endif // SQUIRCLERENDERER_H
