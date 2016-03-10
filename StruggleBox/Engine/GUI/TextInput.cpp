#include "TextInput.h"
#include "Locator.h"
#include "Renderer.h"
#include "Input.h"
#include "Timer.h"
#include <SDL2/SDL.h>

#define TEXTSIZE 16
#define CURSOR_BLINK_RATE 0.5

TextInput::TextInput()
{
    _textInputActive = false;
    _behavior = nullptr;
    
    _lastCursorBlink = 0;
}

TextInput::~TextInput()
{
    
}

void TextInput::OnTextInput( const std::string& inputText )
{
    _inputText = inputText;
    //        _label->SetText(_inputText);
}

bool TextInput::OnEvent( const std::string& theEvent, const float& amount )
{
    if ( theEvent == "back" && _textInputActive )
    {
        if ( amount == 1 )
        {
            StopTextInput();
        }
        return true;
    }
    else if ( theEvent == "start" && _textInputActive )
    {
        if ( amount == 1 )
        {
            StopTextInput();
            if ( _behavior )
            {
                _behavior->Trigger(_inputText);
            }
        }
        return true;
    }
    return false;
}

const void TextInput::Draw() const
{
    if ( !_visible ) return;
    //        Widget::Draw();
    //
    //        Primitives2D& primitives = *Locator::getRenderer().DrawPrimitives2D();
    
    //        glm::ivec2 drawPos = glm::ivec2(_position.x-(_size.x*0.5), _position.y-(_size.y*0.5));
    
    //        // Pixel perfect outer border (should render with 1px shaved off corners)
    //        primitives.Line(glm::vec2(drawPos.x,drawPos.y), glm::vec2(drawPos.x,drawPos.y+h), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER, z);  // L
    //        primitives.Line(glm::vec2(drawPos.x,drawPos.y+h), glm::vec2(drawPos.x+w,drawPos.y+h), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER, z);  // T
    //        primitives.Line(glm::vec2(drawPos.x+w+1,drawPos.y+h), glm::vec2(drawPos.x+w+1,drawPos.y), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER, z);  // R
    //        primitives.Line(glm::vec2(drawPos.x+w,drawPos.y-1), glm::vec2(drawPos.x,drawPos.y-1), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER, z);  // B
    //
    //        // Inner gradient fill
    //        Color gradColTop = COLOR_UI_GRADIENT_TOP;
    //        Color gradColBottom = COLOR_UI_GRADIENT_BOTTOM;
    //        if ( _focus)
    //        {
    //            if ( _textInputActive )
    //            {
    //                gradColTop *= 0.9;
    //                gradColBottom *= 0.9;
    //            }
    //            else
    //            {
    //                gradColTop *= 1.1;
    //                gradColBottom *= 1.1;
    //            }
    //        }
    //        primitives.RectangleGradientY(glm::vec2(_position.x,_position.y),
    //                                      glm::vec2(_size.x,_size.y),
    //                                      gradColTop,
    //                                      gradColBottom,
    //                                      _position.z);
    //
    //        // Inside border
    //        glEnable(GL_BLEND);
    //        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //        primitives.RectOutline(glm::vec2(_position.x,_position.y),
    //                               glm::vec2(_size.x-2,_size.y-2),
    //                               COLOR_UI_BORDER_INNER,
    //                               _position.z+1);
    //
    //        // Render blinking cursor
    //        if ( _textInputActive ) {
    //            if (_cursorBlink)
    //            {
    //                // Draw cursor
    //                glm::vec2 textSize = _label->GetSize();
    //                Primitives2D& primitives = *Locator::getRenderer().DrawPrimitives2D();
    //                float cursorX = _label->position.x+textSize.x + 4;
    //                primitives.Line(glm::vec2(cursorX, _position.y-8+(_size.y-TEXTSIZE)*0.6f),
    //                                glm::vec2(cursorX , _position.y-8+_size.y-(_size.y-TEXTSIZE)*0.6f),
    //                                COLOR_WHITE, COLOR_WHITE, _position.z);
    //            }
    //        }
}

const void TextInput::Update()
{
    double timeNow = Timer::Seconds();
    double cursorBlinkDelta = timeNow - _lastCursorBlink;
    if (cursorBlinkDelta > CURSOR_BLINK_RATE)
    {
        _cursorBlink = !_cursorBlink;
        _lastCursorBlink = timeNow;
    }
}

void TextInput::SetFocus( const bool focus)
{
    _focus = focus;
}

void TextInput::SetActive( const bool active )
{
    if ( active != _textInputActive ) {
        if ( _active && _textInputActive ) // Widget made inactive with input active
        {
            StopTextInput();
        }
    }
    _active = active;
}

void TextInput::SetVisible( const bool visible )
{
    if ( _visible != visible ) {
        if ( _visible && _textInputActive ) // Widget made invisible with input active
        {
            StopTextInput();
        }
    }
    _visible = visible;
}

// When clicked/pressed
void TextInput::OnInteract( const bool interact, const glm::ivec2& coord )
{
    if ( !_focus ) return;
    if ( !interact && !_textInputActive )
    {
        StartTextInput();
    }
}

void TextInput::StartTextInput()
{
    if ( _textInputActive ) return;
    //        Locator::getInput().StartTextInput(this);
    //        Locator::getInput().RegisterEventObserver(this);
    
    _textInputActive = true;
    if ( _inputText == _defaultText )
    {
        ClearText();
    }
}

void TextInput::StopTextInput()
{
    if ( !_textInputActive ) return;
    _textInputActive = false;
    //        Locator::getInput().StopTextInput(this);
    //        Locator::getInput().UnRegisterEventObserver(this);
}

void TextInput::ClearText()
{
    _inputText.clear();
    //        _label->SetText(_inputText);
}
