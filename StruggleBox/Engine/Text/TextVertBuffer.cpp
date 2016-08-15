#include "TextVertBuffer.h"
#include "FontAtlas.h"
#include "VertBuffer.h"
#include "Renderer.h"
#include "Log.h"

TextVertBuffer::TextVertBuffer(std::shared_ptr<Renderer> renderer) :
	_vertBuffer(nullptr),
_count(0),
_size(0)
{
	Log::Debug("[TextVertBuffer] Constructor, instance at: %p", this);
	_vertBuffer = renderer->addVertBuffer(TexturedVerts);
}

void TextVertBuffer::bind()
{
	_vertBuffer->bind();
}

void TextVertBuffer::buffer(const std::string &text,
                            const FontAtlas &atlas,
                            const int fontSize)
{
    const GlyphInfo* g = atlas.GetGlyphInfo();
    
    // Cursor coordinates
    float x = 0;
    float y = 0;
    float z = 0;
    
    // Temporary storage for one quad
    TexturedVertexData* verts = new TexturedVertexData[text.length()*6];
    
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
    //Log::Debug("[TextVertBuffer] buffering %i verts (%i bytes) to GPU",
    //          _count, sizeof(TexturedVertexData)*_count);

    // Upload verts
	_vertBuffer->upload(verts, sizeof(TexturedVertexData)*_count, false);
	delete[] verts;
}

void TextVertBuffer::draw()
{
    if (_count == 0)
		return;

	_vertBuffer->draw(GL_TRIANGLES, _count);
}
