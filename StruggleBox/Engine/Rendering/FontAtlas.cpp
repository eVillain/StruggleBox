#include "FontAtlas.h"
#include <algorithm>

FontAtlas::FontAtlas(FT_Face face,
                     int height)
{
    FT_Set_Pixel_Sizes(face, 0, height);
    FT_GlyphSlot g = face->glyph;
    
    int roww = 0;
    int rowh = 0;
    _width = 0;
    _height = 0;
    
    // Clear up character information array
    memset(_glyphInfo, 0, sizeof(_glyphInfo));
    
    /* Find minimum size for a texture holding all visible ASCII characters */
    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            printf("[FontAtlas] Loading character %c (ASCII %i) failed!\n", i, i);
            continue;
        }
        if (roww + g->bitmap.width + 1 >= MAX_ATLAS_WIDTH) {
            _width = std::max(_width, roww);
            _height += rowh;
            roww = 0;
            rowh = 0;
        }
        roww += g->bitmap.width + 1;
        rowh = std::max(rowh, g->bitmap.rows);
    }
    
    _width = std::max(_width, roww);
    _height += rowh;
    
    /* Create a texture that will be used to hold all ASCII glyphs */
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &_texID);
    glBindTexture(GL_TEXTURE_2D, _texID);
    
    // Our text shaders can read the red channel for the alpha value
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _width, _height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    
    /* We require 1 byte alignment when uploading texture data */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    /* Clamping to edges is important to prevent artifacts when scaling */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    /* Linear filtering usually looks best for text */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    /* Paste all glyph bitmaps into the texture, remembering the offset */
    int ox = 0;
    int oy = 0;
    
    rowh = 0;
    
    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed!\n", i);
            continue;
        }
        
        if (ox + g->bitmap.width + 1 >= MAX_ATLAS_WIDTH) {
            oy += rowh;
            rowh = 0;
            ox = 0;
        }
        int w = g->bitmap.width;
        int r = g->bitmap.rows;
        glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, w, r, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
        
        _glyphInfo[i].ax = g->advance.x >> 6;
        _glyphInfo[i].ay = g->advance.y >> 6;
        
        _glyphInfo[i].bw = g->bitmap.width;
        _glyphInfo[i].bh = g->bitmap.rows;
        
        _glyphInfo[i].bl = g->bitmap_left;
        _glyphInfo[i].bt = g->bitmap_top;
        
        _glyphInfo[i].tx = ox / (float)_width;
        _glyphInfo[i].ty = oy / (float)_height;
        
        rowh = std::max(rowh, r);
        ox += g->bitmap.width + 1;
    }
    
//    printf("Generated a %d x %d (%d kb) texture atlas\n", w, h, w * h / 1024);
}

FontAtlas::~FontAtlas()
{
    glDeleteTextures(1, &_texID);
}
