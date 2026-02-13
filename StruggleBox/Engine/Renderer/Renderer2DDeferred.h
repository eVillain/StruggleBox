#pragma once

#include "Renderer2D.h"
#include "Lighting2DDeferred.h"
#include "FrameBuffer.h"
#include "Options.h"

class Renderer2DDeferred : public Renderer2D
{
public:
	Renderer2DDeferred(RenderCore& renderCore, Allocator& allocator, Options& options);
	~Renderer2DDeferred();

	void initialize();
	void terminate();

	void update(const double deltaTime);
	void flush(const std::vector<Shape2D*>& occluders);

	Lighting2DDeferred& getLightSystem() { return m_lightSystem; }

private:
	Options& m_options;
	FrameBuffer m_frameBuffer;
	Lighting2DDeferred m_lightSystem;

	ShaderID m_lightPassShaderID;
	bool m_lightingEnabled;
};
