#include "AppContext.h"
#include "Log.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>

bool AppContext::_sdlInitialized = false;
bool AppContext::_glewInitialized = false;

AppContext::AppContext()
{
	Log::Info("[AppContext] constructor, instance at %p", this);
}

AppContext::~AppContext()
{
	Log::Info("[AppContext] destructor, instance at %p", this);
}

bool AppContext::InitApp(const std::string title,
                         const int windowWidth,
                         const int windowHeight,
                         const bool fullScreen)
{

    if (!InitSDL()) return false;
    if (fullScreen) {
        if (!_window.OpenFullScreen(title, windowWidth, windowHeight)) return false;
    } else {
        if (!_window.OpenWindow(title, windowWidth, windowHeight)) return false;
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
    if (_sdlInitialized) return true;
    
    // Init SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("[AppContext] SDL_Init_Everything failed!");
        return false;
    }
    
    //Use OpenGL 3.2 core
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    
    _sdlInitialized = true;
    return _sdlInitialized;
}

bool AppContext::InitGLEW()
{
    if (_glewInitialized) return true;
    
    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        printf("[AppContext] Error initializing GLEW! %s",
               glewGetErrorString(glewError));
        return false;
    }
    glGetError();   // Ignore one bad ENUM in GLEW initialization
    
    const GLubyte * version = glGetString(GL_VERSION);
    printf("[AppContext] OpenGL Version:%s\n", version);
    
    GLint major = 0;
    GLint minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 3 || (major == 3 && minor < 2))
    {
        printf("[AppContext] Error could not get a modern OpenGL context!");
    }
    
    //Use Vsync
    if( SDL_GL_SetSwapInterval( 1 ) < 0 )
    {
        printf("[AppContext] Unable to set VSync! SDL Error: %s",
               SDL_GetError());
    }
    
    printf("[AppContext] Created OpenGL context.\n");
    printf("[AppContext] Pixel format: %s\n",
            SDL_GetPixelFormatName(SDL_GetWindowPixelFormat(_window)));
    _glewInitialized = true;
    
    return _glewInitialized;
}

