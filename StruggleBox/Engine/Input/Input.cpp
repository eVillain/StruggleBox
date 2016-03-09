#include "Input.h"
#include "Console.h"
#include "CommandProcessor.h"
#include "StringUtil.h"
#include "RangeReverseAdapter.h"
#include "Log.h"
#include "Locator.h"
#include "AppContext.h"
#include "Window.h"
#include <SDL2/SDL.h>

Input::Input(Locator& locator) :
_locator(locator),
_inputText(""),
_textInputListener(nullptr)
{ Log::Debug("[Input] Constructor..."); }

bool Input::Initialize()
{
    Log::Debug("[Input] Initializing...");
    // Add our binding mechanic to our console commands
    CommandProcessor::AddCommand("bind",
                                 Command<const std::string&,const std::string&>([=](const std::string& input,const std::string& event)
    {
        this->Bind(input, event);
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

void Input::Update(const double deltaTime)
{
    ProcessInput();
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
        Log::Warn("[Input] tried to unregister non-existent InputEventListener (%lu in stack)\n", _inputEventListeners.size());
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
        Log::Warn("[Input] tried to unregister non-existent MouseEventListener (%lu in stack)\n", _mouseEventListeners.size());
    }
}

void Input::Bind(std::string input,
                 std::string event)
{
    _inputBindings[input] = event;
}

/* Print modifier info */
void PrintModifiers( Uint32 mod )
{
    printf( "Modifers: " );
    
    /* If there are none then say so and return */
    if( mod == KMOD_NONE ){
        printf( "None\n" );
        return;
    }
    
    /* Check for the presence of each SDLMod value */
    /* This looks messy, but there really isn't    */
    /* a clearer way.                              */
    if( mod & KMOD_NUM ) printf( "NUMLOCK " );
    if( mod & KMOD_CAPS ) printf( "CAPSLOCK " );
    if( mod & KMOD_LCTRL ) printf( "LCTRL " );
    if( mod & KMOD_RCTRL ) printf( "RCTRL " );
    if( mod & KMOD_RSHIFT ) printf( "RSHIFT " );
    if( mod & KMOD_LSHIFT ) printf( "LSHIFT " );
    if( mod & KMOD_RALT ) printf( "RALT " );
    if( mod & KMOD_LALT ) printf( "LALT " );
    if( mod & KMOD_CTRL ) printf( "CTRL " );
    if( mod & KMOD_SHIFT ) printf( "SHIFT " );
    if( mod & KMOD_ALT ) printf( "ALT " );
    if( mod & KMOD_LGUI ) printf( "LGUI " );
    if( mod & KMOD_RGUI ) printf( "RGUI " );
    if( mod & KMOD_MODE ) printf( "MODE " );
    if( mod & KMOD_RESERVED ) printf( "RESERVED " );
    printf( "\n" );
}

/* Print all information about a key event */
void PrintKeyInfo( SDL_KeyboardEvent *key )
{
    /* Is it a release or a press? */
    if( key->type == SDL_KEYUP )
        printf( "Release:- " );
    else
        printf( "Press:- " );
    
    /* Print the hardware scancode first */
    printf( "Scancode: 0x%02X", key->keysym.scancode );
    /* Print the name of the key */
    printf( ", Name: %s", SDL_GetKeyName( key->keysym.sym ) );
    printf( "\n" );
    /* Print modifier info */
    PrintModifiers( key->keysym.mod );
}

void Input::ProcessInput()
{
    //Event handler
    SDL_Event event;
    
    /* Poll for events. SDL_PollEvent() returns 0 when there are no  */
    /* more events on the event queue, our while loop will exit when */
    /* that occurs.                                                  */
    while( SDL_PollEvent( &event ) )
    {
        //User requests quit, terminate gracefully
        if( event.type == SDL_QUIT )
        {
            CommandProcessor::ExecuteCommand("quit");
            return;
        }
        
        // Amount variable depends on press/release or axis status
        // Range -1.0 ~ 1.0
        float amount = 0.0f;
        std::string input;
        
        // Check for text input, that takes precedence over regular events
        if (event.type == SDL_KEYDOWN)
        {
            if ( event.key.repeat == 0 )
            {
                amount = 1.0f;
                input = SDL_GetKeyName( event.key.keysym.sym );
            }
        }
        else if (event.type == SDL_KEYUP)
        {
            if ( event.key.repeat == 0 )
            {
                amount = -1.0f;
                input = SDL_GetKeyName( event.key.keysym.sym );
            }
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            glm::ivec2 adjustedCoords = glm::ivec2(event.motion.x, event.motion.y);
            for ( MouseEventListener* listener : _mouseEventListeners )
            {
                bool swallowed = (*listener).OnMouse( adjustedCoords );
                if ( swallowed ) // Event was swallowed, don't propagate
                {
                    break;
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
                amount = event.wheel.y;
                input = "MouseWheelY";
            } else if (event.wheel.x != 0) {
                amount = event.wheel.x;
                input = "MouseWheelX";
            }
        }
        
        // We have to check for input events first, blocking the regular events
        if ( _textInputListener && ProcessTextInput(event))
        {
            // Call the registered observer
            (*_textInputListener).OnTextInput(_inputText);
        }
        else if ( input.length() )   // Regular events
        {
          printf("Input: %s\n", input.c_str());
            if ( _inputBindings.find(input) != _inputBindings.end() )
            {
                if (_textInputListener && input != "Escape" && input != "Return")
                {
                    return;
                }
//           printf("Bound to: %s\n", _inputBindings[input].c_str());
                for ( InputEventListener* listener : _inputEventListeners )
                {
                    bool swallowed = (*listener).OnEvent( _inputBindings[input], amount );
                    if ( swallowed )
                    { break; }  // Event was swallowed, don't propagate
                }
            }
        }
    }
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
    Bind(KEY_RETURN, INPUT_START);
    Bind(KEY_ESCAPE, INPUT_BACK);
    Bind(KEY_P, INPUT_PAUSE);
    Bind(KEY_C, INPUT_CONSOLE);

    Bind(KEY_DELETE, INPUT_ERASE_RIGHT);
    Bind(KEY_BACKSPACE, INPUT_ERASE_LEFT);
    Bind(KEY_UP, INPUT_LOOK_UP);
    Bind(KEY_DOWN, INPUT_LOOK_DOWN);
    Bind(KEY_LEFT, INPUT_LOOK_LEFT);
    Bind(KEY_RIGHT, INPUT_LOOK_RIGHT);
    Bind(KEY_W, INPUT_MOVE_FORWARD);
    Bind(KEY_S, INPUT_MOVE_BACK);
    Bind(KEY_A, INPUT_MOVE_LEFT);
    Bind(KEY_D, INPUT_MOVE_RIGHT);
    Bind(KEY_SPACE, INPUT_JUMP);
    Bind(KEY_LEFT_SHIFT, INPUT_RUN);
    Bind(KEY_RIGHT_SHIFT, INPUT_SNEAK);
    Bind(KEY_G, INPUT_GRAB);
    Bind(KEY_I, INPUT_INVENTORY);
    Bind(MOUSE_BUTTON_1, INPUT_SHOOT);
    Bind(MOUSE_BUTTON_3, INPUT_SHOOT2);
    Bind(MOUSE_WHEEL_Y, INPUT_SCROLL_Y);
    Bind(KEY_R, INPUT_BLOCKS_REPLACE);
    Bind(KEY_E, INPUT_EDIT_BLOCKS);
    Bind(KEY_TAB, INPUT_EDIT_OBJECT);
    Bind(KEY_M, INPUT_GRAB_CURSOR);
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
    SDL_WarpMouseInWindow(_locator.Get<AppContext>()->GetWindow()->GetSDLWindow(),
                          coord.x,
                          coord.y);
}
