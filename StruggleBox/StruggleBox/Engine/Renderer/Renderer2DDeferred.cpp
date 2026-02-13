#include "Renderer2DDeferred.h"

#include "DefaultShaders.h"
#include "Shader.h"

const glm::mat4 s_projection2D = glm::ortho<float>(-0.5f, 0.5f, -0.5f, 0.5f, -1.f, 1.f);

Renderer2DDeferred::Renderer2DDeferred(RenderCore& renderCore, Allocator& allocator, Options& options)
	: Renderer2D(renderCore, allocator)
	, m_options(options)
	, m_frameBuffer(renderCore, "Main2DFrameBuffer")
	, m_lightSystem(*this)
	, m_lightPassShaderID(0)
	, m_lightingEnabled(true)
{
}

Renderer2DDeferred::~Renderer2DDeferred()
{
}

void Renderer2DDeferred::initialize()
{
	Renderer2D::initialize();
	m_frameBuffer.initialize(m_renderCore.getRenderResolution().x, m_renderCore.getRenderResolution().y);
	m_lightSystem.initialize();
	m_lightPassShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, lightPass2DFragmentShaderSource, "LightPass2DShader");
}

void Renderer2DDeferred::terminate()
{
	m_renderCore.removeShader(m_lightPassShaderID);
	m_frameBuffer.terminate();
	m_lightSystem.terminate();
	Renderer2D::terminate();
}

void Renderer2DDeferred::update(const double deltaTime)
{
	Renderer2D::update(deltaTime);
}

void Renderer2DDeferred::flush(const std::vector<Shape2D*>& occluders)
{
	if (m_lightingEnabled)
	{
		m_frameBuffer.bindAndClear(COLOR_NONE);

		for (const Shape2D* shape : occluders)
		{
			if (shape->getType() == Shape2DType::Circle)
			{
				Shape2DCircle* circle = (Shape2DCircle*)shape;
				drawCircleColor(shape->getPosition(), shape->getRotation(), circle->getRadius(), shape->getOutlineColor(), shape->getColor(), shape->getDepth(), 8);
			}
			else if (shape->getType() == Shape2DType::Segment)
			{
				Shape2DSegment* segment = (Shape2DSegment*)shape;
				drawLine(segment->getBegin(), segment->getEnd(), segment->getColor(), segment->getDepth());
			}
		}
	}

	Renderer2D::flush();

	if (m_lightingEnabled)
	{
		m_lightSystem.renderLighting(occluders);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, m_renderCore.getRenderResolution().x, m_renderCore.getRenderResolution().y);

		TempVertBuffer buffer;
		m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(buffer, 4);
		TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)buffer.data;
		dataPtr[0] = { glm::vec3(-0.5,-0.5, 0.0), glm::vec2(0, 0) };
		dataPtr[1] = { glm::vec3(0.5,-0.5, 0.0), glm::vec2(1, 0) };
		dataPtr[2] = { glm::vec3(0.5, 0.5, 0.0), glm::vec2(1, 1) };
		dataPtr[3] = { glm::vec3(-0.5, 0.5, 0.0), glm::vec2(0, 1) };
		const Shader* lightPassShader = m_renderCore.getShaderByID(m_lightPassShaderID);
		lightPassShader->begin();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_lightSystem.getFrameBuffer().getGLTexture());
		lightPassShader->setUniform1iv("lightMap", 1);
		m_renderCore.draw(m_lightPassShaderID, m_frameBuffer.getTextureID(), m_texturedVertsDrawDataID, s_projection2D, DrawMode::TriangleFan, dataPtr, 0, 4, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);
	}
}
