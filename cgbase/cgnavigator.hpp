/* Copyright (C) 2013, 2014, 2015, 2018 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de> */

#ifndef CGNAVIGATOR_HPP
#define CGNAVIGATOR_HPP

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>


namespace Cg {

/* This class provides very simple mouse-based navigation for the Cg tutorials. */
class Navigator
{
private:
    QVector3D _sceneCenter;
    float _sceneRadius;

    int _viewport[4];

    QVector3D _pos;
    QQuaternion _rot;

    QPoint _lastMousePos;
    QVector3D _lastBallPos;
    float _lastDist;

    bool checkPos(const QPoint& mousePos);
    QVector3D ballMap(const QPoint& mousePos);

public:
    Navigator();

    // Set your scene. You must specify center and radius.
    void initialize(const QVector3D& sceneCenter, float sceneRadius);

    // The navigator must always know the current viewport.
    void setViewport(int x, int y, int w, int h);

    // Return the viewer position.
    QVector3D viewPos() const;

    // Return the viewer orientation.
    QQuaternion viewRot() const;

    // Return the view matrix.
    QMatrix4x4 viewMatrix() const;

    // Reset to the default view.
    void reset();

    // Rotate based on mouse coordinates.
    void startRot(const QPoint& mousePos);
    void rot(const QPoint& mousePos);

    // Shift based on mouse coordinates.
    void startShift(const QPoint& mousePos);
    void shift(const QPoint& mousePos);

    // Zoom based on mouse coordinates.
    void startZoom(const QPoint& mousePos);
    void zoom(const QPoint& mousePos);

    // Zoom based on mouse wheel.
    void zoom(float wheel_rot /* in radians, pos or neg */);
};

}

#endif
