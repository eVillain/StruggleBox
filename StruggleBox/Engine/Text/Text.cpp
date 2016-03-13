#include "Text.h"
#include "TextDefaults.h"
#include "Label.h"
#include "Locator.h"
#include "Renderer.h"
#include "Shader.h"
#include "Log.h"
#include <string>
#include <glm/gtc/matrix_transform.hpp>     // glm::translate, glm::rotate, glm::scale

Text::Text() :
_renderer(nullptr),
_initialized(false),
_atlasFactory(nullptr),
_textShader(nullptr),
_textShaderDeferred(nullptr)
{
    Log::Debug("[Text] Constructor...");
}

Text::~Text()
{
    Log::Debug("[Text] Destructor...");
}

bool Text::Initialize(Locator& locator)
{
    Log::Debug("[Text] Initializing...");
    if (_initialized) {
        Log::Error("[Text] can't initialize, already initialized!");
        return false;
    }
    _renderer = locator.Get<Renderer>();
    
    _atlasFactory = std::unique_ptr<FontAtlasPool>(new FontAtlasPool());
    _atlasFactory->Initialize();
    
    _textShader = std::make_unique<Shader>();
    _textShader->InitFromSource(text_vertex_shader_forward,
                                text_frag_shader_forward);
    _textShaderDeferred = std::make_unique<Shader>();
    _textShaderDeferred->InitFromSource(text_vertex_shader_deferred,
                                        text_frag_shader_deferred);
    
    if (_textShader->GetProgram() == 0 ||
        _textShaderDeferred->GetProgram() == 0)
    {
        Log::Error("[Text] failed to load text shader program!");
        return false;
    }
    
    _initialized = true;
    return true;
}

bool Text::Terminate()
{
    if (!_initialized) return false;
    _initialized = false;
    return true;
}

void Text::Draw()
{
    glm::mat4 uiMVP;
    _renderer->GetUIMatrix(uiMVP);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _textShader->Begin();
    for (auto labelPair : _labels2D)
    {
        std::shared_ptr<Label> label = labelPair.first;
        if (!label->isVisible()) continue;
        
        const std::string fontFile = Fonts::GetFileName(label->getFont());
        auto atlas =_atlasFactory->GetAtlas(fontFile,
                                            label->getFontSize());

        glm::vec2 labelSize = label->getSize();
        glm::vec3 labelOffset;
        switch (label->getAlignment())
        {
            case Align_Left:
                labelOffset = glm::vec3(-labelSize.x, -labelSize.y*0.5f, 0.0f);
                break;
            case Align_Right:
                labelOffset = glm::vec3(0.0f, -labelSize.y*0.5f, 0.0f);
                break;
            case Align_Center:
                labelOffset = glm::vec3(-labelSize*0.5f, 0.0f);
            default:
                break;
        }
        
        glm::mat4 labelMVP = label->getTransform().GetMatrix();
        labelMVP = glm::translate(labelMVP, labelOffset);

        TextVertBuffer* buffer = labelPair.second;

        if (label->isDirty())
        {
            label->_size = CalculateTextSize(label->getText(),
                                             label->getFont(),
                                             label->getFontSize());
            buffer->Buffer(label->getText(),
                           *atlas.get(),
                           label->getFontSize());
            label->setNotDirty();
        }

        glBindTexture(GL_TEXTURE_2D, atlas->GetTextureID());
        _textShader->setUniformM4fv("MVP", uiMVP*labelMVP);
        _textShader->setUniform4fv("color", label->getColor());
        buffer->Draw();
    }
}

std::shared_ptr<Label> Text::CreateLabel(const std::string text)
{
    std::shared_ptr<Label> label = std::shared_ptr<Label>(new Label());
    label->setText(text);
    _labels2D[label] = new TextVertBuffer();
    return label;
}

void Text::DestroyLabel(std::shared_ptr<Label> label)
{
    if (_labels2D.find(label) != _labels2D.end()) {
        delete _labels2D[label];
        _labels2D.erase(label);
    } else if (_labels3D.find(label) != _labels3D.end()) {
        delete _labels3D[label];
        _labels3D.erase(label);
    }
}

glm::vec2 Text::CalculateTextSize(const std::string& text,
                                  Fonts::FontID font,
                                  const int fontSize)
{
    std::string fontName = Fonts::GetFileName(font);
    std::shared_ptr<FontAtlas> atlas = _atlasFactory->GetAtlas(fontName,
                                                               fontSize);
    
    const GlyphInfo* g = atlas->GetGlyphInfo();
    
    glm::vec2 returnSize;
    
    // Pre-calculate sizes and positions we will need
    float height = 0;   // Holds total height of all lines
    float width = 0;    // Holds width of current line
    float widthMax = 0; // Holds width of widest line
    const uint8_t *p;
    
    // Loop through all characters
    for (p = (const uint8_t *)text.c_str(); *p; p++)
    {
        // Skip newline character
        if (strncmp((const char*)p, "\n", 1) == 0)
        {
            // Size calculation
            height += fontSize;
            if (width > widthMax) { widthMax = width; }
            width = 0;
            
            continue;
        }
        
        float h = g[*p].bh;
        
        // Size calculation
        width += g[*p].ax;
        if (height < h) height = h;
        
        // Skip glyphs that have no pixels
        if (!g[*p].bw || !h)
        {
            continue;
        }
    }
    
    /* Final size calculation */
    if ( width > widthMax ) { widthMax = width; }
    returnSize = glm::vec2(widthMax, height);
    
    return returnSize;
}