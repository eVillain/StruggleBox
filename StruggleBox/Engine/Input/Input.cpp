#include "Input.h"
#include "Console.h"
#include "CommandProcessor.h"
#include "StringUtil.h"
#include "RangeReverseAdapter.h"
#include "Log.h"
#include "AppContext.h"
#include "OSWindow.h"
#include <SDL2/SDL.h>

Input::Input(OSWindow& window) :
	_window(window),
	_inputText(""),
	_textInputListener(nullptr)
{
	Log::Info("[Input] constructor, instance at %p", this);
}

bool Input::Initialize()
{
    Log::Debug("[Input] Initializing...");
    // Add our binding mechanic to our console commands
    CommandProcessor::AddCommand("bind",
                                 Command<const std::string&, const int&>([=](
									 const std::string& input,
									 const int& event)
    {
        this->Bind(input, (InputEvent)event);
    } ));

    SetDefaultBindings();
    return true;
}

bool Input::Terminate()
{
    Log::Debug("[Input] Terminating...");
    // Clear up all bindings
    _inputBindings.clear();
    _inputEventListeners.clear();
    _textInputListener = nullptr;
    return true;
}

void Input::RegisterEventObserver(InputEventListener* observer)
{
    _inputEventListeners.push_back(observer);
}

void Input::UnRegisterEventObserver(InputEventListener* observer)
{
    std::vector<InputEventListener*>::iterator it = std::find(_inputEventListeners.begin(),
                                                              _inputEventListeners.end(),
                                                              observer);
    if ( it != _inputEventListeners.end() )
    {
        _inputEventListeners.erase(it);
    } else {
        Log::Warn("[Input] tried to unregister non-existent InputEventListener (%lu in stack)", _inputEventListeners.size());
    }
}

void Input::RegisterMouseObserver(MouseEventListener* observer)
{
    _mouseEventListeners.push_back(observer);
}

void Input::UnRegisterMouseObserver(MouseEventListener* observer)
{
    std::vector<MouseEventListener*>::iterator it = std::find(_mouseEventListeners.begin(),
                                                              _mouseEventListeners.end(),
                                                              observer);
    if ( it != _mouseEventListeners.end() )
    {
        _mouseEventListeners.erase(it);
    } else {
        Log::Warn("[Input] tried to unregister non-existent MouseEventListener (%lu in stack)", _mouseEventListeners.size());
    }
}

void Input::Bind(std::string input, InputEvent event)
{
    _inputBindings[input] = event;
}

/* Print modifier info */
void PrintModifiers( Uint32 mod )
{
    Log::Info( "Modifers: " );
    
    /* If there are none then say so and return */
    if( mod == KMOD_NONE ){
        Log::Info( "None\n" );
        return;
    }
    
    /* Check for the presence of each SDLMod value */
    /* This looks messy, but there really isn't    */
    /* a clearer way.                              */
    if( mod & KMOD_NUM ) Log::Info( "NUMLOCK " );
    if( mod & KMOD_CAPS ) Log::Info( "CAPSLOCK " );
    if( mod & KMOD_LCTRL ) Log::Info( "LCTRL " );
    if( mod & KMOD_RCTRL ) Log::Info( "RCTRL " );
    if( mod & KMOD_RSHIFT ) Log::Info( "RSHIFT " );
    if( mod & KMOD_LSHIFT ) Log::Info( "LSHIFT " );
    if( mod & KMOD_RALT ) Log::Info( "RALT " );
    if( mod & KMOD_LALT ) Log::Info( "LALT " );
    if( mod & KMOD_CTRL ) Log::Info( "CTRL " );
    if( mod & KMOD_SHIFT ) Log::Info( "SHIFT " );
    if( mod & KMOD_ALT ) Log::Info( "ALT " );
    if( mod & KMOD_LGUI ) Log::Info( "LGUI " );
    if( mod & KMOD_RGUI ) Log::Info( "RGUI " );
    if( mod & KMOD_MODE ) Log::Info( "MODE " );
    if( mod & KMOD_RESERVED ) Log::Info( "RESERVED " );
}

/* Print all information about a key event */
void PrintKeyInfo( SDL_KeyboardEvent *key )
{
    /* Is it a release or a press? */
    if (key->type == SDL_KEYUP)
    {
        Log::Info("Release:- ");
    }
    else
    {
        Log::Info("Press:- ");
    }
    
    /* Print the hardware scancode first */
    Log::Info( "Scancode: 0x%02X", key->keysym.scancode );
    /* Print the name of the key */
    Log::Info( ", Name: %s", SDL_GetKeyName( key->keysym.sym ) );
    Log::Info( "\n" );
    /* Print modifier info */
    PrintModifiers( key->keysym.mod );
}

bool Input::ProcessInput(const SDL_Event& event)
{
	// Amount variable depends on press/release or axis status
	// Range -1.0 ~ 1.0
	float amount = 0.0f;
	std::string input;

	// Check for text input, that takes precedence over regular events
	if (event.type == SDL_KEYDOWN)
	{
		if (event.key.repeat == 0)
		{
			amount = 1.0f;
			input = SDL_GetKeyName(event.key.keysym.sym);
#if 0
            Log::Debug("Input::ProcessInput key name: %s", input.c_str());
#endif
		}
	}
	else if (event.type == SDL_KEYUP)
	{
		if (event.key.repeat == 0)
		{
			amount = -1.0f;
			input = SDL_GetKeyName(event.key.keysym.sym);
		}
	}
	else if (event.type == SDL_MOUSEMOTION)
	{
		glm::ivec2 adjustedCoords = glm::ivec2(event.motion.x, event.motion.y);
		for (MouseEventListener* listener : _mouseEventListeners)
		{
			bool swallowed = (*listener).OnMouse(adjustedCoords);
			if (swallowed) // Event was swallowed, don't propagate
			{
				return true;
			}
		}
	}
	else if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		amount = 1.0f;
		input = "MouseButton" + StringUtil::IntToString(event.button.button);
	}
	else if (event.type == SDL_MOUSEBUTTONUP)
	{
		amount = -1.0f;
		input = "MouseButton" + StringUtil::IntToString(event.button.button);
	}
	else if (event.type == SDL_MOUSEWHEEL)
	{
		if (event.wheel.y != 0) {
			amount = (float)event.wheel.y;
			input = MOUSE_WHEEL_Y;
		}
		else if (event.wheel.x != 0) {
			amount = (float)event.wheel.x;
			input = MOUSE_WHEEL_X;
		}
	}

	// We have to check for text input events first, blocking the regular events
	if (_textInputListener && ProcessTextInput(event))
	{
		// Call the registered observer
		(*_textInputListener).OnTextInput(_inputText);
        return true;
	}
    else if (_textInputListener && input != "Escape" && input != "Return")
    {
        return false;
    }
	else if (input.length())   // Regular events
	{
		if (_inputBindings.find(input) != _inputBindings.end())
		{
			// Reversed because latest added listener should get the event first
			for (InputEventListener* listener : reverse_adapt_container(_inputEventListeners))
			{
				bool swallowed = (*listener).OnEvent(_inputBindings[input], amount);
				if (swallowed)
				{
					return true;	// Event was swallowed, don't propagate
				}
			}
		}
	}
	return false;
}

bool Input::ProcessTextInput(const SDL_Event &event)
{
    if( event.type == SDL_KEYDOWN )
    {
        //Handle backspace
        if( event.key.keysym.sym == SDLK_BACKSPACE && _inputText.length() > 0 )
        {
            //lop off character
            _inputText.pop_back();
            return true;
        }
        //Handle copy
        else if( event.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
        {
            SDL_SetClipboardText( _inputText.c_str() );
            return false;
        }
        //Handle paste
        else if( event.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
        {
            _inputText = SDL_GetClipboardText();
            return true;
        }
    }
    //Special text input events
    else if( event.type == SDL_TEXTINPUT )
    {
        //Not copy or pasting
        if( !( (event.text.text[ 0 ] == 'c' || event.text.text[ 0 ] == 'C' ) &&
              ( event.text.text[ 0 ] == 'v' || event.text.text[ 0 ] == 'V' ) && SDL_GetModState() & KMOD_CTRL ) )
        {
            //Append character
            _inputText += event.text.text;
            return true;
        }
    }
//    else if( event.type == SDL_TEXTEDITING )
//    {
//        return true;
//    }
    return false;
}

void Input::SetDefaultBindings()
{
    Bind(KEY_RETURN, InputEvent::Start);
    Bind(KEY_ESCAPE, InputEvent::Back);
    Bind(KEY_P, InputEvent::Pause);
    Bind(KEY_C, InputEvent::Console);
    Bind(KEY_TILDE, InputEvent::Stats);

    Bind(KEY_DELETE, InputEvent::Erase_Right);
    Bind(KEY_BACKSPACE, InputEvent::Erase_Left);
    Bind(KEY_UP, InputEvent::Look_Up);
    Bind(KEY_DOWN, InputEvent::Look_Down);
    Bind(KEY_LEFT, InputEvent::Look_Left);
    Bind(KEY_RIGHT, InputEvent::Look_Right);
    Bind(KEY_W, InputEvent::Move_Forward);
    Bind(KEY_S, InputEvent::Move_Backward);
    Bind(KEY_A, InputEvent::Move_Left);
    Bind(KEY_D, InputEvent::Move_Right);
    Bind(KEY_SPACE, InputEvent::Jump);
    Bind(KEY_LEFT_SHIFT, InputEvent::Run);
    Bind(KEY_LEFT_ALT, InputEvent::Sneak);
    Bind(KEY_G, InputEvent::Grab);
    Bind(KEY_I, InputEvent::Inventory);
    Bind(MOUSE_BUTTON_1, InputEvent::Shoot);
    Bind(MOUSE_BUTTON_3, InputEvent::Aim);
    Bind(MOUSE_WHEEL_Y, InputEvent::Scroll_Y);
    Bind(KEY_R, InputEvent::Edit_Blocks_Replace);
    Bind(KEY_E, InputEvent::Edit_Mode_Blocks);
    Bind(KEY_TAB, InputEvent::Edit_Mode_Object);
    Bind(KEY_M, InputEvent::Edit_Grab_Cursor);
}

// Text input
void Input::StartTextInput(TextInputEventListener* observer)
{
    //Enable text input
    SDL_StartTextInput();
    _textInputListener = observer;
    _inputText.clear();
}

void Input::StopTextInput(TextInputEventListener* observer)
{
    _textInputListener = nullptr;
    // Disable text input
    SDL_StopTextInput();
}

void Input::MoveCursor(const glm::ivec2 coord)
{
    SDL_WarpMouseInWindow(_window.GetSDLWindow(),
                          coord.x,
                          coord.y);
}
