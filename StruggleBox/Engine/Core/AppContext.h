#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include "Window.h"
#include <string>

/// Simple Context for setting up an SDL / OpenGL application
class AppContext
{
public:
    bool InitApp(const std::string title,
                 const int windowWidth,
                 const int windowHeight,
                 const bool fullScreen);
    bool TerminateApp();
    
    Window* GetWindow() { return &_window; }
private:
    Window _window;

    bool InitSDL();
    bool InitGLEW();
    
    static bool _sdlInitialized;
    static bool _glewInitialized;
};
#endif /* APPCONTEXT_H */
