#ifndef INPUT_LISTENER_H
#define INPUT_LISTENER_H

//  The boolean return value dictates whether the event was swallowed
//  False means the event should propagate to other listeners

#include <string>
#include "glm/glm.hpp"

class InputEventListener
{
    friend class Input;
private:
    virtual bool OnEvent(const std::string& event,
                         const float& amount) = 0;
};

class MouseEventListener
{
    friend class Input;
private:
    virtual bool OnMouse(const glm::ivec2& coords) = 0;
};

class TextInputEventListener
{
    friend class Input;
private:
    virtual void OnTextInput(const std::string& text) = 0;
};

#endif /* INPUT_LISTENER_H */
