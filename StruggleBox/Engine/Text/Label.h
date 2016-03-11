#ifndef LABEL_H
#define LABEL_H

#include "Transform.h"
#include "GFXDefines.h"
#include "Color.h"
#include "FontAtlas.h"
#include "TextConstants.h"
#include <string>

class Renderer;

class TextService;
static const unsigned int DEFAULT_TEXT_SIZE = 16;

class Label
{
    friend class Text;
public:
    ~Label();
    
    void setVisible(const bool visible) { _visible = visible; }
    void setText(const std::string& newText);
    void setFont(const Fonts::FontID newFont);
    void setFontSize(const unsigned int newFontSize);
    void setColor(const Color& color);
    void setAlignment(const TextAlignment& alignment);
    
    const bool isVisible() { return _visible; };
    Transform& getTransform() { return _transform; };
    const std::string& getText() const { return _text; };
    const glm::vec2& getSize() const;
    const Fonts::FontID& getFont() const { return _font; };
    const unsigned int getFontSize() const { return _fontSize; };
    const Color& getColor() const { return _color; };
    const TextAlignment& getAlignment() const { return _alignment; };
    
protected:
    Label();
    
    const bool isDirty() { return _dirty; }
    void setNotDirty() { _dirty = false; }
private:
    Transform _transform;
    bool _visible;
    
    // Label vars
    std::string _text;
    glm::vec2 _size;
    Fonts::FontID _font;
    unsigned int _fontSize;
    Color _color;
    TextAlignment _alignment;
    
    // Content has changed and needs to be re-buffered
    bool _dirty;
};


#endif /* LABEL_H */
