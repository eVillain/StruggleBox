#ifndef CORE_INCLUDES_H
#define CORE_INCLUDES_H

#ifdef _WIN32
// Gotta include windows.h before GLEW & GLFW
// But gotta include winsock2.h before windows.h
// What a mess...
#include <winsock2.h>
#include <windows.h>
// Using GLEW as static lib in windows
#define GLEW_STATIC

#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#ifndef HAVE_M_PI
#define HAVE_M_PI
#endif

#endif

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "SDL2/SDL.h"

#ifndef ZLIB_WINAPI
#define ZLIB_WINAPI
#endif
#include "zlib.h"

// ugly hack to avoid zlib corruption on win systems
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#endif // CORE_INCLUDES_H