#pragma once

#include "Node.h"
#include <string>

class Renderer;
class GUI;

class LabelNode : public Node
{
public:
    LabelNode(const GUI& gui, const std::string& text, const std::string& font, const uint8_t fontHeight);

    void draw(Renderer& renderer, const glm::vec3& parentPosition, const glm::vec2& parentScale) override;

    void setText(const std::string& text);
    void setFont(const std::string& font);
    void setFontHeight(const uint8_t fontHeight);
    
    const std::string& getText() const { return m_text; };
    const std::string& getFont() const { return m_font; };
    const uint8_t getFontHeight() const { return m_fontHeight; };
    
private:
    const GUI& m_gui;
    std::string m_text;
    std::string m_font;
    uint8_t m_fontHeight;

    void refreshContentSize();
};
