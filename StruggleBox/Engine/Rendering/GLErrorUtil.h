//
//  GLErrorUtil.h
//  DungeonSmith
//
//  Created by The Drudgerist on 07/05/15.
//  Copyright (c) 2015 The Drudgerist. All rights reserved.
//
// Based on http://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/

#ifndef __DungeonSmith__GLErrorUtil__
#define __DungeonSmith__GLErrorUtil__

void _check_gl_error(const char *file, int line);

///
/// Usage
/// [... some opengl calls]
/// check_gl_error();
///
#define CHECK_GL_ERROR() _check_gl_error(__FILE__,__LINE__)

#endif /* defined(__DungeonSmith__GLErrorUtil__) */
