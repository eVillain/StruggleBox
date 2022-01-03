#pragma once

#include "RendererDefines.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>

class Allocator;
class TextAtlas;
class TextureCache;
struct Glyph;

class TextAtlasLoader
{
public:
    static TextAtlas* load(const std::string& filename, const uint8_t fontHeight, Allocator& allocator, TextureCache& textureCache);

    static void terminate();
private:
    static const uint32_t MAX_TEXT_ATLAS_WIDTH;

    static FT_Library s_freeType;
    static bool s_initialized;

    static void initialize();
    static void calculateAtlasSize(FT_Face& face, const uint8_t fontHeight, uint32_t& width, uint32_t& height);
    static void readGlyphs(FT_Face& face, const uint8_t fontHeight, const uint32_t width, const uint32_t height, Glyph* glyphs);
};
