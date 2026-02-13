#pragma once

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
    
	OSWindow* GetWindow() { return &m_window; }
private:
	OSWindow m_window;

    bool InitSDL();
    bool InitGLEW();
    
    static bool s_sdlInitialized;
    static bool s_glewInitialized;
};
