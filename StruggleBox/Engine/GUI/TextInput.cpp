#include "TextInput.h"
#include "Locator.h"
#include "Renderer.h"
#include "Input.h"
#include "Timer.h"
#include "Text.h"
#include <SDL2/SDL.h>

#define TEXTSIZE 16
#define CURSOR_BLINK_RATE 0.5

TextInput::TextInput(Locator& locator) :
Widget(locator)
{
    _textInputActive = false;
    _behavior = nullptr;
    _label = _locator.Get<Text>()->CreateLabel("");
    _lastCursorBlink = 0;
}

TextInput::~TextInput()
{
    _locator.Get<Text>()->DestroyLabel(_label);
}

void TextInput::OnTextInput( const std::string& inputText )
{
    _inputText = inputText;
    _label->setText(_inputText);
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

void TextInput::Draw(Renderer* renderer)
{
    if ( !_visible ) return;
    
    Widget::Draw(renderer);
    
    glm::vec2 textSize = _label->getSize();
    float leftEdge = _transform.GetPosition().x - _size.x*0.5;
    float cursorX = leftEdge + 8 + textSize.x + 4;
    unsigned int halfFontHeight = _label->getFontSize() / 2;
    float labelY = _transform.GetPosition().y;
    _label->getTransform().SetPositionX(leftEdge + 8);
    _label->getTransform().SetPositionY(labelY);

    _label->setAlignment(Align_Left);
    
    // Render blinking cursor
    if ( _textInputActive ) {
        if (_cursorBlink)
        {
            // Draw cursor
            renderer->Buffer2DLine(glm::vec2(cursorX, _transform.GetPosition().y + halfFontHeight),
                                   glm::vec2(cursorX, _transform.GetPosition().y - halfFontHeight),
                                   COLOR_WHITE,
                                   COLOR_WHITE,
                                   _transform.GetPosition().z-1);
        }
    }
    renderer->Render2DLines();
}

const void TextInput::Update(const double deltaTime)
{
    double timeNow = Timer::Seconds();
    double cursorBlinkDelta = timeNow - _lastCursorBlink;
    if (cursorBlinkDelta > CURSOR_BLINK_RATE)
    {
        _cursorBlink = !_cursorBlink;
        _lastCursorBlink = timeNow;
    }
}

void TextInput::SetFocus(const bool focus)
{
    _focus = focus;
}

void TextInput::SetActive(const bool active)
{
    if ( active != _textInputActive ) {
        if ( _active && _textInputActive ) // Widget made inactive with input active
        {
            StopTextInput();
        }
    }
    _active = active;
}

void TextInput::SetVisible(const bool visible)
{
    if (_visible &&
        !visible &&
        _textInputActive)
    {
        StopTextInput();
    }
    
    _label->setVisible(visible);
    _visible = visible;
}

void TextInput::setDefaultText(const std::string text)
{
    _defaultText = text;
    if (_inputText.length() == 0)
    {
        _label->setText(_defaultText);
        _label->setColor(COLOR_UI_TEXT_INACTIVE);
    }
}

void TextInput::OnInteract( const bool interact, const glm::ivec2& coord )
{
    if (!_focus) return;
    if (!interact && !_textInputActive)
    {
        StartTextInput();
    }
}

void TextInput::StartTextInput()
{
    if (_textInputActive) return;
    _locator.Get<Input>()->StartTextInput(this);
    _locator.Get<Input>()->RegisterEventObserver(this);
    
    _textInputActive = true;
    if (_inputText.length() == 0)
    {
        _label->setText("");
        _label->setColor(COLOR_UI_TEXT_ACTIVE);
    }
}

void TextInput::StopTextInput()
{
    if (!_textInputActive) return;
    _textInputActive = false;
    _locator.Get<Input>()->StopTextInput(this);
    _locator.Get<Input>()->UnRegisterEventObserver(this);
}

void TextInput::ClearText()
{
    _inputText.clear();
    _label->setText("");
}
