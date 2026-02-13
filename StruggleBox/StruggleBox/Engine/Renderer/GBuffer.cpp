#include "GBuffer.h"

#include "ArenaOperators.h"
#include "RenderCore.h"
#include "GLUtils.h"
#include "Texture2D.h"

#include <iostream>

GBuffer::GBuffer(RenderCore& renderCore)
: m_renderCore(renderCore)
{
}

void GBuffer::Initialize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	/* --- Generate our frame buffer textures --- */
	m_albedoTextureHandle = GLUtils::GenerateTextureRGBAF(m_width, m_height);
	m_materialTextureHandle = GLUtils::GenerateTextureRGBAF(m_width, m_height);
	m_depthTextureHandle = GLUtils::GenerateTextureDepth(m_width, m_height);
	m_normalTextureHandle = GLUtils::GenerateTextureNormal(m_width, m_height);

	Texture2D* textureAlbedo = CUSTOM_NEW(Texture2D, m_renderCore.getAllocator())(m_albedoTextureHandle, m_width, m_height, GL_RGBA, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, 0);
	m_albedoTextureID = m_renderCore.getTextureCache().addTexture(textureAlbedo, "GBufferAlbedo");
	Texture2D* textureMaterial = CUSTOM_NEW(Texture2D, m_renderCore.getAllocator())(m_materialTextureHandle, m_width, m_height, GL_RGBA, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, 0);
	m_materialTextureID = m_renderCore.getTextureCache().addTexture(textureMaterial, "GBufferMaterial");
	Texture2D* textureDepth = CUSTOM_NEW(Texture2D, m_renderCore.getAllocator())(m_depthTextureHandle, m_width, m_height, GL_RGBA, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, 0);
	m_depthTextureID = m_renderCore.getTextureCache().addTexture(textureDepth, "GBufferDepth");
	Texture2D* textureNormal = CUSTOM_NEW(Texture2D, m_renderCore.getAllocator())(m_normalTextureHandle, m_width, m_height, GL_RGB, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, 0);
	m_normalTextureID = m_renderCore.getTextureCache().addTexture(textureNormal, "GBufferNormal");

	// Generate main rendering frame buffer
	glGenFramebuffers(1, &m_fboHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle); // Bind our frame buffer
	// Attach the texture render_texture to the color buffer in our frame buffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_albedoTextureHandle, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_materialTextureHandle, 0);
	// Attach the normal buffer to our frame buffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_normalTextureHandle, 0);
	// Attach the depth buffer depth_texture to our frame buffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_depthTextureHandle, 0);

	// Set the list of draw buffers.
	// Passing GL_DEPTH_ATTACHMENT as second buffer returns an invalid enum glerror
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, (GLenum*)DrawBuffers);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("[GBuffer] Couldn't create render frame buffer, code:%i", status);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind our new frame buffer
}

void GBuffer::Terminate()
{
	if (m_albedoTextureHandle) glDeleteTextures(1, &m_albedoTextureHandle);
	if (m_materialTextureHandle) glDeleteTextures(1, &m_materialTextureHandle);
	if (m_depthTextureHandle) glDeleteTextures(1, &m_depthTextureHandle);
	if (m_normalTextureHandle) glDeleteTextures(1, &m_normalTextureHandle);
	if (m_fboHandle) glDeleteFramebuffers(1, &m_fboHandle);

	Texture2D* textureAlbedo = m_renderCore.getTextureCache().getTextureByID(m_albedoTextureID);
	CUSTOM_DELETE(textureAlbedo, m_renderCore.getAllocator());
	m_renderCore.getTextureCache().removeTexture(m_albedoTextureID);

	Texture2D* textureMaterial = m_renderCore.getTextureCache().getTextureByID(m_materialTextureID);
	CUSTOM_DELETE(textureMaterial, m_renderCore.getAllocator());
	m_renderCore.getTextureCache().removeTexture(m_materialTextureID);

	Texture2D* textureDepth = m_renderCore.getTextureCache().getTextureByID(m_depthTextureID);
	CUSTOM_DELETE(textureDepth, m_renderCore.getAllocator());
	m_renderCore.getTextureCache().removeTexture(m_depthTextureID);

	Texture2D* textureNormal = m_renderCore.getTextureCache().getTextureByID(m_normalTextureID);
	CUSTOM_DELETE(textureNormal, m_renderCore.getAllocator());
	m_renderCore.getTextureCache().removeTexture(m_normalTextureID);
}

void GBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle);
}

void GBuffer::BindDraw()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboHandle);
}

void GBuffer::BindRead()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboHandle);
}

void GBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::Clear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
