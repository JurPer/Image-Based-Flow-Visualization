/* Copyright (C) 2018 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de> */

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "cgopenglwidget.hpp"


namespace Cg {

OpenGLWidget::OpenGLWidget()
#ifdef CG_HAVE_QVR
    : _wantExit(false)
#endif
{
    // Set requested OpenGL version. You can override this in your own constructor.
    QSurfaceFormat format;
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
        format.setVersion(3, 2);
    } else {
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setVersion(4, 5);
    }
    QSurfaceFormat::setDefaultFormat(format);
#ifndef CG_HAVE_QVR
    resize(800, 600);
    connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(animate()));
    connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    _updateTimer.start();
#endif
}

OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::quit()
{
#ifdef CG_HAVE_QVR
    _wantExit = true;
#else
    close();
#endif
}

void OpenGLWidget::paintGL(const QMatrix4x4&, const QMatrix4x4&, int w, int h)
{
#ifdef CG_HAVE_QVR
    Q_UNUSED(w);
    Q_UNUSED(h);
#else
    navigator()->setViewport(0, 0, w, h);
#endif
}

#ifdef CG_HAVE_QVR
void OpenGLWidget::render(QVRWindow*, const QVRRenderContext& context, const unsigned int* textures)
{
    static GLuint fbo = 0;
    static GLuint depthTex = 0;

    // initialize FBO once
    if (fbo == 0)
        glGenFramebuffers(1, &fbo);

    // initialize depth tex once
    if (depthTex == 0) {
        glGenTextures(1, &depthTex);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Loop over the required views
    for (int view = 0; view < context.viewCount(); view++) {
        // Get view dimensions
        int width = context.textureSize(view).width();
        int height = context.textureSize(view).height();
        // Set up framebuffer object to render into
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height,
                0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[view], 0);
        // Call the render function
        QMatrix4x4 P = context.frustum(view).toMatrix4x4();
        QMatrix4x4 V = context.viewMatrix(view);
        paintGL(P, V, width, height);
        // Invalidate depth attachment (to help OpenGL ES performance)
        const GLenum fboInvalidations[] = { GL_DEPTH_ATTACHMENT };
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, fboInvalidations);
    }
}
void OpenGLWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        quit();
}
void OpenGLWidget::keyReleaseEvent(QKeyEvent*) {}
void OpenGLWidget::mouseMoveEvent(QMouseEvent*) {}
void OpenGLWidget::mousePressEvent(QMouseEvent*) {}
void OpenGLWidget::mouseReleaseEvent(QMouseEvent*) {}
void OpenGLWidget::mouseDoubleClickEvent(QMouseEvent*) {}
void OpenGLWidget::wheelEvent(QWheelEvent*) {}
#else
void OpenGLWidget::paintGL()
{
    int w = width() * devicePixelRatio();
    int h = height() * devicePixelRatio();
    float n, f;
    getNearFar(&n, &f);
    QMatrix4x4 P;
    P.perspective(50.0f, static_cast<float>(w) / h, n, f);
    QMatrix4x4 V = navigator()->viewMatrix();
    paintGL(P, V, w, h);
}
void OpenGLWidget::keyPressEvent(QKeyEvent* event)
{
    static bool fullscreen = false;
    switch (event->key())
    {
    case Qt::Key_Space:
        navigator()->reset();
        break;
    case Qt::Key_F:
        if (fullscreen) {
            showNormal();
            fullscreen = false;
        } else {
            showFullScreen();
            fullscreen = true;
        }
        break;
    case Qt::Key_Escape:
        if (fullscreen) {
            showNormal();
            fullscreen = false;
        } else {
            quit();
        }
        break;
    case Qt::Key_Q:
        quit();
        break;
    }
}
void OpenGLWidget::keyReleaseEvent(QKeyEvent*)
{
}
void OpenGLWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
        navigator()->startRot(event->pos());
    else if (event->buttons() & Qt::MidButton)
        navigator()->startShift(event->pos());
    else if (event->buttons() & Qt::RightButton)
        navigator()->startZoom(event->pos());
}
void OpenGLWidget::mouseReleaseEvent(QMouseEvent*)
{
}
void OpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
        navigator()->rot(event->pos());
    else if (event->buttons() & Qt::MidButton)
        navigator()->shift(event->pos());
    else if (event->buttons() & Qt::RightButton)
        navigator()->zoom(event->pos());
}
void OpenGLWidget::wheelEvent(QWheelEvent* event)
{
    navigator()->zoom(event->angleDelta().y() / 8.0f);
}
void OpenGLWidget::mouseDoubleClickEvent(QMouseEvent*)
{
}
#endif

}
