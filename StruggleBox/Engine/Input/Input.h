#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "InputListener.h"
#include "InputConstants.h"
#include "glm/glm.hpp"
#include <SDL2/SDL_events.h>
#include <map>
#include <vector>
#include <memory>

class OSWindow;

///  Takes keyboard/mouse/joystick events from SDL and passes them to
///  the currently active listener
class Input
{
public:
    Input(std::shared_ptr<OSWindow> window);
    
    bool Initialize();
    bool Terminate();
    
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
    
    bool ProcessInput(const SDL_Event& event);
    
    void SetDefaultBindings();
    
    void MoveCursor(const glm::ivec2 coord);

private:    
	std::shared_ptr<OSWindow> _window;

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

#endif /* INPUT_H */
