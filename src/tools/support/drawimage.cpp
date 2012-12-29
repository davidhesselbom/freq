#include "drawimage.h"

#include <QImage>

#include "TaskTimer.h"
#include "GlException.h"
#include "gl.h"
#include "glPushContext.h"

namespace Tools {
namespace Support {

DrawImage::
DrawImage(QString imagePath, QPointF pos )
    : pos_(pos)
{
    img_ = loadImage(imagePath.toStdString());

    setupVbo();
}


QPointF DrawImage::
pos()
{
    return pos_;
}


void DrawImage::
move(QPointF pos)
{
    pos_ = pos;
}


QSize DrawImage::
size() const
{
    return QSize(img_->getWidth(), img_->getHeight());
}


boost::shared_ptr<GlTexture> DrawImage::
loadImage(std::string imagePath)
{
    TaskTimer tt("%s %s", __FUNCTION__, imagePath.c_str());

    GlException_CHECK_ERROR();

    QImage data(imagePath.c_str());
    tt.info("format = %d, size = (%d,%d)", data.format(), data.width(), data.height());
    QImage::Format expected_ftm = QImage::Format_ARGB32;
    if (data.format() != expected_ftm)
    {
        tt.info("changing format to %d", expected_ftm);
        data = data.convertToFormat(expected_ftm);
    }


    std::vector<unsigned char> swizzled(data.byteCount());

    QRgb*ptr = (QRgb*)data.bits();
    for (int y=0; y<data.height(); ++y) for (int x=0; x<data.width(); ++x)
    {
        unsigned o = y*data.width() + x;
        const QRgb& p = ptr[o];
        swizzled[4*o + 0] = qRed(p);
        swizzled[4*o + 1] = qGreen(p);
        swizzled[4*o + 2] = qBlue(p);
        swizzled[4*o + 3] = qAlpha(p);
    }

    return boost::shared_ptr<GlTexture>(new GlTexture(data.width(), data.height(), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &swizzled[0]));
}


void DrawImage::
setupVbo()
{
    if (!postexvbo_)
        postexvbo_.reset(new Vbo( 2*4*2*sizeof(float), GL_ARRAY_BUFFER, GL_STATIC_DRAW ));

    glBindBuffer(postexvbo_->vbo_type(), *postexvbo_);
    float *p = (float *) glMapBuffer(postexvbo_->vbo_type(), GL_WRITE_ONLY);

    for (int y=0; y<2; ++y) for (int x=0; x<2; ++x)
    {
        *p++ = x;
        *p++ = 1-y;

        *p++ = x;
        *p++ = y;
    }

    glUnmapBuffer(postexvbo_->vbo_type());
    glBindBuffer(postexvbo_->vbo_type(), 0);
}


void DrawImage::
        drawImage(int viewport_width, int viewport_height) const
{
    glPushAttribContext push_attribs;

    glPushMatrixContext push_proj( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, viewport_width, 0, viewport_height, -1, 1);

    glDisable(GL_DEPTH_TEST);

    glPushMatrixContext push_model( GL_MODELVIEW );

    glLoadIdentity();
    glTranslatef(pos_.x(), pos_.y(), 0.f);
    glScalef(size().width(), size().height(), 1.f);

    directDraw();
}


void DrawImage::
        directDraw() const
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_COLOR_MATERIAL);
    glColor4f(1,1,1,1);

    glBindBuffer(GL_ARRAY_BUFFER, *postexvbo_);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(float)*4, 0);
    glVertexPointer(2, GL_FLOAT, sizeof(float)*4, (float*)0 + 2);

    {
        GlTexture::ScopeBinding bindTexture = img_->getScopeBinding();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_DEPTH_TEST);

    GlException_CHECK_ERROR();
}


} // namespace Support
} // namespace Tools
