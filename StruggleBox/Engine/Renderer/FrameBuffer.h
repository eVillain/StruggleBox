#pragma once

#include "RendererDefines.h"
#include <stdint.h>
#include <string>

class RenderCore;

class FrameBuffer
{
public:
	FrameBuffer(RenderCore& renderCore, const std::string& name);
	~FrameBuffer();

	void initialize(const uint32_t width, const uint32_t height);
	void initialize(const uint32_t width, const uint32_t height, const uint32_t depthTexture);
	void terminate();

	void bind() const;
	void bindAndClear(const Color& clearColor) const;

	uint32_t getFBOHandle() const { return m_fboHandle; }
	uint32_t getGLTexture() const { return m_textureHandle; }
	TextureID getTextureID() const { return m_textureID; }
	uint32_t getWidth() const { return m_width; }
	uint32_t getHeight() const { return m_height; }

private:
	RenderCore& m_renderCore;
	uint32_t m_fboHandle;
	uint32_t m_textureHandle;
	uint32_t m_width;
	uint32_t m_height;

	const std::string m_textureName;
	TextureID m_textureID;
	void setupTexture(const uint32_t width, const uint32_t height);
};

