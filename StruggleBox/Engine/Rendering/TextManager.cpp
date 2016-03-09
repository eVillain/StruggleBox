#include <string>

#include "TextManager.h"
#include "HyperVisor.h"

#include "Renderer.h"
#include "FileUtil.h"
#include "Options.h"

#include "ShaderManager.h"
#include <glm/gtc/matrix_transform.hpp>     // glm::translate, glm::rotate, glm::scale

static Renderer* g_renderer = NULL;
// Vector of text blobs to render
typedef std::vector<TextBlob> TextBlobVector;
// Container for text labels
static TextBlobVector textLabels;

/**********************************************************************
 * Default shader programs
 *********************************************************************/
const GLchar *text_vertex_shader21[] = {
    "#version 120\n"
    "attribute vec4 coord;"
    "varying vec2 texcoord;"
    "void main(void) {"
    "gl_Position = vec4(coord.xy, 0, 1);"
    "texcoord = coord.zw;"
    "}"
};
const GLchar *text_frag_shader21[] = {
    "#version 120\n"
    "varying vec2 texcoord;"
    "uniform sampler2D tex;"
    "uniform vec4 color;"
    "void main(void) {"
    "gl_FragColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) * color;"
    "}"
};
const GLchar *text_vertex_shader32[] = {
    "#version 330 core\n"
    "layout(location = 0) in vec4 vCoord;"
    "layout(location = 1) in vec2 tCoord;"
    "uniform mat4 MVP;"
    "out vec2 texCoord;"
    "void main(void) {"
    "gl_Position = MVP * vec4(vCoord.xyz,1);"
    "texCoord = tCoord;"
    "}"
};
const GLchar *text_frag_shader32[] = {
    "#version 330 core\n"
    "in vec2 texCoord;"
    "out vec4 fragColor;"
    "uniform sampler2D tex;"
    "uniform vec4 color;"
    "void main(void) {"
    "fragColor = vec4(1, 1, 1, texture(tex, texCoord).r) * color;"
    "}"
};

TextManager::TextManager()
{
    // The next blob ID to be generated
    _nextId = 0;

    // Counter for each frame
    blobsRendered = 0;
    
    initialized = false;
    shadersAvailable = false;
    
    // Shader mode vars TODO:MOVE TO RENDERER
    textShader = NULL;
    vertexArrayID = 0;
    vertexBufferID = 0;
}

void TextManager::Initialize(Locator& locator)
{
    if ( initialized ) {
        Terminate();     // Already had a previous init, clean up first
    }
    
    g_renderer = locator.Get<Renderer>();
    if ( g_renderer == NULL ) {
        printf("[TextMan] ERROR: No renderer hooked on init");
        return;
    }
    // Check if shaders available
    if ( locator.Get<Options>()->getOption<bool>("r_useShaders") ) {
        shadersAvailable = true;
        textShader = new Shader();
        textShader->InitFromSource(text_vertex_shader32, text_frag_shader32);
        if ( textShader->GetProgram() == 0 ) {
            printf("[TextMan] failed to load text shader program\n");
            return;
        }
        printf("[TextMan] generating vertex arrays\n");
        // Create vertex array object
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);
        // Create the vertex buffer object
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        
        /* Set up the VBO for our vertex data */
        glEnableVertexAttribArray( 0 );
        glEnableVertexAttribArray( 1 );
        glBindVertexArray(0);
    } else {
        shadersAvailable = false;
    }
    
    // Initialize FreeType library
    if( FT_Init_FreeType(&ft) != 0 ) {
        printf("[TextMan] Could not init freetype library\n");
    }
    initialized = true;
}

void TextManager::Terminate()
{
    if ( !initialized ) return;
    AtlasIter atl;
    for ( atl = atlases.begin(); atl != atlases.end(); atl++ ) {
        delete atl->second;
    }
    atlases.clear();
    
    FT_Done_FreeType(ft);
    if ( shadersAvailable ) {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        printf("[TextMan] releasing vertex arrays\n");
        delete textShader;
        textShader = NULL;
    }
    
    textLabels.clear();     // Empty out all blobs
    initialized = false;
}

void TextManager::Update( double delta ) {
    TextBlobVector::iterator it=textLabels.begin();
    while ( it != textLabels.end() ) {
        if ( it->timer != 0.0 ) {
            if ( it->timer <= delta ) {
                // label timer over, remove
                textLabels.erase(it);
                continue;
            } else if ( it->timer > delta ) {
                if ( it->timer <= TEXT_FADE_TIME ) {
                    // fade out label
                    it->color.a = (GLfloat)(it->timer/TEXT_FADE_TIME);
                }
                it->timer -= delta;
            }
        }
        it++;
    }
}
void TextManager::RenderLabels( void ) {
    if ( !initialized ) return;
    blobsRendered = 0;
    /* Enable blending, necessary for our alpha texture */
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if ( shadersAvailable ) {
        // Bind our VAO and VBO
        glBindVertexArray(vertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    }
    for ( unsigned int i=0; i < textLabels.size(); i++ ) {
        TextBlob* label = &textLabels[i];
        RenderBlob( *label );
        blobsRendered++;
    }
    if ( shadersAvailable ) {
        glBindVertexArray(0);
    }
}
int TextManager::AddText( std::string text, glm::vec3 position, bool UISpace, int size,
                         FontName font, double timer, Color color, glm::vec3 rotation ) {
    TextBlob newBlob = {position, rotation, color, size, font, 0, timer, UISpace, text};
    return AddBlob(newBlob);
}
void TextManager::RemoveText( int blobID ) {
    for ( unsigned int i=0; i < textLabels.size(); i++ ) {
        if ( textLabels[i].blobID == blobID ) {
            textLabels.erase(textLabels.begin()+i);
            return;
        }
    }
}
void TextManager::UpdateText( unsigned int blobID, std::string newText ) {
    for ( unsigned int i=0; i < textLabels.size(); i++ ) {
        if ( textLabels[i].blobID == blobID ) {
            textLabels[i].text = newText;
            return;
        }
    }
}
void TextManager::UpdateTextColor( unsigned int blobID, Color newColor ) {
    for ( unsigned int i=0; i < textLabels.size(); i++ ) {
        if (textLabels[i].blobID == blobID) {
            textLabels[i].color = newColor;
            return;
        }
    }
}
void TextManager::UpdateTextPos( unsigned int blobID, glm::vec3 newPos ) {
    for ( unsigned int i=0; i < textLabels.size(); i++ ) {
        if (textLabels[i].blobID == blobID) {
            textLabels[i].pos = newPos;
            return;
        }
    }
}
void TextManager::UpdateTextRot( unsigned int blobID, glm::vec3 newRot ) {
    for ( unsigned int i=0; i < textLabels.size(); i++ ) {
        if (textLabels[i].blobID == blobID) {
            textLabels[i].rot = newRot;
            return;
        }
    }
}
void TextManager::UpdateTextTimer( unsigned int blobID, double newTimer ) {
    for ( unsigned int i=0; i < textLabels.size(); i++ ) {
        if (textLabels[i].blobID == blobID) {
            textLabels[i].timer = newTimer;
            return;
        }
    }
}
void TextManager::GetTextSize( unsigned int blobID, float &width, float &height ) {
    for ( unsigned int i=0; i < textLabels.size(); i++ ) {
        if ( textLabels[i].blobID == blobID ) {
            std::string fontFile = GetFontFileName( textLabels[i].font );
            FontAtlas* a = GetAtlas(fontFile, textLabels[i].size);
            GlyphInfo* g = a->GetGlyphInfo();
            const uint8_t *p;
            /* Loop through all characters */
            for (p = (const uint8_t *)textLabels[i].text.c_str(); *p; p++) {
                width += g[*p].ax;
                float h = g[*p].bh;
                if ( height < h ) height = h;
            }
        }
    }
}
std::string TextManager::GetFontFileName( FontName theFont ) {
    std::string fontFile;
    if ( theFont == FONT_DEFAULT ) {
        fontFile = ("ClearSans-Regular.ttf");
    } else if ( theFont == FONT_MENU ) {
        fontFile = ("HiLoDeco.ttf");
    } else if ( theFont == FONT_PIXEL ) {
        fontFile = ("ORANGEKI.ttf");
    } else if ( theFont == FONT_FANCY ) {
        fontFile = ("CRETINO.ttf");
    } else if ( theFont == FONT_JURA ) {
        fontFile = ("Jura-Book.ttf");
    } else if ( theFont == FONT_SEGMENT ) {
        fontFile = ("Segment14.otf");
    } else if ( theFont == FONT_FELL_NORMAL ) {
        fontFile = ("IMFeENrm29P.ttf");
    } else if ( theFont == FONT_FELL_CAPS ) {
        fontFile = ("IMFeNsc29P.ttf");
    } else if ( theFont == FONT_BENTHAM ) {
        fontFile = ("Bentham.ttf");
    } else {
        fontFile = "NO_FONT";
    }
    return fontFile;
}





/**
 Sees if the font atlas at that size was generated. If not, generates it.
 Should always return a valid font atlas if filename is legit and size isn't whack
 */
FontAtlas* TextManager::GetAtlas( std::string filename, int size ) {
    char buf[256];
#ifdef _WIN32
    sprintf_s(buf, "%s%i", filename.c_str(), size);
#else
	sprintf(buf, "%s%i", filename.c_str(), size);
#endif
	std::string fontKey = std::string(buf);
    
    FontAtlas* fAtlas = NULL;
    AtlasIter result = atlases.find(fontKey);
    if( result != atlases.end() ) {
        return result->second;
    }
    
    std::string path = FileUtil::GetPath();
    path.append("Data/Fonts/");
    path.append(filename);
    FT_Face face = NULL;
    // Load font, also known as a Face in FreeType
    if( FT_New_Face(ft, path.c_str(), 0, &face) ) {
        fprintf(stderr, "Could not open font %s\n", path.c_str() );
        return NULL;
    }
    fAtlas = new FontAtlas(face, size, shadersAvailable);
    // Clean up face
    FT_Done_Face(face);
    // Store newly created atlas
    atlases[fontKey] = fAtlas;
    return fAtlas;
}

void TextManager::RenderBlob( TextBlob b ) {
    std::string fontFile = GetFontFileName(b.font);
    float width = 0.0f;
    float height = 0.0f;
    GetTextSize(b.blobID, width, height);
    if ( shadersAvailable ) {
        FontAtlas* fAtlas = GetAtlas(fontFile, b.size);
        glm::mat4 mvp;
        if ( b.isUIBlob ) {
            g_renderer->GetUIMatrix(mvp);
        } else {
            g_renderer->GetGameMatrix(mvp);
        }
        if (!b.isUIBlob) {
            float r_scale = 1.0f/(b.size*4.0f);
            width *= r_scale;
            height *= r_scale;
            mvp = glm::translate(mvp, glm::vec3(b.pos.x+width*0.5f, b.pos.y+height*0.5f, b.pos.z) );
            mvp = glm::rotate(mvp, b.rot.y, glm::vec3(0.0, 1.0, 0.0) );
            mvp = glm::rotate(mvp, b.rot.x, glm::vec3(1.0, 0.0, 0.0) );
            mvp = glm::rotate(mvp, b.rot.z, glm::vec3(0.0, 0.0, 1.0) );
            mvp = glm::scale(mvp, glm::vec3(r_scale));
        }
        textShader->Begin();
        // Pass MVP to shader
        textShader->setUniformM4fv("MVP", mvp);
        // Pass color to shader
        textShader->setUniform4fv("color", b.color);
        /* Use the texture containing the atlas */
        glBindTexture( GL_TEXTURE_2D, fAtlas->GetTextureID() );
        glBindBuffer( GL_ARRAY_BUFFER, vertexBufferID );
        Render(b, fAtlas);
        textShader->End();
    } else {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        FontAtlas* fAtlas = GetAtlas(fontFile, b.size);
        /* Use the texture containing the atlas */
        glEnable(GL_TEXTURE_2D);
        glBindTexture( GL_TEXTURE_2D, fAtlas->GetTextureID() );
        // Render
        Render(b, fAtlas);
        // Set GL state back
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glDisable(GL_TEXTURE_2D);
    }
}
/**
 * Render text using the currently loaded font and currently set font size.
 */
void TextManager::Render(TextBlob& b, FontAtlas * a) {
	const uint8_t *p;
	unsigned long length = b.text.length();
	glm::vec4* coords = new glm::vec4[6 * length];
    glm::vec2* tCoords = new glm::vec2[6 * length];
    int c = 0;
    GlyphInfo* g = a->GetGlyphInfo();
    float x = b.pos.x;
    float y = b.pos.y;
    float z = b.pos.z;
    /* Loop through all characters */
    for (p = (const uint8_t *)b.text.c_str(); *p; p++) {
        // TODO:: NEWLINE CHARACTER USAGE
        /*if ( strncmp((const char*)p, "\n", 2) == 0) {
            printf("[TEXT RENDERER] WANTS NEW LINE SWAP\n");
            y += g[*p].bh;
            p++;
            continue;
        }*/
        /* Calculate the vertex and texture coordinates */
        float x2 = x + g[*p].bl;
        float y2 = -y - g[*p].bt;
        float w = g[*p].bw;
        float h = g[*p].bh;
        
        /* Advance the cursor to the start of the next character */
        x += g[*p].ax;
        y += g[*p].ay;
        
        /* Skip glyphs that have no pixels */
        if (!w || !h)
            continue;
        int aw = a->GetWidth();
        int ah = a->GetHeight();
        
        tCoords[c] = glm::vec2(g[*p].tx + g[*p].bw / aw, g[*p].ty);
		coords[c++] = glm::vec4( x2+w, -y2, z, 1.0f );
		tCoords[c] = glm::vec2(g[*p].tx, g[*p].ty);
		coords[c++] = glm::vec4( x2, -y2, z, 1.0f );
        tCoords[c] = glm::vec2(g[*p].tx, g[*p].ty + g[*p].bh / ah);
		coords[c++] = glm::vec4( x2, -y2-h, z, 1.0f );
        tCoords[c] = glm::vec2(g[*p].tx + g[*p].bw / aw, g[*p].ty);
		coords[c++] = glm::vec4( x2+w, -y2, z, 1.0f );
        tCoords[c] = glm::vec2(g[*p].tx, g[*p].ty + g[*p].bh / ah);
		coords[c++] = glm::vec4( x2, -y2-h, z, 1.0f );
        tCoords[c] = glm::vec2(g[*p].tx + g[*p].bw / aw, g[*p].ty + g[*p].bh / ah);
		coords[c++] = glm::vec4( x2+w, -y2-h, z, 1.0f );
    }
    if ( shadersAvailable ) {
        glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(glm::vec4)*6*length) );
        /* Draw all the characters on the screen in one go */
        glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::vec4)*6*length)+sizeof(glm::vec2)*6*length, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof(glm::vec4) * 6 * length), coords);
        glBufferSubData(GL_ARRAY_BUFFER, (sizeof(glm::vec4)*6*length), sizeof(glm::vec2)*6*length, tCoords);
        glDrawArrays(GL_TRIANGLES, 0, c);
    } else {
        glPushMatrix(); {
            
            glColor4f(b.color.r, b.color.g, b.color.b, b.color.a);
            //            glTranslatef(b.pos.x, b.pos.y, -b.pos.z);
            glRotatef(b.rot.x, 1.0f, 0.0f, 0.0f);
            glRotatef(b.rot.y, 0.0f, 1.0f, 0.0f);
            glRotatef(b.rot.z, 0.0f, 0.0f, 1.0f);
            if ( !b.isUIBlob ) {
                glEnable(GL_DEPTH_TEST);
                float r_scale = 1.0f/b.size;
                glScalef(r_scale, r_scale, r_scale);
            }
            
            glVertexPointer(4, GL_FLOAT, 0, coords);
            glTexCoordPointer(2, GL_FLOAT, 0, tCoords);
            
            glDrawArrays(GL_TRIANGLES, 0, c);
            
        } glPopMatrix();
    }
	delete[] coords;
	delete[] tCoords;
}

    // Adds a new label and returns its index ID
int TextManager::AddBlob( TextBlob& newBlob ) {
    newBlob.blobID = _nextId++;
    textLabels.push_back(newBlob);
    return newBlob.blobID;
}
