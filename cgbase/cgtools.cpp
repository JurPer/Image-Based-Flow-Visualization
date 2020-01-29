/* Copyright (C) 2013, 2014, 2015, 2018 Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de> */

#include <cstdlib>
#include <cmath>

#include <QTemporaryFile>
#include <QOpenGLExtraFunctions>
#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#ifdef CG_HAVE_QVR
# include <qvr/manager.hpp>
#endif

#include "tiny_obj_loader.h"

#include "cgtools.hpp"

namespace Cg
{

void init(int& argc, char* argv[], OpenGLWidget* widget)
{
    // Start the application, either via QVR or standalone
#ifdef CG_HAVE_QVR
    QVRManager* manager = new QVRManager(argc, argv);
    if (!manager->init(widget)) {
        qFatal("Cannot initialize QVR manager");
    }
#else
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    widget->show();
#endif
}

bool loadObj(const QString& fileName,
        QVector<float>& positions,
        QVector<float>& normals,
        QVector<float>& texCoords,
        QVector<unsigned int>& indices)
{
    // Handle special case: if the file is a Qt resource (name begins with ':'),
    // then tiny_obj_loader cannot handle it, so we write the data to a temporary
    // file first. The createNativeFile() function returns 0 if the file is already
    // native.
    QString plainFileName;
    QTemporaryFile* tmpFile = QTemporaryFile::createNativeFile(fileName);
    if (tmpFile) {
        QFileInfo tmpFileInfo(*tmpFile);
        plainFileName = tmpFileInfo.filePath();
    } else {
        plainFileName = fileName;
    }

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string errMsg;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &errMsg, qPrintable(plainFileName), nullptr)) {
        if (!errMsg.empty() && errMsg.back() == '\n')
            errMsg.back() = '\0';
        qCritical("Failed to load %s: %s", qPrintable(plainFileName), errMsg.c_str());
        return false;
    }
    if (shapes.size() == 0) {
        qCritical("No shapes in %s", qPrintable(plainFileName));
        return false;
    }

    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    bool ok = true;
    bool haveNormals = true;
    bool haveTexCoords = true;
    std::map<std::tuple<int, int, int>, unsigned int> indexTupleMap;
    for (size_t i = 0; ok && i < shapes.size(); i++) {
        for (size_t j = 0; ok && j < shapes[i].mesh.indices.size(); j++) {
            tinyobj::index_t index = shapes[i].mesh.indices[j];
            int vertexIndex = index.vertex_index;
            int normalIndex = index.normal_index;
            if (normalIndex < 0)
                haveNormals = false;
            int texCoordIndex = index.texcoord_index;
            if (texCoordIndex < 0)
                haveTexCoords = false;
            std::tuple<int, int, int> indexTuple = std::make_tuple(vertexIndex, normalIndex, texCoordIndex);
            std::map<std::tuple<int, int, int>, unsigned int>::iterator it = indexTupleMap.find(indexTuple);
            if (it == indexTupleMap.end()) {
                unsigned int newIndex = indexTupleMap.size();
                if (newIndex == std::numeric_limits<unsigned int>::max()) {
                    qCritical("More vertices than I can handle in %s", qPrintable(plainFileName));
                    ok = false;
                    break;
                }
                positions.append(attrib.vertices[3 * vertexIndex + 0]);
                positions.append(attrib.vertices[3 * vertexIndex + 1]);
                positions.append(attrib.vertices[3 * vertexIndex + 2]);
                if (haveNormals) {
                    normals.append(attrib.normals[3 * normalIndex + 0]);
                    normals.append(attrib.normals[3 * normalIndex + 1]);
                    normals.append(attrib.normals[3 * normalIndex + 2]);
                }
                if (haveTexCoords) {
                    texCoords.append(attrib.texcoords[2 * texCoordIndex + 0]);
                    texCoords.append(attrib.texcoords[2 * texCoordIndex + 1]);
                }
                indices.append(newIndex);
                indexTupleMap.insert(std::make_pair(indexTuple, newIndex));
            } else {
                indices.append(it->second);
            }
        }
    }
    if (!ok) {
        positions.clear();
        normals.clear();
        texCoords.clear();
        indices.clear();
    } else {
        if (!haveNormals) {
            normals = createNormals(positions, indices);
        }
        if (!haveTexCoords) {
            texCoords.resize(positions.size() / 3 * 2);
            for (int i = 0; i < texCoords.size(); i++)
                texCoords[i] = 0.0f;
        }
    }
    return ok;
}

bool loadIntoTexture(const QString& fileName, GLenum target, bool mirrorY)
{
    QOpenGLExtraFunctions* gl = QOpenGLContext::currentContext()->extraFunctions();

    QImage img;
    if (!img.load(fileName))
        return false;

    if (mirrorY)
        img = img.mirrored(false, true);
    img = img.convertToFormat(QImage::Format_RGBA8888);
    gl->glTexImage2D(target, 0, GL_RGBA8,
            img.width(), img.height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
    return true;
}

unsigned int loadTexture(const QString& fileName, bool generateMipMap, bool mirrorY)
{
    QOpenGLExtraFunctions* gl = QOpenGLContext::currentContext()->extraFunctions();

    unsigned int tex = 0;
    QImage img;
    if (img.load(fileName)) {
        if (mirrorY)
            img = img.mirrored(false, true);
        img = img.convertToFormat(QImage::Format_RGBA8888);
        gl->glGenTextures(1, &tex);
        gl->glBindTexture(GL_TEXTURE_2D, tex);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                img.width(), img.height(), 0,
                GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
        if (generateMipMap) {
            gl->glGenerateMipmap(GL_TEXTURE_2D);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        } else {
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    return tex;
}

unsigned int createVertexArrayObject(
        const QVector<float>& positions,
        const QVector<float>& normals,
        const QVector<float>& texCoords,
        const QVector<unsigned int>& indices,
        const QMatrix4x4& transformationMatrix)
{
    Q_ASSERT(positions.size() % 3 == 0);
    Q_ASSERT(positions.size() > 0);
    Q_ASSERT(positions.size() == normals.size());
    Q_ASSERT(positions.size() / 3 == texCoords.size() / 2);
    Q_ASSERT(indices.size() > 0);
    Q_ASSERT(indices.size() % 3 == 0);

    QOpenGLExtraFunctions* gl = QOpenGLContext::currentContext()->extraFunctions();
    QVector<float> data;
    int vertexCount = positions.size() / 3;

    unsigned int vao;
    gl->glGenVertexArrays(1, &vao);
    gl->glBindVertexArray(vao);

    GLuint positionBuffer;
    gl->glGenBuffers(1, &positionBuffer);
    gl->glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    if (transformationMatrix.isIdentity()) {
        gl->glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), positions.data(), GL_STATIC_DRAW);
    } else {
        data.resize(vertexCount * 3);
        for (int j = 0; j < vertexCount; j++) {
            QVector3D v = QVector3D(positions[3 * j + 0], positions[3 * j + 1], positions[3 * j + 2]);
            QVector3D w = transformationMatrix * v;
            data[3 * j + 0] = w.x();
            data[3 * j + 1] = w.y();
            data[3 * j + 2] = w.z();
        }
        gl->glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), data.data(), GL_STATIC_DRAW);
    }
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl->glEnableVertexAttribArray(0);

    GLuint normalBuffer;
    gl->glGenBuffers(1, &normalBuffer);
    gl->glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    if (transformationMatrix.isIdentity()) {
        gl->glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), normals.data(), GL_STATIC_DRAW);
    } else {
        data.resize(vertexCount * 3);
        QMatrix4x4 normalMatrix = QMatrix4x4(transformationMatrix.normalMatrix());
        for (int j = 0; j < vertexCount; j++) {
            QVector3D v = QVector3D(normals[3 * j + 0], normals[3 * j + 1], normals[3 * j + 2]);
            QVector3D w = normalMatrix * v;
            data[3 * j + 0] = w.x();
            data[3 * j + 1] = w.y();
            data[3 * j + 2] = w.z();
        }
        gl->glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), data.data(), GL_STATIC_DRAW);
    }
    gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl->glEnableVertexAttribArray(1);

    GLuint texcoordBuffer;
    gl->glGenBuffers(1, &texcoordBuffer);
    gl->glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
    gl->glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(float), texCoords.data(), GL_STATIC_DRAW);
    gl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    gl->glEnableVertexAttribArray(2);

    GLuint indexBuffer;
    gl->glGenBuffers(1, &indexBuffer);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.count() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    gl->glBindVertexArray(0);
    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return vao;
}

QString loadFile(const QString& fileName)
{
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QTextStream in(&f);
    return in.readAll();
}

QString prependGLSLVersion(const QString& shaderCode)
{
    int major = QOpenGLContext::currentContext()->format().majorVersion();
    int minor = QOpenGLContext::currentContext()->format().minorVersion();
    QString version = QString("#version %1%2%3\n").arg(major).arg(minor).arg(0);
    QString modifiedCode = shaderCode;
    modifiedCode.prepend(version);
    return modifiedCode;
}

void glCheck(const char* callingFunction, const char* file, int line)
{
    QOpenGLExtraFunctions* gl = QOpenGLContext::currentContext()->extraFunctions();
    GLenum err = gl->glGetError();
    if (err != GL_NO_ERROR) {
        qCritical("%s:%d: OpenGL error 0x%04X in the following function:", file, line, err);
        qCritical("%s", callingFunction);
        std::exit(1);
    }
}

QVector<float> createNormals(const QVector<float>& positions,
        const QVector<unsigned int>& indices, int method)
{
    QVector<QVector3D> faceNormals; // normal for each face
    faceNormals.reserve(indices.size() / 3);
    QVector<QVector<unsigned int>> vertexFaces;         // list of faces for each vertex
    vertexFaces.resize(positions.size() / 3);

    for (int i = 0; i < indices.size(); i += 3) {
        int faceIndex = i / 3;
        QVector3D v[3];
        v[0] = QVector3D(positions[3 * indices[i + 0] + 0], positions[3 * indices[i + 0] + 1], positions[3 * indices[i + 0] + 2]);
        v[1] = QVector3D(positions[3 * indices[i + 1] + 0], positions[3 * indices[i + 1] + 1], positions[3 * indices[i + 1] + 2]);
        v[2] = QVector3D(positions[3 * indices[i + 2] + 0], positions[3 * indices[i + 2] + 1], positions[3 * indices[i + 2] + 2]);
        QVector3D e0 = v[1] - v[0];
        QVector3D e1 = v[2] - v[0];
        QVector3D e2 = e1 - e0;
        if (QVector3D::dotProduct(e0, e0) <= 0.0f
                || QVector3D::dotProduct(e1, e1) <= 0.0f
                || QVector3D::dotProduct(e2, e2) <= 0.0f) {
            faceNormals.append(QVector3D(0.0f, 0.0f, 0.0f));
        } else {
            faceNormals.append(QVector3D::crossProduct(e0, e1).normalized());
        }
        vertexFaces[indices[i + 0]].append(faceIndex);
        vertexFaces[indices[i + 1]].append(faceIndex);
        vertexFaces[indices[i + 2]].append(faceIndex);
    }

    QVector<float> vertexNormals;
    vertexNormals.reserve(positions.size());
    for (unsigned int i = 0; i < static_cast<unsigned int>(positions.size() / 3); i++) {
        QVector3D n = QVector3D(0.0f, 0.0f, 0.0f);
        if (vertexFaces[i].size() == 0) {
            // vertex without a face: will not be rendered anyway
        } else if (vertexFaces[i].size() == 1) {
            // only one face: no choice in methods
            n = faceNormals[vertexFaces[i][0]];
        } else {
            if (method == 2) {
                for (int j = 0; j < vertexFaces[i].size(); j++) {
                    unsigned int faceIndex = vertexFaces[i][j];
                    unsigned int faceIndices[3] = {
                        indices[3 * faceIndex + 0],
                        indices[3 * faceIndex + 1],
                        indices[3 * faceIndex + 2] };
                    QVector3D e0, e1;
                    if (i == faceIndices[0]) {
                        e0 = QVector3D(positions[3 * faceIndices[1] + 0], positions[3 * faceIndices[1] + 1], positions[3 * faceIndices[1] + 2]);
                        e1 = QVector3D(positions[3 * faceIndices[2] + 0], positions[3 * faceIndices[2] + 1], positions[3 * faceIndices[2] + 2]);
                    } else if (i == faceIndices[1]) {
                        e0 = QVector3D(positions[3 * faceIndices[2] + 0], positions[3 * faceIndices[2] + 1], positions[3 * faceIndices[2] + 2]);
                        e1 = QVector3D(positions[3 * faceIndices[0] + 0], positions[3 * faceIndices[0] + 1], positions[3 * faceIndices[0] + 2]);
                    } else {
                        e0 = QVector3D(positions[3 * faceIndices[0] + 0], positions[3 * faceIndices[0] + 1], positions[3 * faceIndices[0] + 2]);
                        e1 = QVector3D(positions[3 * faceIndices[1] + 0], positions[3 * faceIndices[1] + 1], positions[3 * faceIndices[1] + 2]);
                    }
                    e0 = e0 - QVector3D(positions[3 * i + 0], positions[3 * i + 1], positions[3 * i + 3]);
                    e1 = e1 - QVector3D(positions[3 * i + 0], positions[3 * i + 1], positions[3 * i + 3]);
                    if (QVector3D::dotProduct(e0, e0) <= 0.0f || QVector3D::dotProduct(e1, e1) <= 0.0f)
                        continue;
                    float x = QVector3D::dotProduct(e0.normalized(), e1.normalized());
                    if (x < -1.0f)
                        x = -1.0f;
                    else if (x > 1.0f)
                        x = 1.0f;
                    float alpha = std::acos(x);
                    n += alpha * faceNormals[faceIndex];
                }
            }
            if (method == 1 || (method == 2 && QVector3D::dotProduct(n, n) <= 0.0f)) { // use equal weights for each face
                n = QVector3D(0.0f, 0.0f, 0.0f);
                for (int j = 0; j < vertexFaces[i].size(); j++) {
                    int faceIndex = vertexFaces[i][j];
                    n += faceNormals[faceIndex];
                }
            }
            if (method == 0 || QVector3D::dotProduct(n, n) <= 0.0f) { // use first face normal
                int faceIndex = vertexFaces[i][0];
                n = faceNormals[faceIndex];
            }
            n.normalize();
#if 0
            // Sanity check: the angle between our computed normal and each face
            // normal must always be smaller than 90 degrees. If that is not the
            // case, simply fall back to the first face normal.
            for (size_t j = 0; j < vertexFaces[i].size(); j++) {
                size_t faceIndex = vertexFaces[i][j];
                vec3 fn = faceNormals[faceIndex];
                if (dot(n, fn) >= half_pi<float>()) {
                    n = faceNormals[vertexFaces[i][0]];
                    break;
                }
            }
#endif
        }
        vertexNormals.append(n.x());
        vertexNormals.append(n.y());
        vertexNormals.append(n.z());
    }

    return vertexNormals;
}

QVector<unsigned int> createAdjacency(const QVector<unsigned int>& indices)
{
    Q_ASSERT(indices.size() % 3 == 0);
    QVector<unsigned int> indicesWithAdjacency(indices.size() / 3 * 6);
    for (int t = 0; t < indices.size() / 3; t++) {
        unsigned int v[3] = { indices[3 * t + 0], indices[3 * t + 1], indices[3 * t + 2] };
        unsigned int nv[3] = { v[2], v[0], v[1] }; // neighbor triangle vertices for edges 0, 1, 2
        for (int nt = 0; nt < indices.size() / 3; nt++) {
            if (nt == t)
                continue;
            unsigned int test_nv[3] = { indices[3 * nt + 0], indices[3 * nt + 1], indices[3 * nt + 2] };
            // Test edges. Neighbor triangles must have the same orientation as the
            // current triangle, so we only have to check one edge direction.
            // edge 0
            if (nv[0] == v[2]) {
                if (v[0] == test_nv[1] && v[1] == test_nv[0])
                    nv[0] = test_nv[2];
                else if (v[0] == test_nv[2] && v[1] == test_nv[1])
                    nv[0] = test_nv[0];
                else if (v[0] == test_nv[0] && v[1] == test_nv[2])
                    nv[0] = test_nv[1];
            }
            // edge 1
            if (nv[1] == v[0]) {
                if (v[1] == test_nv[1] && v[2] == test_nv[0])
                    nv[1] = test_nv[2];
                else if (v[1] == test_nv[2] && v[2] == test_nv[1])
                    nv[1] = test_nv[0];
                else if (v[1] == test_nv[0] && v[2] == test_nv[2])
                    nv[1] = test_nv[1];
            }
            // edge 2
            if (nv[2] == v[1]) {
                if (v[2] == test_nv[1] && v[0] == test_nv[0])
                    nv[2] = test_nv[2];
                else if (v[2] == test_nv[2] && v[0] == test_nv[1])
                    nv[2] = test_nv[0];
                else if (v[2] == test_nv[0] && v[0] == test_nv[2])
                    nv[2] = test_nv[1];
            }
            if (nv[0] != v[2] && nv[1] != v[0] && nv[2] != v[1])
                break;
        }
        indicesWithAdjacency[6 * t + 0] = v[0];
        indicesWithAdjacency[6 * t + 1] = nv[0];
        indicesWithAdjacency[6 * t + 2] = v[1];
        indicesWithAdjacency[6 * t + 3] = nv[1];
        indicesWithAdjacency[6 * t + 4] = v[2];
        indicesWithAdjacency[6 * t + 5] = nv[2];
    }
    return indicesWithAdjacency;
}

}
