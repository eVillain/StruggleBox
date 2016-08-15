#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include "GFXIncludes.h"
#include "GFXDefines.h"
#include "Color.h"
#include <memory>

class Renderer;
class ShaderManager;

class Shader;
class VertBuffer;

class DebugDraw
{
public:
	DebugDraw(
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<ShaderManager> shaders);
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
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<ShaderManager> _shaders;

	std::shared_ptr<Shader> _lineShader;

	std::shared_ptr<VertBuffer> _lineVB;
	unsigned int _vbo, _vao;
	ColorVertexData* _lineBuffer2D;
	ColorVertexData* _lineBuffer3D;
	unsigned int _lines2DCount, _lines3DCount;
};


#endif
