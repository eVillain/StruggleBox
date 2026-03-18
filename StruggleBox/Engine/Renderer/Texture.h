#pragma once

#include <string>
#include "GFXDefines.h"

class Texture
{
public:
    Texture(
		uint32_t textureID,
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		uint32_t format,
		uint32_t wrapMethod,
		uint32_t minFilter,
		uint32_t magFilter,
		uint32_t mipLevel);
    ~Texture();
 
	uint32_t getGLTextureID() const { return m_glTextureID; };
	uint32_t getWidth() const { return m_width; };
	uint32_t getHeight() const { return m_height; };
	uint32_t getDepth() const { return m_depth; };

private:
	uint32_t m_glTextureID;
	uint32_t m_width;      
	uint32_t m_height;     
	uint32_t m_depth;     
	uint32_t m_format;     
	uint32_t m_wrapMethod; 
	uint32_t m_minFilter;  
	uint32_t m_magFilter;
	uint32_t m_mipLevel;   
};
