#ifndef GLSTATE_H
#define GLSTATE_H

#include "gl.h"

/**
 * @brief The GlState class should track OpenGL states and reduce the number of
 * redundant state changes being transferred to the OpenGL driver.
 *
 * The state changes are not transferred until a draw call or sync().
 */
namespace GlState
{
    void glEnable (GLenum cap, bool now=false);
    void glDisable (GLenum cap, bool now=false);

    void glBindBuffer(GLenum target, GLuint buffer);
    void glDeleteBuffers(GLsizei n, const GLuint *buffers);
    void glUseProgram(GLuint program);
    void notifyDeletedProgram(GLuint program);

    void glEnableVertexAttribArray (GLuint index);
    void glDisableVertexAttribArray (GLuint index);

    void glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
    void glDrawArrays (GLenum mode, GLint first, GLsizei count);

    /**
     * @brief sync isses necessary opengl state changes.
     */
    void sync ();

    /**
     * @brief setGlIsEnabled is used to synchronize the internal state of GlState to that of OpenGL
     * @param cap a glEnable capability
     * @param v use glIsEnabled (cap) if unsure.
     */
    void setGlIsEnabled (GLenum cap, bool v);

    /**
     * @brief assume_default_gl_states assumes that all states tracked by
     * GlState have their default values. Any caps enabled by glstate will be
     * reenabled on the next sync.
     */
    void assume_default_gl_states ();
    void set_default_gl_states_and_sync ();

    /**
     * @brief assume_default_qt_quick_states sets the states that Qt Quick sets.
     * It is close to default_gl_states but not exactly the same.
     *
     * Note that QGLWidget uses the default gl states.
     */
    void assume_default_qt_quick_states ();
}

#endif // GLSTATE_H
