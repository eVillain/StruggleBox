#include "OSWindow.h"
#include "Log.h"
#include <SDL3/SDL.h>
#include <cassert>

OSWindow::OSWindow()
: _window(nullptr)
, _title("StruggleBox")
, _open(false)
, _rawGLContext(nullptr)
{
	//Log::Info("[OSWindow] constructor, instance at %p", this);
}

OSWindow::~OSWindow()
{
	//Log::Info("[OSWindow] destructor, instance at %p", this);

    if (_open)
    {
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
    if (_window)
    {
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
    _window = SDL_CreateWindow(_title.c_str(), width, height, flags);        
    
    //Create context
    _rawGLContext = SDL_GL_CreateContext(_window);
    if(_rawGLContext != nullptr)
    {
        _open = true;
    }
    else
    {
        Log::Error("[OSWindow] OpenGL context could not be created!");
        Log::Error("[OSWindow] SDL Error: %s", SDL_GetError());
    }
    return _open;
}
