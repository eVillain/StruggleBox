#pragma once

//  The boolean return value dictates whether the event was swallowed
//  False means the event should propagate to other listeners

#include "InputConstants.h"
#include <string>
#include "glm/glm.hpp"

class InputEventListener
{
    friend class Input;
private:
    virtual bool OnEvent(const InputEvent event, const float amount) = 0;
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
