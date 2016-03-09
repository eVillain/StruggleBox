//
//  FontAtlas.cpp
//  Bloxelizer
//
//  Created by Ville-Veikko Urrila on 1/17/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//
#include "FontAtlas.h"
#include <algorithm>

FontAtlas::FontAtlas(FT_Face face, int height, bool shaders) {
    FT_Set_Pixel_Sizes(face, 0, height);
    FT_GlyphSlot g = face->glyph;
    
    int roww = 0;
    int rowh = 0;
    w = 0;
    h = 0;
    
    // Clear up character information array
    memset(gInfo, 0, sizeof gInfo);
    
    /* Find minimum size for a texture holding all visible ASCII characters */
    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            printf("[FontAtlas] Loading character %c (ASCII %i) failed!\n", i, i);
            continue;
        }
        if (roww + g->bitmap.width + 1 >= MAX_ATLAS_WIDTH) {
            w = std::max(w, roww);
            h += rowh;
            roww = 0;
            rowh = 0;
        }
        roww += g->bitmap.width + 1;
        rowh = std::max(rowh, g->bitmap.rows);
    }
    
    w = std::max(w, roww);
    h += rowh;
    
    /* Create a texture that will be used to hold all ASCII glyphs */
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    
    // Actually generate image
    if ( shaders ) {
        // Our text shaders can read the red channel for the alpha value
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    } else {
        // Fixed pipeline needs a luminance-alpha texture so we have to expand the data a bit
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 0 );
    }
    
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
        if ( shaders ) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
		}
		else {
			// Here we perform the data expansion:
			// For all non-transparent pixels we put white and store alpha
			int numPixels = g->bitmap.width * g->bitmap.rows;
			unsigned char* expandedData = new unsigned char[numPixels * 2];
            for (int i=0; i<numPixels; i++) {
                GLubyte value = g->bitmap.buffer[i];
                if ( value != 0) {
                    expandedData[i*2+0] = 255;  // Full luminance
                } else {
                    expandedData[i*2+0] = 0;    // Pixel was transparent
                }
                expandedData[i*2+1] = value;    // Store alpha
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expandedData);
        }
        gInfo[i].ax = g->advance.x >> 6;
        gInfo[i].ay = g->advance.y >> 6;
        
        gInfo[i].bw = g->bitmap.width;
        gInfo[i].bh = g->bitmap.rows;
        
        gInfo[i].bl = g->bitmap_left;
        gInfo[i].bt = g->bitmap_top;
        
        gInfo[i].tx = ox / (float)w;
        gInfo[i].ty = oy / (float)h;
        
        rowh = std::max(rowh, g->bitmap.rows);
        ox += g->bitmap.width + 1;
    }
    
//    fprintf(stderr, "Generated a %d x %d (%d kb) texture atlas\n", w, h, w * h / 1024);
}

FontAtlas::~FontAtlas() {
    glDeleteTextures(1, &texID);
}
