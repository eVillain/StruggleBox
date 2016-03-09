//
//  FontAtlas.h
//  Bloxelizer
//
//  Created by Ville-Veikko Urrila on 1/17/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//
/**
 * The atlas class holds a texture that contains the visible US-ASCII characters
 * of a certain font rendered with a certain character height.
 * It also contains an array that contains all the information necessary to
 * generate the appropriate vertex and texture coordinates for each character.
 *
 * After the constructor is run, you don't need to use any FreeType functions anymore.
 */

#ifndef NGN_FONT_ATLAS_H
#define NGN_FONT_ATLAS_H

#include "GFXDefines.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>

// Maximum texture width
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

class FontAtlas {
private:
    
	GLuint texID;       // texture object
    
	int w;              // width of texture in pixels
	int h;              // height of texture in pixels
    
	GlyphInfo gInfo[128];    // character information

public:
    // Constructor
    FontAtlas(FT_Face face, int height, bool shaders);
    // Destructor
    ~FontAtlas( void );
    
    GLuint GetTextureID( void ) { return texID; };
    int GetWidth( void ) { return w; };
    int GetHeight( void ) { return h; };
    GlyphInfo* GetGlyphInfo( void ) { return gInfo; };
};

#endif
