Framework for Computer Graphics tutorials
=========================================

This framework requires only Qt, but can optionally make use of QVR to support
Virtual Reality hardware such as Oculus Rift, HTC Vive, render clusters etc.

Its base is the class Cg::OpenGLWidget. This class is intended to be as
similar to QOpenGLWidget as possible, but there are a few differences:

1. Use Cg::init(argc, argv, widget) instead of widget->show() to start your
   program.
2. Use initializeGL() exactly as you would with QOpenGLWidget.
3. Instead of resizeGL(width, height) and paintGL(), implement
   paintGL(P, V, width, height) and use the provided projection matrix P and
   view matrix V if possible.
4. Use quit() instead of close() to close and exit your application.
5. Optionally use animate() to update scene state for animations etc.

You can still implement keyboard and mouse event handling exactly as you would
with QOpenGLWidget.

When following these rules, you get these advantages:
- When using only Qt: you automatically get mouse-based navigation, SPACE
  resets navigation, ESC or q exits your application, and f toggles
  fullscreen mode.
- When using QVR: you automatically get support for a wide range of Virtual
  Reality systems, including head tracking and controller-based navigation.

Additionally, this framework provides a few useful helper functions to
generate or load simple geometries. See cggeometries.hpp and cgtools.hpp.
