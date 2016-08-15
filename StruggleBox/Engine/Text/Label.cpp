#include "Label.h"
#include "Renderer.h"

Label::Label() :
_text(""),
_color(COLOR_WHITE),
_font(Fonts::FONT_DEFAULT),
_fontSize(DEFAULT_TEXT_SIZE),
_dirty(true),
_visible(true),
_alignment(Align_Center)
{
}

Label::~Label()
{}

void Label::setText(const std::string& newText)
{
    _text = newText;
    _dirty = true;
}

void Label::setFont(const Fonts::FontID newFont)
{
    _font = newFont;
    _dirty = true;
}

void Label::setFontSize(const unsigned int newFontSize)
{
    _fontSize = newFontSize;
    _dirty = true;
}

void Label::setColor(const Color& color)
{
    _color = color;
}

void Label::setAlignment(const TextAlignment& alignment)
{
    _alignment = alignment;
}

const glm::vec2& Label::getSize() const
{
    return _size;
}

