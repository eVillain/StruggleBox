#include "AppContext.h"
#include "Log.h"
#include <SDL3/SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>

bool AppContext::s_sdlInitialized = false;
bool AppContext::s_glewInitialized = false;

AppContext::AppContext()
{
	//Log::Info("[AppContext] constructor, instance at %p", this);
}

AppContext::~AppContext()
{
	//Log::Info("[AppContext] destructor, instance at %p", this);
}

bool AppContext::InitApp(const std::string title,
                         const int windowWidth,
                         const int windowHeight,
                         const bool fullScreen)
{

    if (!InitSDL()) return false;
    if (fullScreen)
    {
        if (!m_window.OpenFullScreen(title, windowWidth, windowHeight)) return false;
    }
    else
    {
        if (!m_window.OpenWindow(title, windowWidth, windowHeight)) return false;
    }
    if (!InitGLEW()) return false;
    return true;
}

bool AppContext::TerminateApp()
{
    SDL_Quit();
    return true;
}

bool AppContext::InitSDL()
{
    if (s_sdlInitialized) return true;
    
    // Init SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        Log::Error("[AppContext] SDL_Init failed!");
        Log::Error("[AppContext] SDL Error: %s", SDL_GetError());
        return false;
    }
    
    //Use OpenGL 4.6 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    s_sdlInitialized = true;
    return s_sdlInitialized;
}

bool AppContext::InitGLEW()
{
    if (s_glewInitialized) return true;
    
    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        Log::Error("[AppContext] Error initializing GLEW! %s",
               glewGetErrorString(glewError));
        return false;
    }
    glGetError();   // Ignore one bad ENUM in GLEW initialization
    
    const GLubyte * version = glGetString(GL_VERSION);
    //Log::Info("[AppContext] OpenGL Version:%s", version);
    
    GLint major = 0;
    GLint minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 3 || (major == 3 && minor < 2))
    {
        Log::Error("[AppContext] Error could not get a modern OpenGL context!");
    }
    
    //Use Vsync
    if (!SDL_GL_SetSwapInterval(1))
    {
        Log::Error("[AppContext] Unable to set VSync! SDL Error: %s",
               SDL_GetError());
    }
    
    //Log::Info("[AppContext] Created OpenGL context.");
    //Log::Info("[AppContext] Pixel format: %s", SDL_GetPixelFormatName(SDL_GetWindowPixelFormat(m_window)));
    s_glewInitialized = true;
    
    return s_glewInitialized;
}

