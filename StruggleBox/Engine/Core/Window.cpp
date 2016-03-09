#include "Window.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <cassert>


Window::Window() :
_window(nullptr),
_title("DrudgeGL Window"),
_open(false)
{ }

Window::~Window()
{
    if (_open) {
        Close();
    }
}

Window::operator SDL_Window*() const
{
    return _window;
}

bool Window::OpenWindow(const std::string title,
                        const int width,
                        const int height)
{
    assert(!_open);
    
    return Open(title, width, height, false);
}

bool Window::OpenFullScreen(const std::string title,
                            const int width,
                            const int height)
{
    assert(!_open);
    
    return Open(title, width, height, true);
}

bool Window::Close()
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

void Window::SwapBuffers() const
{
    // Swap buffers, brings backbuffer to front and vice-versa
    SDL_GL_SwapWindow(_window);
}

const int Window::GetWidth() const
{
    int windowWidth, windowHeight;
    SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
    return windowWidth;
}

const int Window::GetHeight() const
{
    int windowWidth, windowHeight;
    SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
    return windowHeight;
}

void Window::SetTitle(const std::string title)
{
    _title = title;
    SDL_SetWindowTitle(_window, title.c_str());
}

bool Window::Open(const std::string title,
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
        printf("[Window] OpenGL context could not be created!\n");
        printf("[Window] SDL Error: %s", SDL_GetError());
    }
    return _open;
}
