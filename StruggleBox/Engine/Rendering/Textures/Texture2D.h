#pragma once

#include <string>
#include "GFXDefines.h"

class Texture2D
{
public:
    Texture2D(
		uint32_t textureID,
		uint32_t width,
		uint32_t height,
		uint32_t format,
		uint32_t wrapMethod,
		uint32_t minFilter,
		uint32_t magFilter,
		uint32_t mipLevel);
    ~Texture2D();
 
	uint32_t getGLTextureID( void ) const { return m_glTextureID; };
	uint32_t getWidth( void ) const { return m_width; };
	uint32_t getHeight( void ) const { return m_height; };

private:
	uint32_t m_glTextureID;
	uint32_t m_width;      
	uint32_t m_height;     
	uint32_t m_format;     
	uint32_t m_wrapMethod; 
	uint32_t m_minFilter;  
	uint32_t m_magFilter;
	uint32_t m_mipLevel;   
};
