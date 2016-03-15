#include "TextVertBuffer.h"
#include "FontAtlas.h"
#include "Log.h"

typedef struct {
    GLfloat x,y,z,w;
    GLfloat u,v;
} TextVertexData;

TextVertBuffer::TextVertBuffer() :
_vao(0),
_vbo(0),
_count(0),
_size(0)
{
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          sizeof(TextVertexData),
                          0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(TextVertexData),
                          (GLvoid*)(4*sizeof(GLfloat)));
    glBindVertexArray(0);
    Log::Debug("[TextVertBuffer] generated");
}

void TextVertBuffer::Buffer(const std::string &text,
                            const FontAtlas &atlas,
                            const int fontSize)
{
    const GlyphInfo* g = atlas.GetGlyphInfo();
    
    // Cursor coordinates
    float x = 0;
    float y = 0;
    float z = 0;
    
    // Temporary storage for one quad
    TextVertexData* verts = new TextVertexData[text.length()*6];
    
    _count = 0;
    int aw = atlas.GetWidth();
    int ah = atlas.GetHeight();

    // Loop through all characters
    for (const char* p = text.c_str(); *p; p++)
    {
        // Handle newline character
        if (strncmp(p, "\n", 1) == 0)
        {
            // Move cursor to beginning(left) and down one row
            x = 0;
            y -= fontSize;
            continue;
        }
        
        // Calculate the vertex and texture coordinates
        float x2 = x + g[*p].bl;
        float y2 = -y - g[*p].bt;
        float w = g[*p].bw;
        float h = g[*p].bh;
        
        // Advance the cursor to the start of the next character
        x += g[*p].ax;
        y += g[*p].ay;
        
        // Skip glyphs that have no pixels
        if (!w || !h)
        { continue; }
        
        // Pack coords into buffer
        verts[_count++] = {
            x2+w, -y2, z, 1.0f,
            g[*p].tx + g[*p].bw / aw, g[*p].ty
        };
        verts[_count++] = {
            x2, -y2, z, 1.0f,
            g[*p].tx, g[*p].ty
        };
        verts[_count++] = {
            x2, -y2-h, z, 1.0f,
            g[*p].tx, g[*p].ty + g[*p].bh / ah
        };
        verts[_count++] = {
            x2+w, -y2, z, 1.0f,
            g[*p].tx + g[*p].bw / aw, g[*p].ty
        };
        verts[_count++] = {
            x2, -y2-h, z, 1.0f,
            g[*p].tx, g[*p].ty + g[*p].bh / ah
        };
        verts[_count++] = {
            x2+w, -y2-h, z, 1.0f,
            g[*p].tx + g[*p].bw / aw, g[*p].ty + g[*p].bh / ah
        };
    }
    Log::Debug("[TextVertBuffer] buffering %i verts (%i bytes) to GPU",
              _count, sizeof(TextVertexData)*_count);

    // Upload verts
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(TextVertexData)*_count,
                 verts,
                 GL_STATIC_DRAW);
}

void TextVertBuffer::Draw()
{
    if (_count == 0) return;
    
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glDrawArrays(GL_TRIANGLES, 0, _count);
}
