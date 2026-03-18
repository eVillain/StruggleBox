#include "Texture.h"


#include "Log.h"
#include <iostream>

Texture::Texture(
	uint32_t textureID,
	uint32_t width,
	uint32_t height,
	uint32_t depth,
	uint32_t format,
	uint32_t wrapMethod,
	uint32_t minFilter,
	uint32_t magFilter,
	uint32_t mipLevel)
	: m_glTextureID(textureID)
	, m_width(width)
	, m_height(height)
	, m_depth(depth)
	, m_format(format)
	, m_wrapMethod(wrapMethod)
	, m_minFilter(minFilter)
	, m_magFilter(magFilter)
	, m_mipLevel(mipLevel)
{
}

Texture::~Texture()
{
}

