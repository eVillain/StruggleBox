#pragma once

#include "CoreIncludes.h"
#include "RendererDefines.h"

class RenderCore;

class GBuffer
{
public:
	GBuffer(RenderCore& renderCore);

	void Initialize(uint32_t width, uint32_t height);
	void Terminate();

	void Bind();
	void Clear();
	void UnBind();

	void BindDraw();
	void BindRead();

	//void Resize(uint32_t width, uint32_t height);

	const uint32_t GetFBO() const { return m_fboHandle; };

	const uint32_t GetAlbedo() const { return m_albedoTextureHandle; };
	const uint32_t GetMaterial() const { return m_materialTextureHandle; };
	const uint32_t GetDepth() const { return m_depthTextureHandle; };
	const uint32_t GetNormal() const { return m_normalTextureHandle; };

	const TextureID getAlbedoTextureID() const { return m_albedoTextureID; }
	const TextureID getMaterialTextureID() const { return m_materialTextureID; }
	const TextureID getDepthTextureID() const { return m_depthTextureID; }
	const TextureID getNormalTextureID() const { return m_normalTextureID; }

private:
	RenderCore& m_renderCore;

	uint32_t m_width;
	uint32_t m_height;

	uint32_t m_fboHandle;
	uint32_t m_albedoTextureHandle;
	uint32_t m_materialTextureHandle;
	uint32_t m_depthTextureHandle;
	uint32_t m_normalTextureHandle;

	TextureID m_albedoTextureID;
	TextureID m_materialTextureID;
	TextureID m_depthTextureID;
	TextureID m_normalTextureID;
};
