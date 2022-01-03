#include "LabelNode.h"

#include "GUI.h"
#include "Renderer.h"
#include "TextAtlas.h"

LabelNode::LabelNode(const GUI& gui, const std::string& text, const std::string& font, const uint8_t fontHeight)
	: m_gui(gui)
    , m_text(text)
	, m_font(font)
	, m_fontHeight(fontHeight)
{
    refreshContentSize();
}

void LabelNode::draw(Renderer& renderer, const glm::vec3& parentPosition, const glm::vec2& parentScale)
{
    if (m_text.empty())
    {
        Node::draw(renderer, parentPosition, parentScale);
        return;
    }
    TextAtlasID atlasID = renderer.getTextAtlasID(m_font, m_fontHeight);
    TextAtlas* atlas = renderer.getTextAtlasByID(atlasID);
	const Glyph* g = atlas->getGlyphs();
    
    const Rect2D rect = getBoundingBox();

    // Glyph coordinates - using ints here to round off positions to pixel-perfect values
    int x = parentPosition.x + rect.x;
    int y = parentPosition.y + rect.y;
    float z = parentPosition.z + m_position.z;
    
    size_t visible_glyphs = 0;
    for (const char* p = m_text.c_str(); *p; p++)
    {
        if (strncmp(p, "\n", 1) == 0)
        {
            continue;
        }
        const float w = g[*p].bw;
        const float h = g[*p].bh;
        if (!w || !h)
        {
            continue;
        }
        visible_glyphs++;
    }

    TexturedVertexData* verts = renderer.queueTextVerts(visible_glyphs * 6, atlas->getTextureID());

    uint32_t _count = 0;
    float aw = atlas->getWidth();
    float ah = atlas->getHeight();

    // Loop through all characters
    for (const char* p = m_text.c_str(); *p; p++)
    {
        // Handle newline character
        if (strncmp(p, "\n", 1) == 0)
        {
            // Move cursor to beginning(left) and down one row
            x = parentPosition.x + m_position.x;
            y -= m_fontHeight;
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
            glm::vec3(x2+w, -y2, z),
            glm::vec2(g[*p].tx + g[*p].bw / aw, g[*p].ty)
        };
        verts[_count++] = {
            glm::vec3(x2, -y2, z),
            glm::vec2(g[*p].tx, g[*p].ty)
        };
        verts[_count++] = {
            glm::vec3(x2, -y2-h, z),
            glm::vec2(g[*p].tx, g[*p].ty + g[*p].bh / ah)
        };
        verts[_count++] = {
            glm::vec3(x2+w, -y2, z),
            glm::vec2(g[*p].tx + g[*p].bw / aw, g[*p].ty)
        };
        verts[_count++] = {
            glm::vec3(x2, -y2-h, z),
            glm::vec2(g[*p].tx, g[*p].ty + g[*p].bh / ah)
        };
        verts[_count++] = {
            glm::vec3(x2+w, -y2-h, z),
            glm::vec2(g[*p].tx + g[*p].bw / aw, g[*p].ty + g[*p].bh / ah)
        };
    }

    Node::draw(renderer, parentPosition, parentScale);
}

void LabelNode::setText(const std::string& text)
{
    m_text = text;
    refreshContentSize();
}

void LabelNode::setFont(const std::string& font) 
{
    m_font = font;
    refreshContentSize();
}

void LabelNode::setFontHeight(const uint8_t fontHeight)
{
    m_fontHeight = fontHeight;
    refreshContentSize();
}

void LabelNode::refreshContentSize()
{
    m_contentSize = m_gui.calculateTextSize(m_text, m_font, m_fontHeight);
}
