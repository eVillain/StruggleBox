#include "OSWindow.h"
#include "Log.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <cassert>


OSWindow::OSWindow() :
_window(nullptr),
_title("StruggleBox"),
_open(false)
{
	Log::Info("[OSWindow] constructor, instance at %p", this);
}

OSWindow::~OSWindow()
{
	Log::Info("[OSWindow] destructor, instance at %p", this);

    if (_open) {
        Close();
    }
}

OSWindow::operator SDL_Window*() const
{
    return _window;
}

bool OSWindow::OpenWindow(const std::string title,
                        const int width,
                        const int height)
{
    assert(!_open);
    
    return Open(title, width, height, false);
}

bool OSWindow::OpenFullScreen(const std::string title,
                            const int width,
                            const int height)
{
    assert(!_open);
    
    return Open(title, width, height, true);
}

bool OSWindow::Close()
{
    assert(_open);
    
    // clean up
    if ( _window ) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
        
        _open = false;
        
        return true;
    }
    return false;
}

void OSWindow::SwapBuffers() const
{
    // Swap buffers, brings backbuffer to front and vice-versa
    SDL_GL_SwapWindow(_window);
}

const int OSWindow::GetWidth() const
{
    int windowWidth, windowHeight;
    SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
    return windowWidth;
}

const int OSWindow::GetHeight() const
{
    int windowWidth, windowHeight;
    SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
    return windowHeight;
}

void OSWindow::SetTitle(const std::string title)
{
    _title = title;
    SDL_SetWindowTitle(_window, title.c_str());
}

bool OSWindow::Open(const std::string title,
                  const int width,
                  const int height,
                  const bool fullScreen)
{
    assert(!_open);
    
    _title = title;
    
    Uint32 flags = SDL_WINDOW_OPENGL;
    if (fullScreen) flags |= SDL_WINDOW_FULLSCREEN;
    
    // Create a window
    _window = SDL_CreateWindow(_title.c_str(),          // window title
                               SDL_WINDOWPOS_CENTERED,  // x position, centered
                               SDL_WINDOWPOS_CENTERED,  // y position, centered
                               width,                   // width, in pixels
                               height,                  // height, in pixels
                               SDL_WINDOW_OPENGL);      // flags
    
    //Create context
    _rawGLContext = SDL_GL_CreateContext( _window );
    if( _rawGLContext != NULL ) {
        _open = true;
    } else {
        printf("[OSWindow] OpenGL context could not be created!\n");
        printf("[OSWindow] SDL Error: %s", SDL_GetError());
    }
    return _open;
}
