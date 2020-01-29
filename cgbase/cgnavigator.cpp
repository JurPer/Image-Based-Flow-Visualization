/* Copyright (C) 2013, 2014, 2015, 2018 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de> */

#include <cmath>

#include <QtMath>

#include "cgnavigator.hpp"


namespace Cg {

Navigator::Navigator()
{
    initialize(QVector3D(0.0f, 0.0f, 0.0f), 3.0f);
    setViewport(0, 0, -1, -1);
}

bool Navigator::checkPos(const QPoint& mousePos)
{
    return (mousePos.x() >= _viewport[0] && mousePos.x() < _viewport[0] + _viewport[2]
            && mousePos.y() >= _viewport[1] && mousePos.y() < _viewport[0] + _viewport[3]);
}

QVector3D Navigator::ballMap(const QPoint& mousePos)
{
    int x = mousePos.x() - _viewport[0];
    int y = mousePos.y() - _viewport[1];
    int w = std::max(2, _viewport[2]);
    int h = std::max(2, _viewport[3]);

    // bring v=(x,y) to [-1..1]^2
    float vx = (x / (w - 1.0f) - 0.5f) * 2.0f;
    float vy = ((h - 1 - y) / (h - 1.0f) - 0.5f) * 2.0f;

    float ll = vx * vx +  vy * vy;
    if (ll > 1.0f) {
        // outside ArcBall
        ll = std::sqrt(ll);
        return QVector3D(vx / ll, vy / ll, 0.0f);
    } else {
        // inside ArcBall
        return QVector3D(vx, vy, std::sqrt(1.0f - ll));
    }
}

void Navigator::initialize(const QVector3D& center, float radius)
{
    _sceneCenter = center;
    _sceneRadius = radius;
    reset();
}

void Navigator::setViewport(int x, int y, int w, int h)
{
    _viewport[0] = x;
    _viewport[1] = y;
    _viewport[2] = w;
    _viewport[3] = h;
}

QVector3D Navigator::viewPos() const
{
    return _pos + _sceneCenter;
}

QQuaternion Navigator::viewRot() const
{
    return _rot;
}

QMatrix4x4 Navigator::viewMatrix() const
{
    QMatrix4x4 M((-viewRot()).toRotationMatrix());
    M.translate(-viewPos());
    return M;
}

void Navigator::reset()
{
    _pos = QVector3D(0.0f, 0.0f, 2.5f * _sceneRadius);
    _rot = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, -1.0f, 0.0f);
    _lastDist = _pos.length() - _sceneRadius;
}

void Navigator::startRot(const QPoint& mousePos)
{
    _lastMousePos = mousePos;
    _lastBallPos = ballMap(mousePos);
}

void Navigator::rot(const QPoint& mousePos)
{
    if (checkPos(mousePos)) {
        QVector3D ballPos = ballMap(mousePos);
        QVector3D normal = QVector3D::crossProduct(_lastBallPos, ballPos);
        if (normal.length() > 0.001f) {
            QVector3D axis = (_rot.inverted() * normal).normalized();
            float angle = std::acos(QVector3D::dotProduct(_lastBallPos, ballPos));
            angle *= (_pos.length() - _sceneRadius) / _sceneRadius;
            QQuaternion rot = QQuaternion::fromAxisAndAngle(axis, qRadiansToDegrees(angle));
            _pos = rot.inverted() * _pos;
            _rot = _rot * rot;

        }
        _lastBallPos = ballPos;
    }
}

void Navigator::startShift(const QPoint& mousePos)
{
    _lastMousePos = mousePos;
    _lastDist = _pos.length() - _sceneRadius;
}

void Navigator::shift(const QPoint& mousePos)
{
    QVector3D up = _rot.inverted() * QVector3D(0.0f, 1.0f, 0.0f);
    QVector3D view = _rot.inverted() * QVector3D(0.0f, 0.0f, -1.0f);
    QVector3D left = QVector3D::crossProduct(up, view);
    float shiftPerPixelX = (0.1f + _lastDist / _viewport[2]) / (20.0f / _sceneRadius);
    float shiftPerPixelY = (0.1f + _lastDist / _viewport[3]) / (20.0f / _sceneRadius);
    float offsetX = (mousePos.x() - _lastMousePos.x()) * shiftPerPixelX;
    float offsetY = (mousePos.y() - _lastMousePos.y()) * shiftPerPixelY;
    _pos += offsetX * left;
    _pos += offsetY * up;
    _lastMousePos = mousePos;
    _lastDist = _pos.length() - _sceneRadius;
}

void Navigator::startZoom(const QPoint& mousePos)
{
    _lastMousePos = mousePos;
    _lastDist = _pos.length() - _sceneRadius;
}

void Navigator::zoom(const QPoint& mousePos)
{
    float distchangePerPixel = (0.1f + _lastDist / _viewport[3]) / (20.0f / _sceneRadius);
    float offset = (mousePos.y() - _lastMousePos.y()) * distchangePerPixel;
    _pos += offset * (_rot.inverted() * QVector3D(0.0f, 0.0f, -1.0f));
    _lastMousePos = mousePos;
    _lastDist = _pos.length() - _sceneRadius;
}

void Navigator::zoom(float wheelRot)
{
    float distchangePerDegree = (0.1f + _lastDist / _viewport[3]) / (5.0f / _sceneRadius);
    float offset = -wheelRot * distchangePerDegree;
    _pos += offset * (_rot.inverted() * QVector3D(0.0f, 0.0f, -1.0f));
    _lastDist = _pos.length() - _sceneRadius;
}

}
