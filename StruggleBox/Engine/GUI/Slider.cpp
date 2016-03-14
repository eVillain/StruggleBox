#include "Slider.h"
#include "Locator.h"
#include "Text.h"
#include "Renderer.h"
#include "MathUtils.h"
#include "StringUtil.h"

Slider::Slider(Locator& locator) :
Widget(locator)
{
    _behavior = nullptr;
    _sliderValue = 0.5;
    _sliderWidth = 10;
    _sliderPadding = 6;
    _draggingSlider = false;
    _name = "Slider";
    _label = _locator.Get<Text>()->CreateLabel("Slider");
}

Slider::~Slider()
{
    if (_behavior)
    {
        delete _behavior;
        _behavior = nullptr;
    }
    if (_label)
    {
        _locator.Get<Text>()->DestroyLabel(_label);
        _label = nullptr;
    }
}

void Slider::Draw(Renderer* renderer)
{
    if ( !_visible ) return;
    Widget::Draw(renderer);
    
    glm::ivec2 drawPos = glm::ivec2(_transform.GetPosition().x-(_size.x*0.5),
                                    _transform.GetPosition().y-(_size.y*0.5));
    
    // Slider
    if (_behavior)
    {
        const int sliderMaxLeft = drawPos.x + _sliderPadding;
        const int sliderMaxRight = drawPos.x + (_size.x - _sliderPadding);
        const int sliderLength = (sliderMaxRight-sliderMaxLeft);
        const int sliderOffset = _sliderValue*sliderLength;
        
        const int widgetMiddleY = drawPos.y+(_size.y/2);
        const int sliderMiddle = sliderMaxLeft+sliderOffset;
        
        // Slider ruler
        renderer->Buffer2DLine(glm::vec2(sliderMaxLeft, widgetMiddleY),
                               glm::vec2(sliderMaxRight, widgetMiddleY),
                               COLOR_BLACK,
                               COLOR_BLACK,
                               _transform.GetPosition().z+1);
        
        // Slider pointer
        float hF = _sliderWidth/2.0f;
        float pad = _sliderPadding/2.0f;
        glm::vec2 vertices[] = {
            glm::vec2(sliderMiddle-hF, drawPos.y+pad+hF),
            glm::vec2(sliderMiddle   , drawPos.y+pad),
            glm::vec2(sliderMiddle+hF, drawPos.y+pad+hF),
            glm::vec2(sliderMiddle+hF, drawPos.y+_size.y-pad),
            glm::vec2(sliderMiddle-hF, drawPos.y+_size.y-pad),
        };
        renderer->DrawPolygon(5, vertices, COLOR_BLACK, COLOR_GREY);
    }
    
    // Label
    if (_label) {
        _label->getTransform().SetPosition(_transform.GetPosition());
        if (_focus) {
            _label->setColor(COLOR_UI_TEXT_HIGHLIGHT);
        } else {
            _label->setColor(COLOR_UI_TEXT);
        }
    }
}

// When clicked/pressed
void Slider::OnInteract(const bool interact,
                        const glm::ivec2& coord)
{
    if (interact) CheckSliderPress(coord);
    else _draggingSlider = false;
}

void Slider::OnDrag(const glm::ivec2& coord)
{
    CheckSliderPress(coord);
}

void Slider::setVisibility(const bool visible)
{
    _visible = visible;
    if (_label) {
        _label->setVisible(_visible);
    }
}

void Slider::setLabel(const std::string text)
{
    _name = text;
    _label->setText(_name);
}

void Slider::CheckSliderPress(const glm::ivec2 &coord)
{
    if (_focus)
    {
        glm::ivec2 drawPos = glm::ivec2(_transform.GetPosition().x-(_size.x*0.5), _transform.GetPosition().y-(_size.y*0.5));
        // Check if cursor in slider area
        const int sliderMaxLeft = drawPos.x + _sliderPadding;
        const int sliderMaxRight = drawPos.x + (_size.x - _sliderPadding);
        const int sliderLength = (sliderMaxRight-sliderMaxLeft);
        const int sliderOffset = _sliderValue*sliderLength;
        
        const int sliderMiddle = sliderMaxLeft+sliderOffset;
        const int sliderLeft = sliderMiddle-(_sliderWidth/2);
        const int sliderRight = sliderMiddle+(_sliderWidth/2);
        const int sliderBottom = drawPos.y + _sliderPadding;
        const int sliderTop = drawPos.y + (_size.y - _sliderPadding);
        
        if (coord.x > sliderLeft &&
            coord.x < sliderRight &&
            coord.y > sliderBottom &&
            coord.y < sliderTop)
        {
            _draggingSlider = true;
            
        }
        
        if (_draggingSlider)
        {
            int newSliderOffset = coord.x - sliderMaxLeft;
            float newValue = (float)newSliderOffset / (float)sliderLength;
            newValue = MathUtils::Clamp(newValue, 0.0f, 1.0f);
            
            if (_sliderValue != newValue)
            {
                _sliderValue = newValue;
                if ( _behavior ) {
                    _behavior->SetValue(_sliderValue);
                    _label->setText(_name + ": " + _behavior->GetValueString());
                }
                else {
                    _label->setText(_name + "(empty): " + StringUtil::DoubleToString(_sliderValue));
                }
            }
        }
    }
}
