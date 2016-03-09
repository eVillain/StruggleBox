#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "InputListener.h"
#include "InputConstants.h"
#include "glm/glm.hpp"
#include <SDL2/SDL_events.h>
#include <map>
#include <vector>

class Locator;

///  Takes keyboard/mouse/joystick events from SDL and passes them to
///  the currently active listener
class Input
{
public:
    Input(Locator& locator);
    
    bool Initialize();
    bool Terminate();
    void Update(const double deltaTime);
    
    void RegisterEventObserver(InputEventListener* observer);
    void UnRegisterEventObserver(InputEventListener* observer);
    
    void RegisterMouseObserver(MouseEventListener* observer);
    void UnRegisterMouseObserver(MouseEventListener* observer);
        
    // Keyboard text input
    void StartTextInput(TextInputEventListener* observer);
    void StopTextInput(TextInputEventListener* observer);
    
    /// Bind an input (Keyboard/Mouse/Joystick) to an Event
    void Bind(std::string input,
              std::string event);
    
    void ProcessInput();
    
    void SetDefaultBindings();
    
    void MoveCursor(const glm::ivec2 coord);

private:
    Locator& _locator;
    
    bool ProcessTextInput(const SDL_Event& event);
    // Map of inputs to events
    std::map<std::string, std::string> _inputBindings;
    // Observers of input events
    
    std::vector<InputEventListener*> _inputEventListeners;
    std::vector<MouseEventListener*> _mouseEventListeners;
    // Text input observer
    TextInputEventListener* _textInputListener;
    // Current text input
    std::string _inputText;
};

#endif
