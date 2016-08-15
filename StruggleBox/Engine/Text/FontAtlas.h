#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H

/**
 * The atlas class holds a texture that contains the visible US-ASCII characters
 * of a certain font rendered with a certain character height.
 * It also contains an array that contains all the information necessary to
 * generate the appropriate vertex and texture coordinates for each character.
 *
 * After the constructor no FreeType functions are called.
 */

#include "GFXDefines.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>

// Maximum texture width - TODO: Get this at runtime from opengl context
#define MAX_ATLAS_WIDTH 1024

typedef struct GlyphInfo {
		float ax;       // advance.x
		float ay;       // advance.y
        
		float bw;       // bitmap.width;
		float bh;       // bitmap.height;
        
		float bl;       // bitmap_left;
		float bt;       // bitmap_top;
        
		float tx;       // x offset of glyph in texture coordinates
		float ty;       // y offset of glyph in texture coordinates
} GlyphInfo;

class FontAtlas
{
public:
    FontAtlas(FT_Face face,
              int height);
    ~FontAtlas();
    
    const GLuint GetTextureID() const { return _texID; };
    const int GetWidth() const { return _width; };
    const int GetHeight() const { return _height; };
    const GlyphInfo* GetGlyphInfo() const { return _glyphInfo; };

private:
    int _width;     // in pixels
    int _height;    // in pixels
    GlyphInfo _glyphInfo[128];   // character information
    GLuint _texID;  // GL texture object handle
};

#endif
