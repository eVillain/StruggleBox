#include "TextAtlasLoader.h"

#include "Allocator.h"
#include "ArenaOperators.h"
#include "Log.h"
#include "GLUtils.h"
#include "CoreIncludes.h"
#include "TextAtlas.h"
#include "TextureCache.h"
#include "Texture2D.h"

#include "MathUtils.h"
#include "GLErrorUtil.h"

bool TextAtlasLoader::s_initialized = false;
FT_Library TextAtlasLoader::s_freeType;
const uint32_t TextAtlasLoader::MAX_TEXT_ATLAS_WIDTH = 2048;

TextAtlas* TextAtlasLoader::load(const std::string& filename, const uint8_t fontHeight, Allocator& allocator, TextureCache& textureCache)
{
    if (!s_initialized)
    {
        initialize();
        if (!s_initialized)
        {
            return nullptr;
        }
    }

    // We need to load the font and create a new atlas, let's do it
    Log::Info("[TextAtlasLoader] Loading font from file: %s", filename.c_str());
    
    FT_Face face = NULL;
    FT_Error err = FT_New_Face(s_freeType, filename.c_str(), 0, &face);
    if(err)
    {
        Log::Error("[TextAtlasLoader] Could not open font: %s - error: %i", filename.c_str(), err);
        return NULL;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    calculateAtlasSize(face, fontHeight, width, height);
    //width = MathUtils::round_up_to_power_of_2(width);
    //height = MathUtils::round_up_to_power_of_2(height);

    Log::Info("Generated text atlas sized: %i x %i", width, height);
    CHECK_GL_ERROR();

    /* Create a texture that will be used to hold all ASCII glyphs */
    // Our text shaders can read the red channel for the alpha value
    std::vector<GLubyte> emptyData(width * height, 0);
    const uint32_t textureIDGL = GLUtils::createTexture(width, height, 0, GL_RED, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST, false, &emptyData[0]);
    CHECK_GL_ERROR();

    /* We require 1 byte alignment when uploading texture data */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_GL_ERROR();

    char buf[256];
#ifdef _WIN32
    sprintf_s(buf, "%s%i", filename.c_str(), fontHeight);
#else
    sprintf(buf, "%s%i", filename.c_str(), fontHeight);
#endif
    const std::string fontNameAndSize = std::string(buf);

    Texture2D* texture = CUSTOM_NEW(Texture2D, allocator)(textureIDGL, width, height, GL_RED, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureID textureID = textureCache.addTexture(texture, fontNameAndSize);
    TextAtlas* atlas = CUSTOM_NEW(TextAtlas, allocator)(textureID, width, height);

    glBindTexture(GL_TEXTURE_2D, textureIDGL);
    CHECK_GL_ERROR();
    readGlyphs(face, fontHeight, width, height, atlas->getGlyphs());
    CHECK_GL_ERROR();

    // Store newly created atlas
    //_atlases[fontKey] = atlas;
    
    // Clean up face
    FT_Done_Face(face);
    
    return atlas;
}

void TextAtlasLoader::terminate()
{
    if (!s_initialized)
    {
        return;
    }
    FT_Done_FreeType(s_freeType);
}

void TextAtlasLoader::initialize()
{
    if (s_initialized)
    {
        return;
    }
    if (FT_Init_FreeType(&s_freeType) != 0) 
    {
        Log::Error("[TextAtlasLoader] Could not initialize FreeType library!");
        return;
    }
    s_initialized = true;
}

/* Find minimum size for a texture holding all visible ASCII characters */
void TextAtlasLoader::calculateAtlasSize(FT_Face& face, const uint8_t fontHeight, uint32_t& width, uint32_t& height)
{
    FT_Set_Pixel_Sizes(face, 0, fontHeight);
    FT_GlyphSlot g = face->glyph;

    uint32_t roww = 0;
    uint32_t rowh = 0;

    for (int i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            Log::Error("[TextAtlasLoader] Loading character %c (ASCII %i) failed!", i, i);
            continue;
        }

        if (roww + g->bitmap.width + 1 >= MAX_TEXT_ATLAS_WIDTH)
        {
            width = std::max(width, roww);
            height += rowh;
            roww = 0;
            rowh = 0;
        }
        roww += g->bitmap.width + 1;
        rowh = std::max(rowh, g->bitmap.rows);
    }

    width = std::max(width, roww);
    height += rowh;
}

void TextAtlasLoader::readGlyphs(FT_Face& face, const uint8_t fontHeight, const uint32_t width, uint32_t height, Glyph* glyphs)
{
    FT_Set_Pixel_Sizes(face, 0, fontHeight);
    FT_GlyphSlot g = face->glyph;
    /* Paste all glyph bitmaps into the texture, remembering the offset */
    uint32_t ox = 0;
    uint32_t oy = 0;

    uint32_t rowh = 0;

    for (int i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            Log::Error("[TextAtlasLoader] Loading character %c failed!", i);
            continue;
        }

        if (ox + g->bitmap.width + 1 >= MAX_TEXT_ATLAS_WIDTH)
        {
            oy += rowh;
            rowh = 0;
            ox = 0;
        }
        uint32_t w = g->bitmap.width;
        uint32_t r = g->bitmap.rows;

        glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, w, r, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        glyphs[i].ax = g->advance.x >> 6;
        glyphs[i].ay = g->advance.y >> 6;

        glyphs[i].bw = g->bitmap.width;
        glyphs[i].bh = g->bitmap.rows;

        glyphs[i].bl = g->bitmap_left;
        glyphs[i].bt = g->bitmap_top;

        glyphs[i].tx = (float)ox / (float)width;
        glyphs[i].ty = (float)oy / (float)height;

        rowh = std::max(rowh, r);
        ox += g->bitmap.width + 1;
    }
}
