#ifndef WINDOW_H
#define WINDOW_H

#include <string>

class SDL_Window;

/// Wraps an SDL OpenGL Window / Fullscreen surface
class Window
{
public:
    Window();
    ~Window();
    operator SDL_Window*() const;
    
    bool OpenWindow(const std::string title,
                    const int width,
                    const int height);
    
    bool OpenFullScreen(const std::string title,
                        const int width,
                        const int height);
    
    bool Close();
    
    void SwapBuffers() const;
    
    const int GetWidth() const;
    const int GetHeight() const;
    
    SDL_Window* GetSDLWindow() { return _window; }
    void* GetGLContext() { return _rawGLContext; }
    std::string GetTitle() { return _title; }
    bool IsOpen() { return _open; }
    
    void SetTitle(const std::string title);
private:
    SDL_Window * _window;
    void * _rawGLContext;
    std::string _title;
    bool _open;
    
    bool Open(const std::string title,
              const int width,
              const int height,
              const bool fullscreen);
    
    
};

#endif /* WINDOW_H */
