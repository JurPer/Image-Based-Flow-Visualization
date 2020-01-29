#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>

#include <QApplication>
#include <QSurfaceFormat>
#include <QKeyEvent>
#include <QtMath>

#include "cgbase/cggeometries.hpp"
#include "cgbase/cgtools.hpp"

#include "flowvis.hpp"
#include <iostream>


FlowVis::FlowVis() :
	_time_cell(0),
	_time_cell_in_texture(-1),
	_first_iteration(true),
	_meshIteration(false),
	_time_is_passing(true),
	_blendOn(true),
	_indexCount(0),
	_vaoMesh(0),
	_indexCountMesh(0),
	_nMesh(20),
	_stepSize(0.5f),
	_screenWidth(800),
	_screenHeight(600)
{
	_data.resize(_x_cells * _y_cells * _t_cells * 2);
	FILE* f = std::fopen("flow.raw", "rb");
	if (f) {
		fread(_data.data(), 1, _data.size() * sizeof(float), f);
		fclose(f);
	}
}

FlowVis::~FlowVis()
{
}

void FlowVis::initializeGL()
{
	Cg::OpenGLWidget::initializeGL();
	QVector3D center((_x_end + _x_start) / 2.0f, (_y_end + _y_start) / 2.0f, 0.0f);
	float radius = _x_end - center.x();
	navigator()->initialize(center, radius);

	// Set up buffer objects for the geometry
	const QVector<float> positions({
			_x_start, _y_end, 0.0f,   _x_end, _y_end, 0.0f,
			_x_end, _y_start, 0.0f,   _x_start, _y_start, 0.0f
		});
	const QVector<float> normals({
			0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f
		});
	const QVector<float> texcoords({
			0.0f, 0.0f,   1.0f, 0.0f,
			1.0f, 1.0f,   0.0f, 1.0f
		});
	const QVector<unsigned int> indices({
			0, 1, 3, 1, 2, 3
		});
	_vertexArrayObject = Cg::createVertexArrayObject(positions, normals, texcoords, indices);
	_indexCount = indices.size();
	CG_ASSERT_GLCHECK();

	// Set up geometry for a quad in NDC. Can be used to render into framebuffer color targets
	const QVector<float> pos({
			-1.0f, -1.0f, 0.0f,	1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,	-1.0f, 1.0f, 0.0f
		});
	const QVector<float> norm({
			0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f
		});
	const QVector<float> texco({
			0.0f, 0.0f,   1.0f, 0.0f,
			1.0f, 1.0f,   0.0f, 1.0f
		});
	const QVector<unsigned int> indic({
			0, 1, 3, 1, 2, 3
		});
	_vaoQuad = Cg::createVertexArrayObject(pos, norm, texco, indic);
	CG_ASSERT_GLCHECK();

	// create a sparse noise texture that marks critical points
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	CG_ASSERT_GLCHECK();

	QVector<float> vectorsRGBA;
	QVector<float> rgba(4, 1.0f);	
	// initialize random seed
	srand(time(NULL));
	for (int y = 0; y < _y_cells; y++) {
		for (int x = 0; x < _x_cells; x++) {
			float length = getFlowVector(_time_cell, y, x).length();
			// critical point if vector length zero
			if (length <= 0.01)
				rgba = { 1.0f, 0.0f, 0.0f, 1.0f };
			else if (rand() % 100 < 1) {
				rgba = { 0.0f, 1.0f, 0.0f, 1.0f };
			}
			else
				rgba = { 0.0f, 0.0f, 0.0f, 1.0f };
			vectorsRGBA += rgba;
		}
	}
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _x_cells, _y_cells, 0, GL_RGBA, GL_FLOAT, vectorsRGBA.data());

	// Load images into textures QVector to cycle through later
	_texImages.append(Cg::loadTexture(":/img/seeding_points", false, false));
	_texImages.append(texture);
	_texImages.append(Cg::loadTexture(":/img/whiteNoise", false, false));
	_texImages.append(Cg::loadTexture(":/img/whiteNoiseResized", false, false));
	_texImages.append(Cg::loadTexture(":/img/perlinNoise", false, false));
	_texImages.append(Cg::loadTexture(":/img/simplexNoise", false, false));
	_texImages.append(Cg::loadTexture(":/img/grid_biggest", false, false));
	_texImages.append(Cg::loadTexture(":/img/grid_big", false, false));
	_texImages.append(Cg::loadTexture(":/img/grid", false, false));
	_texImages.append(Cg::loadTexture(":/img/checkerBoard", false, false));
	_currentImage = _texImages[0];
	CG_ASSERT_GLCHECK();

	// Set up the programmable pipeline
	_prg.addShaderFromSourceCode(QOpenGLShader::Vertex,
		Cg::prependGLSLVersion(Cg::loadFile(":vs.glsl")));
	_prg.addShaderFromSourceCode(QOpenGLShader::Fragment,
		Cg::prependGLSLVersion(Cg::loadFile(":fs.glsl")));
	_prg.link();
	CG_ASSERT_GLCHECK();

	// Set up the programmable pipeline for the mesh shaders
	_prgMesh.addShaderFromSourceCode(QOpenGLShader::Vertex,
		Cg::prependGLSLVersion(Cg::loadFile(":vsMesh.glsl")));
	_prgMesh.addShaderFromSourceCode(QOpenGLShader::Fragment,
		Cg::prependGLSLVersion(Cg::loadFile(":fsMesh.glsl")));
	_prgMesh.link();
	CG_ASSERT_GLCHECK();

	/*
	-------- Add two framebuffers to render the mesh offscreen
	*/

	// save default framebuffer
	int buffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);

	// initial screen resolution
	GLsizei SCR_WIDTH = _screenWidth;
	GLsizei SCR_HEIGHT = _screenHeight;
	// initialize two framebuffer objects
	glGenFramebuffers(2, _meshFB);
	glGenTextures(2, _meshTexture);
	for (GLuint i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, _meshFB[i]);
		glBindTexture(GL_TEXTURE_2D, _meshTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _meshTexture[i], 0);

		// - Finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

	/*
	// Tell OpenGL which color attachment to use for rendering
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	*/

	// rebind default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);
}

QVector2D FlowVis::getFlowVector(int t, int y, int x)
{
	return QVector2D(
		_data[2 * (t * _y_cells * _x_cells + y * _x_cells + x) + 0],
		_data[2 * (t * _y_cells * _x_cells + y * _x_cells + x) + 1]);
}

void FlowVis::paintGL(const QMatrix4x4& P, const QMatrix4x4& V, int w, int h)
{
	// Resize the textures of the FBOs if resolution changed
	if (w != _screenWidth || h != _screenHeight) {
		_screenWidth = w;
		_screenHeight = h;
		fboTexResize();
	}

	Cg::OpenGLWidget::paintGL(P, V, w, h);
	// Set up view
	glViewport(0, 0, w, h);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);

	// save default framebuffer
	int buffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);

	if (_first_iteration || _time_cell != _time_cell_in_texture) {
		// bind offscreen framebuffers for the mesh
		glBindFramebuffer(GL_FRAMEBUFFER, _meshFB[_meshIteration]);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind the program (linked shaders) to render off screen
		_prgMesh.bind();
		_prgMesh.setUniformValue("tex", 0);
		createMesh();
		_time_cell_in_texture = _time_cell;

		if (_blendOn) {
			glEnable(GL_BLEND);
			// use different blending for seeding textures
			if (_currentImage <= _texImages[0])
				glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
			else 
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else {
			glDisable(GL_BLEND);
		}

		// Draw distorted mesh with either the current image or the previous result
		_prgMesh.setUniformValue("alpha", 1.0f);
		glBindVertexArray(_vaoMesh);
		glBindTexture(GL_TEXTURE_2D, _first_iteration ? _currentImage : _meshTexture[!_meshIteration]);
		glDrawElements(GL_TRIANGLES, _indexCountMesh, GL_UNSIGNED_INT, 0);
		CG_ASSERT_GLCHECK();

		if (_blendOn) {
			// Draw initial texture into a quad (NDC) for blending
			_prgMesh.setUniformValue("alpha", 0.1f);
			glBindVertexArray(_vaoQuad);
			glBindTexture(GL_TEXTURE_2D, _currentImage);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			CG_ASSERT_GLCHECK();
		}

		_meshIteration = !_meshIteration;
		if (_first_iteration)
			_first_iteration = false;
	}

	// rebind default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);
	glDisable(GL_BLEND);

	// Render: draw triangles with a texture
	_prg.bind();
	_prg.setUniformValue("projection_matrix", P);
	_prg.setUniformValue("max_length", 1.5f); // just a guess
	_prg.setUniformValue("tex", 0); // just a guess
	// bind the vao that has two triangles
	glBindVertexArray(_vertexArrayObject);

	// first window (top) to see mesh method
	QMatrix4x4 modViewMesh = V;
	modViewMesh.translate(0.0f, 0.5f, 0.0f);
	_prg.setUniformValue("modelview_matrix", modViewMesh);
	glBindTexture(GL_TEXTURE_2D, _meshTexture[0]);
	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, 0);

	// a window (bottom) to show default texture used for texture advection
	QMatrix4x4 modviewMatrix = V;
	modviewMatrix.translate(0.0f, -0.6f, 0.0f);
	_prg.setUniformValue("modelview_matrix", modviewMatrix);
	//glBindTexture(GL_TEXTURE_2D, _currentImage);
	// testing
	glBindTexture(GL_TEXTURE_2D, _currentImage);
	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, 0);
	CG_ASSERT_GLCHECK();

	// Update to animate, turned on/off with Key_T
	if (_time_is_passing) {
		_time_cell++;
		if (_time_cell >= _t_cells)
			_time_cell = 0;
	}
}

/* Transforms an NDC input into tex coords between 0.0 and 1.0 */
float texTF(float input) {
	return (input + 1.0f) / 2.0f;
}

/* Transforms a [0, dimLength] input into tex coords between 0.0 and 1.0 */
float texTF(float input, float dimLength) {
	return input / dimLength;
}

/* Transforms the vector coordinates from [0, width || height] into NDC according to width and height */
QVector2D vecToNDC(QVector2D vector, float width, float height) {
	return QVector2D(vector.x() * 2 / width - 1.0f, vector.y() * 2 / height - 1.0f);
}

/* Bilinearly interpolates coordinates before getting the flow vector */
QVector2D FlowVis::getFlowVector(float x, float y, int t) {
	/*
	// Nearest Neighbour
	int cx = round(x);
	int cy = round(y);
	cx = std::min(std::max(cx, 0), _x_cells - 1);
	cy = std::min(std::max(cy, 0), _y_cells - 1);

	return getFlowVector(t, cy, cx);
	*/

	// Biliniear Interpolation in 2D as it is described in the SciVis script part 02 page 20
	int x00 = floor(x);
	x00 = std::min(std::max(x00, 0), _x_cells - 1);
	int x10 = ceil(x);
	x10 = std::min(std::max(x10, 0), _x_cells - 1);
	int y00 = floor(y);
	y00 = std::min(std::max(y00, 0), _y_cells - 1);
	int y01 = ceil(y);
	y01 = std::min(std::max(y01, 0), _y_cells - 1);
	QVector2D f00 = getFlowVector(t, y00, x00);
	QVector2D f10 = getFlowVector(t, y00, x10);
	QVector2D f01 = getFlowVector(t, y01, x00);
	QVector2D f11 = getFlowVector(t, y01, x10);
	float alpha = 0.0f;
	if (x10 - x00 != 0)
		alpha = (x - x00) / (x10 - x00);
	QVector2D f0 = alpha * f10 + (1 - alpha) * f00;
	QVector2D f1 = alpha * f11 + (1 - alpha) * f01;

	float beta = 0.0f;
	if (y01 - y00 != 0)
		beta = (y - y00) / (y01 - y00);
	QVector2D f = beta * f1 + (1 - beta) * f0;

	return f;
}

/* Use Heun integration to better approximate flow vectors */
QVector2D FlowVis::heun(float stepSize, QVector2D position) {
	QVector2D result;
	QVector2D speed = getFlowVector(position.x(), position.y(), _time_cell);

	result = position + stepSize * getFlowVector(position.x(), position.y(), _time_cell);
	// TODO: timecell + stepsize?
	QVector2D speedNext = getFlowVector(result.x(), result.y(), _time_cell);

	result = position + (stepSize * 0.5 * (speed + speedNext));

	return result;
}

/* Creates a mesh and distorts it in the direction of the flow (in NDC) */
void FlowVis::createMesh() {
	float width = _x_cells;
	float height = _y_cells;
	int NMESH_Y = _nMesh;
	float DIST = height / NMESH_Y;
	int NMESH_X = width / DIST;

	QVector<float> positions, normals, texcoords;
	QVector<unsigned int> indices;
	_indexCountMesh = 0;
	QVector2D flowVec;
	QVector3D pos;
	float offset = 0.2f;

	// add a border to the left edge to fix texture injection bug
	for (int i = 0; i < NMESH_Y; i++) {
		float x1 = 0;
		float x2 = offset;

		float y1 = DIST * i;
		float y2 = y1 + DIST;

		flowVec = QVector2D(x1, y2);
		pos = QVector3D(vecToNDC(flowVec, width, height));
		positions.append({ pos.x(), pos.y(), pos.z() });
		normals.append({ 0.0f, 0.0f, 1.0f });
		texcoords.append({ texTF(x1, width), texTF(y2, height) });

		flowVec = heun(_stepSize, QVector2D(x2, y2));
		pos = QVector3D(vecToNDC(flowVec, width, height));
		positions.append({ pos.x(), pos.y(), pos.z() });
		normals.append({ 0.0f, 0.0f, 1.0f });
		texcoords.append({ texTF(x2, width), texTF(y2, height) });

		flowVec = heun(_stepSize, QVector2D(x2, y1));
		pos = QVector3D(vecToNDC(flowVec, width, height));
		positions.append({ pos.x(), pos.y(), pos.z() });
		normals.append({ 0.0f, 0.0f, 1.0f });
		texcoords.append({ texTF(x2, width), texTF(y1, height) });

		flowVec = QVector2D(x1, y1);
		pos = QVector3D(vecToNDC(flowVec, width, height));
		positions.append({ pos.x(), pos.y(), pos.z() });
		normals.append({ 0.0f, 0.0f, 1.0f });
		texcoords.append({ texTF(x1, width), texTF(y1, height) });

		indices.append({ _indexCountMesh, _indexCountMesh + 1, _indexCountMesh + 3, _indexCountMesh + 1, _indexCountMesh + 2, _indexCountMesh + 3 });
		_indexCountMesh += 4;
	}

	for (int i = 0; i < NMESH_X; i++) {
		// plus offset when using the border
		float x1 = DIST * i + offset;
		float x2 = x1 + DIST;

		for (int j = 0; j < NMESH_Y; j++) {
			float y1 = DIST * j;
			float y2 = y1 + DIST;

			flowVec = heun(_stepSize, QVector2D(x1, y2));
			pos = QVector3D(vecToNDC(flowVec, width, height));
			positions.append({ pos.x(), pos.y(), pos.z() });
			normals.append({ 0.0f, 0.0f, 1.0f });
			texcoords.append({ texTF(x1, width), texTF(y2, height) });

			flowVec = heun(_stepSize, QVector2D(x2, y2));
			pos = QVector3D(vecToNDC(flowVec, width, height));
			positions.append({ pos.x(), pos.y(), pos.z() });
			normals.append({ 0.0f, 0.0f, 1.0f });
			texcoords.append({ texTF(x2, width), texTF(y2, height) });

			flowVec = heun(_stepSize, QVector2D(x2, y1));
			pos = QVector3D(vecToNDC(flowVec, width, height));
			positions.append({ pos.x(), pos.y(), pos.z() });
			normals.append({ 0.0f, 0.0f, 1.0f });
			texcoords.append({ texTF(x2, width), texTF(y1, height) });

			flowVec = heun(_stepSize, QVector2D(x1, y1));
			pos = QVector3D(vecToNDC(flowVec, width, height));
			positions.append({ pos.x(), pos.y(), pos.z() });
			normals.append({ 0.0f, 0.0f, 1.0f });
			texcoords.append({ texTF(x1, width), texTF(y1, height) });

			indices.append({ _indexCountMesh, _indexCountMesh + 1, _indexCountMesh + 3, _indexCountMesh + 1, _indexCountMesh + 2, _indexCountMesh + 3 });
			_indexCountMesh += 4;
		}
	}
	
	// Delete if not initial call, so the name can be used again
	if (_vaoMesh != 0)
		glDeleteVertexArrays(1, &_vaoMesh);
	
	_vaoMesh = Cg::createVertexArrayObject(positions, normals, texcoords, indices);
	_indexCountMesh = indices.size();
}

/* Resizes the textures of the FBOs */
void FlowVis::fboTexResize() {
	GLsizei SCR_WIDTH = _screenWidth;
	GLsizei SCR_HEIGHT = _screenHeight;

	for (int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, _meshTexture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	}
}

void FlowVis::keyPressEvent(QKeyEvent* event)
{
	Cg::OpenGLWidget::keyPressEvent(event);
	int key = event->key();
	switch (key) {
	case Qt::Key_T:
		_time_is_passing = !_time_is_passing;
		break;
	case Qt::Key_F:
		_first_iteration = true;
		_meshIteration = false;
		break;
	case Qt::Key_B:
		_first_iteration = true;
		_meshIteration = false;
		_blendOn = !_blendOn;
		if (_blendOn)
			_stepSize = 1.0f;
		else
			_stepSize = 0.5f;
		break;
	case Qt::Key_Plus:
		_nMesh++;
		_first_iteration = true;
		_meshIteration = false;
		break;
	case Qt::Key_Minus:
		if (_nMesh > 2) {
			_nMesh--;
			_first_iteration = true;
			_meshIteration = false;
		}
		break;
	case Qt::Key_H:
		_stepSize -= 0.05;
		if (_stepSize < 0.05)
			_stepSize = 0.05;
		break;
	case Qt::Key_J:
		_stepSize += 0.05;
		break;
	}
	// Key pressed is between 1 and 9; change the current image and reset
	if (key >= 49 && key <= 57) {
		_currentImage = _texImages[(key - 49) % _texImages.size()];
		_first_iteration = true;
		_meshIteration = false;
		//navigator()->reset();
	}
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QSurfaceFormat format;
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setVersion(4, 5);
	QSurfaceFormat::setDefaultFormat(format);
	FlowVis example;
	Cg::init(argc, argv, &example);
	return app.exec();
}
