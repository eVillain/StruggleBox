#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include "OSWindow.h"
#include <string>

/// Simple Context for setting up an SDL / OpenGL application
class AppContext
{
public:
	AppContext();
	~AppContext();

    bool InitApp(const std::string title,
                 const int windowWidth,
                 const int windowHeight,
                 const bool fullScreen);
    bool TerminateApp();
    
	OSWindow* GetWindow() { return &_window; }
private:
	OSWindow _window;

    bool InitSDL();
    bool InitGLEW();
    
    static bool _sdlInitialized;
    static bool _glewInitialized;
};
#endif /* APPCONTEXT_H */
