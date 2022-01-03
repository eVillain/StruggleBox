#pragma once

#include "RendererDefines.h"
#include <stdint.h>

typedef struct Glyph {
    float ax;       // advance.x
    float ay;       // advance.y

    float bw;       // bitmap.width;
    float bh;       // bitmap.height;

    float bl;       // bitmap_left;
    float bt;       // bitmap_top;

    float tx;       // x offset of glyph in texture coordinates
    float ty;       // y offset of glyph in texture coordinates
} Glyph;

class TextAtlas
{
public:
    TextAtlas(const TextureID textureID, const uint32_t width, const uint32_t height);
    
    const TextureID getTextureID() const { return m_textureID; };
    const uint32_t getWidth() const { return m_width; };
    const uint32_t getHeight() const { return m_height; };
    Glyph* getGlyphs() { return m_glyphs; };

private:
    TextureID m_textureID;
    uint32_t m_width;
    uint32_t m_height;  
    Glyph m_glyphs[128];
};
