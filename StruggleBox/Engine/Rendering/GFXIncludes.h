#ifndef NGN_GFX_INCLUDES_H
#define NGN_GFX_INCLUDES_H

#ifdef _WIN32
// Gotta include windows.h before GLEW & GLFW
// But gotta include winsock2.h before windows.h
// What a mess...
    #include <winsock2.h>
    #include <windows.h>
// Using GLEW as static lib in windows
    #define GLEW_STATIC
#endif
// Include GLEW and GLFW
#include "GL/glew.h"
//#include "GLFW/glfw3.h"
// Include GLM
#include "glm/glm.hpp"

#endif
