#include "DebugDraw.h"
#include "Log.h"
#include "Renderer.h"
#include "Shader.h"
#include "VertBuffer.h"
#include "GLErrorUtil.h"

const int MAX_VERTS = 1024;

DebugDraw::DebugDraw(
	Renderer& renderer) :
	_renderer(renderer),
	_lines2DCount(0),
	_lines3DCount(0)
{
	Log::Info("[DebugDraw] constructor, instance at %p", this);
	//_lineShader = _shaders.load("d_default_vColor.vsh", "d_default.fsh");
	_lineBuffer2D = (ColorVertexData*)malloc(sizeof(ColorVertexData) * MAX_VERTS);
	_vao = _renderer.addVertexArray();
	_lineVB = _renderer.addVertBuffer(VertexDataType::ColorVerts);
}

DebugDraw::~DebugDraw()
{
	Log::Info("[DebugDraw] destructor, instance at %p", this);

}

void DebugDraw::line2D(glm::vec3 a, glm::vec3 b, Color aColor, Color bColor)
{
	_lineBuffer2D[_lines2DCount++] = {
		a.x, a.y, a.z, 1.0,
		aColor.r, aColor.g, aColor.b, aColor.a
	};
	_lineBuffer2D[_lines2DCount++] = {
		b.x, b.y, b.z, 1.0,
		bColor.r, bColor.g, bColor.b, bColor.a
	};
}

void DebugDraw::line2D(glm::vec3 a, glm::vec3 b, Color color)
{
	line2D(a, b, color, color);
}

void DebugDraw::line2D(glm::vec2 a, glm::vec2 b, float z, Color aColor, Color bColor)
{
	_lineBuffer2D[_lines2DCount++] = {
		a.x, a.y, z, 1.0,
		aColor.r, aColor.g, aColor.b, aColor.a
	};
	_lineBuffer2D[_lines2DCount++] = {
		b.x, b.y, z, 1.0,
		bColor.r, bColor.g, bColor.b, bColor.a
	};
}

void DebugDraw::line2D(glm::vec2 a, glm::vec2 b, float z, Color color)
{
	line2D(a, b, z, color, color);
}

void DebugDraw::flush()
{
	CHECK_GL_ERROR();

	glm::mat4 mvp2D;
	_renderer.GetUIMatrix(mvp2D);

	_lineShader->begin();
	_lineShader->setUniformM4fv("MVP", mvp2D);

	glBindVertexArray(_vao);
	_lineVB->bind();
	_lineVB->upload(
		_lineBuffer2D,
		sizeof(ColorVertexData)*_lines2DCount,
		true);

	glDrawArrays(GL_LINES, 0, _lines2DCount);
	CHECK_GL_ERROR();
	_lines2DCount = 0;
}

