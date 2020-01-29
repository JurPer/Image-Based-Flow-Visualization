/* Copyright (C) 2018 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de> */

#ifndef CGTOOLS_HPP
#define CGTOOLS_HPP

#include <QVector>
#include <QMatrix4x4>
#include <QString>

#include "cgopenglwidget.hpp"

namespace Cg
{

/* Initialize a Cg::OpenGLWidget(). Calling this function is necessary iff
 * you want the optional VR support to work. */
void init(int& argc, char* argv[], OpenGLWidget* widget);

/* Load geometry from an OBJ file. Only positions, normals, and texture
 * coordinates are imported. Materials are ignored.
 * The data is suitable for rendering in GL_TRIANGLES mode. */
bool loadObj(const QString& fileName,
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices);

/* Load a texture from an image file. Optionally a mipmap is generated automatically;
 * the texture filtering parameters will be set accordingly. */
unsigned int loadTexture(const QString& fileName, bool generateMipMap = true, bool mirrorY = true);

/* Load an image file into an existing texture, e.g. into cube map components. */
bool loadIntoTexture(const QString& fileName, GLenum target = GL_TEXTURE_2D, bool mirrorY = true);

/* Create a vertex array object from geometry data that is suitable for rendering
 * in GL_TRIANGLES mode. This function can pre-transform the geometry with a
 * transformation matrix. */
unsigned int createVertexArrayObject(
        const QVector<float>& positions,
        const QVector<float>& normals,
        const QVector<float>& texCoords,
        const QVector<unsigned int>& indices,
        const QMatrix4x4& transformationMatrix = QMatrix4x4());

/* Load a text file into a string. This is mainly useful to load GLSL shader code. */
QString loadFile(const QString& fileName);

/* Automatically prepend a '#version' directive to the given shader code string.
 * This version directive will match the current OpenGL or OpenGL ES context version.
 * This is useful to have the same shader code work across both OpenGL and OpenGL ES. */
QString prependGLSLVersion(const QString& shaderCode);

/* Check for OpenGL errors. This function is usually not called directly. Instead,
 * use the CG_ASSERT_GLCHECK() macros. This macro will do nothing when built in
 * Release mode, but when build in Debug mode it will check for an OpenGL error and
 * abort the program with a meaningful error message if necessary. */
void glCheck(const char* callingFunction, const char* file, int line);
#ifdef QT_NO_DEBUG
# define CG_ASSERT_GLCHECK() /* nothing */
#else
# define CG_ASSERT_GLCHECK() Cg::glCheck(Q_FUNC_INFO, __FILE__, __LINE__)
#endif

/* For a given geometry in GL_TRIANGLES mode with indices, create vertex normals.
 * You can choose the following methods:
 * 0: The vertex normal is set to the normal of the first face that the vertex
 *    belongs to.
 * 1: The vertex normal is set to the average of the face normals of all faces
 *    that the vertex belongs to.
 * 2: The vertex normal is set to the weighted average of the face normals of
 *    all faces that the vertex belongs to. The weights depend on the angle that
 *    each face contributes to the vertex. See Thürmer, G., Wüthrich, C.,
 *    Computing Vertex Normals from Polygonal Facets, Journal of Graphics Tools,
 *    3(1), 1998 pps. 43-46. This method is generally considered to result in
 *    high quality normals for many different types of geometry (see e.g. the
 *    comments in Meshlab).
 */
QVector<float> createNormals(const QVector<float>& positions,
        const QVector<unsigned int>& indices, int method = 2);

/* For a given geometry in GL_TRIANGLES mode with indices, create a new index list
 * that provides GL_TRIANGLES_ADJACENCY. This is useful for geometry shaders.
 * If a neighboring triangle is not found for an edge of a given triangle, the
 * neighbor for that edge will be set to the triangle itself, only in opposite direction.
 * WARNING: the current naive implementation is O(n²). Only use once on initialization,
 * and do not use with larger models. */
QVector<unsigned int> createAdjacency(const QVector<unsigned int>& indices);

}

#endif
