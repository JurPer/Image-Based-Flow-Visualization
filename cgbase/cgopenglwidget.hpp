/* Copyright (C) 2018 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de> */

#ifndef CGOPENGLWIDGET_HPP
#define CGOPENGLWIDGET_HPP

#include <QOpenGLExtraFunctions>

#ifdef CG_HAVE_QVR
# include <qvr/app.hpp>
#else
# include <QOpenGLWidget>
# include <QTimer>
#endif

#include "cgnavigator.hpp"


namespace Cg {

/* This class provides a widget that is similar to QOpenGLWidget, but with
 * a slightly different interface that allows mouse-based navigation (if used
 * as a standalone widget) or VR support including head tracking and
 * controller-based navigation (if used with QVR).
 *
 * See README.txt for details on how to use this class. */
class OpenGLWidget :
#ifdef CG_HAVE_QVR
    public QVRApp,
#else
    public QOpenGLWidget,
#endif
    protected QOpenGLExtraFunctions
{
#ifndef CG_HAVE_QVR
Q_OBJECT
#endif

private:
#ifdef CG_HAVE_QVR
    bool _wantExit;
#else
    QTimer _updateTimer;
#endif
    Navigator _navigator;

public:
    OpenGLWidget();
    virtual ~OpenGLWidget();

    // Get the navigator
    Navigator* navigator() { return &_navigator; }

    // Quit the application
    void quit();

    // Initialize GL objects
    virtual void initializeGL() { initializeOpenGLFunctions(); }

    // Optionally set near and far plane values
    virtual void getNearFar(float* n, float* f) { *n = 0.05f; *f = 100.0f; }

    // Render a view into the current framebuffer with the given width and height.
    // Use projection matrix P and view matrix V if possible to get automatic navigation
    // and VR support.
    virtual void paintGL(const QMatrix4x4& P, const QMatrix4x4& V, int width, int height);

#ifndef CG_HAVE_QVR
public slots:
#endif
    // Update scene parameters, for animation. This will be called whenever the program
    // has nothing else to do. For static scenes, just leave this empty.
    virtual void animate() {}

public:
    // Handle keyboard and mouse events in the same way as QOpenGLWidget
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

    // The rest is internal glue code for QVR or QOpenGLWidget; you can ignore this.
#ifdef CG_HAVE_QVR
    void render(QVRWindow*, const QVRRenderContext& context, const unsigned int* textures) override;
    void update(const QList<QVRObserver*>&) override { animate(); }
    bool wantExit() override { return _wantExit; }
    bool initProcess(QVRProcess*) { initializeGL(); return true; }
    void keyPressEvent(const QVRRenderContext&, QKeyEvent* event) override { keyPressEvent(event); }
    void keyReleaseEvent(const QVRRenderContext&, QKeyEvent* event) override { keyReleaseEvent(event); }
    void mouseMoveEvent(const QVRRenderContext&, QMouseEvent* event) override { mouseMoveEvent(event); }
    void mousePressEvent(const QVRRenderContext&, QMouseEvent* event) override { mousePressEvent(event); }
    void mouseReleaseEvent(const QVRRenderContext&, QMouseEvent* event) override { mouseReleaseEvent(event); }
    void mouseDoubleClickEvent(const QVRRenderContext&, QMouseEvent* event) override { mouseDoubleClickEvent(event); }
    void wheelEvent(const QVRRenderContext&, QWheelEvent* event) override { wheelEvent(event); }
#else
    void paintGL() override;
#endif
};

}

#endif
