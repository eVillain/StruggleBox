#include "TextAtlas.h"
#include <cstring>

TextAtlas::TextAtlas(const TextureID textureID, const uint32_t width, const uint32_t height)
	: m_textureID(textureID)
	, m_width(width)
	, m_height(height)
{
	memset(m_glyphs, 0, sizeof(Glyph) * 128);
}
