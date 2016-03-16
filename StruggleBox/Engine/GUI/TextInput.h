#ifndef GUI_TEXTINPUT_H
#define GUI_TEXTINPUT_H

#include "Widget.h"
#include "TextInputBehavior.h"
#include "InputListener.h"
#include <SDL2/SDL_events.h>

class Label;
class TextInput:
public Widget,
public InputEventListener,
public TextInputEventListener
{
    friend class GUI;
public:
    ~TextInput();

    // Text input operations
    void StartTextInput();
    void StopTextInput();
    void ClearText();
    
    // Attach a behavior to pass the input text somewhere
    void SetBehavior( TextInputBehavior* behavior ) { _behavior = behavior; }
    void setDefaultText(const std::string text);
    
    std::shared_ptr<Label> getLabel() { return _label; }
protected:
    TextInput(Locator& locator);

    void Draw(Renderer* renderer);
    void Update(const double deltaTime);
    void setFocus(const bool focus);
    void setActive(const bool active);
    void setVisibility(const bool visible);
    
    void OnInteract(const bool interact,
                    const glm::ivec2& coord);
private:
    TextInputBehavior* _behavior;
    // Regular events, we need them to accept/cancel text input
    bool OnEvent(const std::string& event,
                 const float& amount);
    // Text input parameters
    void OnTextInput(const std::string& text);
    bool _textInputActive;
    
    // Text input attributes
    std::shared_ptr<Label> _label;
    std::string _inputText;
    std::string _defaultText;
    
    double _lastCursorBlink;
    bool _cursorBlink;
};

#endif
