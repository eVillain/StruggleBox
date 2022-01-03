
#pragma once

// Based on http://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/

void _check_gl_error(const char *file, int line);

///
/// Usage
/// [... some opengl calls]
/// check_gl_error();
///
#define CHECK_GL_ERROR() _check_gl_error(__FILE__,__LINE__)
