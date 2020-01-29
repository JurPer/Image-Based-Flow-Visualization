/* Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de> */

#define _USE_MATH_DEFINES
#include <cmath>

#include "cggeometries.hpp"

static const float pi = M_PI;
static const float half_pi = M_PI_2;
static const float two_pi = 2 * M_PI;

namespace Cg {

void quad(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices)
{
    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    for (int i = 0; i <= slices; i++) {
        float ty = i / (slices / 2.0f);
        for (int j = 0; j <= slices; j++) {
            float tx = j / (slices / 2.0f);
            float x = -1.0f + tx;
            float y = -1.0f + ty;
            float z = 0.0f;
            positions.append(x);
            positions.append(y);
            positions.append(z);
            normals.append(0.0f);
            normals.append(0.0f);
            normals.append(1.0f);
            texCoords.append(tx / 2.0f);
            texCoords.append(ty / 2.0f);
            if (i < slices && j < slices) {
                indices.append((i + 0) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
            }
        }
    }
}

void cube(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices)
{
    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    int verticesPerSide = (slices + 1) * (slices + 1);
    for (int side = 0; side < 6; side++) {
        float nx, ny, nz;
        switch (side) {
        case 0: // front
            nx = 0.0f;
            ny = 0.0f;
            nz = +1.0f;
            break;
        case 1: // back
            nx = 0.0f;
            ny = 0.0f;
            nz = -1.0f;
            break;
        case 2: // left
            nx = -1.0f;
            ny = 0.0f;
            nz = 0.0f;
            break;
        case 3: // right
            nx = +1.0f;
            ny = 0.0f;
            nz = 0.0f;
            break;
        case 4: // top
            nx = 0.0f;
            ny = +1.0f;
            nz = 0.0f;
            break;
        case 5: // bottom
            nx = 0.0f;
            ny = -1.0f;
            nz = 0.0f;
            break;
        }
        for (int i = 0; i <= slices; i++) {
            float ty = i / (slices / 2.0f);
            for (int j = 0; j <= slices; j++) {
                float tx = j / (slices / 2.0f);
                float x, y, z;
                switch (side) {
                case 0: // front
                    x = -1.0f + tx;
                    y = -1.0f + ty;
                    z = 1.0f;
                    break;
                case 1: // back
                    x = 1.0f - tx;
                    y = -1.0f + ty;
                    z = -1.0f;
                    break;
                case 2: // left
                    x = -1.0f;
                    y = -1.0f + ty;
                    z = -1.0f + tx;
                    break;
                case 3: // right
                    x = 1.0f;
                    y = -1.0f + ty;
                    z = 1.0f - tx;
                    break;
                case 4: // top
                    x = -1.0f + ty;
                    y = 1.0f;
                    z = -1.0f + tx;
                    break;
                case 5: // bottom
                    x = 1.0f - ty;
                    y = -1.0f;
                    z = -1.0f + tx;
                    break;
                }
                positions.append(x);
                positions.append(y);
                positions.append(z);
                normals.append(nx);
                normals.append(ny);
                normals.append(nz);
                texCoords.append(tx / 2.0f);
                texCoords.append(ty / 2.0f);
                if (i < slices && j < slices) {
                    indices.append(side * verticesPerSide + (i + 0) * (slices + 1) + (j + 0));
                    indices.append(side * verticesPerSide + (i + 0) * (slices + 1) + (j + 1));
                    indices.append(side * verticesPerSide + (i + 1) * (slices + 1) + (j + 0));
                    indices.append(side * verticesPerSide + (i + 0) * (slices + 1) + (j + 1));
                    indices.append(side * verticesPerSide + (i + 1) * (slices + 1) + (j + 1));
                    indices.append(side * verticesPerSide + (i + 1) * (slices + 1) + (j + 0));
                }
            }
        }
    }
}

void disk(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        float innerRadius,
        int slices)
{
    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    const int loops = 1;

    Q_ASSERT(innerRadius >= 0.0f);
    Q_ASSERT(innerRadius <= 1.0f);
    Q_ASSERT(slices >= 4);
    Q_ASSERT(loops >= 1);

    for (int i = 0; i <= loops; i++) {
        float ty = static_cast<float>(i) / loops;
        float r = innerRadius + ty * (1.0f - innerRadius);
        for (int j = 0; j <= slices; j++) {
            float tx = static_cast<float>(j) / slices;
            float alpha = tx * two_pi + half_pi;
            positions.append(r * std::cos(alpha));
            positions.append(r * std::sin(alpha));
            positions.append(0.0f);
            normals.append(0.0f);
            normals.append(0.0f);
            normals.append(1.0f);
            texCoords.append(1.0f - tx);
            texCoords.append(ty);
            if (i < loops && j < slices) {
                indices.append((i + 0) * (slices + 1) + (j + 0));
                indices.append((i + 1) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
                indices.append((i + 1) * (slices + 1) + (j + 1));
            }
        }
    }
}

void sphere(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices, int stacks)
{
    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    Q_ASSERT(slices >= 4);
    Q_ASSERT(stacks >= 2);

    for (int i = 0; i <= stacks; i++) {
        float ty = static_cast<float>(i) / stacks;
        float lat = ty * pi;
        for (int j = 0; j <= slices; j++) {
            float tx = static_cast<float>(j) / slices;
            float lon = tx * two_pi - half_pi;
            float sinlat = std::sin(lat);
            float coslat = std::cos(lat);
            float sinlon = std::sin(lon);
            float coslon = std::cos(lon);
            float x = sinlat * coslon;
            float y = coslat;
            float z = sinlat * sinlon;
            positions.append(x);
            positions.append(y);
            positions.append(z);
            normals.append(x);
            normals.append(y);
            normals.append(z);
            texCoords.append(1.0f - tx);
            texCoords.append(1.0f - ty);
            if (i < stacks && j < slices) {
                indices.append((i + 0) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
            }
        }
    }
}

void cylinder(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices, int stacks)
{
    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    for (int i = 0; i <= stacks; i++) {
        float ty = static_cast<float>(i) / stacks;
        for (int j = 0; j <= slices; j++) {
            float tx = static_cast<float>(j) / slices;
            float alpha = tx * two_pi - half_pi;
            float x = std::cos(alpha);
            float y = -(ty * 2.0f - 1.0f);
            float z = std::sin(alpha);
            positions.append(x);
            positions.append(y);
            positions.append(z);
            normals.append(x);
            normals.append(0);
            normals.append(z);
            texCoords.append(1.0f - tx);
            texCoords.append(1.0f - ty);
            if (i < stacks && j < slices) {
                indices.append((i + 0) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
            }
        }
    }
}

void cone(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices, int stacks)
{
    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    Q_ASSERT(slices >= 4);
    Q_ASSERT(stacks >= 2);

    for (int i = 0; i <= stacks; i++) {
        float ty = static_cast<float>(i) / stacks;
        for (int j = 0; j <= slices; j++) {
            float tx = static_cast<float>(j) / slices;
            float alpha = tx * two_pi - half_pi;
            float x = ty * std::cos(alpha);
            float y = -(ty * 2.0f - 1.0f);
            float z = ty * std::sin(alpha);
            positions.append(x);
            positions.append(y);
            positions.append(z);
            float nx = x;
            float ny = 0.5f;
            float nz = z;
            float nl = std::sqrt(nx * nx + ny * ny + nz * nz);
            normals.append(nx / nl);
            normals.append(ny / nl);
            normals.append(nz / nl);
            texCoords.append(1.0f - tx);
            texCoords.append(1.0f - ty);
            if (i < stacks && j < slices) {
                indices.append((i + 0) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
                indices.append((i + 0) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 1));
                indices.append((i + 1) * (slices + 1) + (j + 0));
            }
        }
    }
}

void torus(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        float innerRadius,
        int sides, int rings)
{
    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    Q_ASSERT(innerRadius >= 0.0f);
    Q_ASSERT(innerRadius < 1.0f);
    Q_ASSERT(sides >= 4);
    Q_ASSERT(rings >= 4);

    float ringradius = (1.0f - innerRadius) / 2.0f;
    float ringcenter = innerRadius + ringradius;

    for (int i = 0; i <= sides; i++) {
        float ty = static_cast<float>(i) / sides;
        float alpha = ty * two_pi - half_pi;
        float c = std::cos(alpha);
        float s = std::sin(alpha);
        for (int j = 0; j <= rings; j++) {
            float tx = static_cast<float>(j) / rings;
            float beta = tx * two_pi - pi;

            float x = ringcenter + ringradius * std::cos(beta);
            float y = 0.0f;
            float z = ringradius * std::sin(beta);
            float rx = c * x + s * y;
            float ry = c * y - s * x;
            float rz = z;
            positions.append(rx);
            positions.append(ry);
            positions.append(rz);

            float rcx = c * ringcenter;
            float rcy = - s * ringcenter;
            float rcz = 0.0f;
            float nx = rx - rcx;
            float ny = ry - rcy;
            float nz = rz - rcz;
            float nl = std::sqrt(nx * nx + ny * ny + nz * nz);
            normals.append(nx / nl);
            normals.append(ny / nl);
            normals.append(nz / nl);

            texCoords.append(1.0f - tx);
            texCoords.append(1.0f - ty);
            if (i < sides && j < rings) {
                indices.append((i + 0) * (rings + 1) + (j + 0));
                indices.append((i + 0) * (rings + 1) + (j + 1));
                indices.append((i + 1) * (rings + 1) + (j + 0));
                indices.append((i + 0) * (rings + 1) + (j + 1));
                indices.append((i + 1) * (rings + 1) + (j + 1));
                indices.append((i + 1) * (rings + 1) + (j + 0));
            }
        }
    }
}

}
