#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include "CoreIncludes.h"
#include "GFXDefines.h"
#include "Color.h"
#include <memory>

class Renderer;

class Shader;
class VertBuffer;

class DebugDraw
{
public:
	DebugDraw(Renderer& renderer);
	~DebugDraw();

	// 2D Lines
	void line2D(
		glm::vec3 a,
		glm::vec3 b,
		Color aColor,
		Color bColor);

	void line2D(
		glm::vec3 a,
		glm::vec3 b,
		Color color);

	void line2D(
		glm::vec2 a,
		glm::vec2 b,
		float z,
		Color aColor,
		Color bColor);

	void line2D(
		glm::vec2 a,
		glm::vec2 b,
		float z,
		Color color);

	void flush();

private:
	Renderer& _renderer;

	Shader* _lineShader;

	VertBuffer* _lineVB;
	unsigned int _vbo, _vao;
	ColorVertexData* _lineBuffer2D;
	ColorVertexData* _lineBuffer3D;
	unsigned int _lines2DCount, _lines3DCount;
};


#endif
