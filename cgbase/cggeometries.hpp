/* Copyright (C) 2018 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de> */

#ifndef CGGEOMETRIES_HPP
#define CGGEOMETRIES_HPP

#include <QVector>

namespace Cg
{

/* Returns a quad geometry.
 * This quad has the corners (-1, -1, 0), (+1, -1, 0), (+1, +1, 0), (-1, +1, 0). */
void quad(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices = 40);

/* Returns a cube geometry.
 * This geometry is centered on the origin and fills [-1,+1]^3. */
void cube(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices = 40);

/* Returns a disk geometry.
 * This geometry is centered on the origin and fills [-1,+1]^3. */
void disk(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        float innerRadius = 0.2f,
        int slices = 40);

/* Returns a sphere geometry.
 * This geometry is centered on the origin and fills [-1,+1]^3. */
void sphere(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices = 40, int stacks = 20);

/* Returns a cylinder geometry.
 * This geometry is centered on the origin and fills [-1,+1]^3. */
void cylinder(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices = 40, int stacks = 20);

/* Returns a cone geometry.
 * This geometry is centered on the origin and fills [-1,+1]^3. */
void cone(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        int slices = 40, int stacks = 20);

/* Returns a torus geometry.
 * This geometry is centered on the origin and fills [-1,+1]^3. */
void torus(
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices,
        float innerRadius = 0.4f,
        int sides = 40, int rings = 40);

}

#endif
