//
//  RenderUtils.h
//  DungeonSmith
//
//  Created by The Drudgerist on 01/08/15.
//  Copyright (c) 2015 The Drudgerist. All rights reserved.
//

#ifndef DungeonSmith_RenderUtils_h
#define DungeonSmith_RenderUtils_h


/*  ----------------------------    *
 *    OpenGL GBuffer STUFF      *
 *  ----------------------------    */
class RenderUtils {
public:
    static GLuint GenerateTextureRGBAF(GLuint width, GLuint height, const GLvoid* data = NULL)
    {
        GLuint returnTexture = -1;
        glGenTextures(1, &returnTexture);  // Generate rgb color render texture
        glBindTexture(GL_TEXTURE_2D, returnTexture);   // Bind current texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, data);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        return returnTexture;
    }
    
    static GLuint GenerateTextureDepth(GLuint width, GLuint height)
    {
        GLuint returnTexture = -1;
        glGenTextures(1, &returnTexture);  // Generate rgb color render texture
        glBindTexture(GL_TEXTURE_2D, returnTexture);   // Bind current texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glBindTexture(GL_TEXTURE_2D, 0);
        return returnTexture;
    }
    
    static GLuint GenerateTextureNormal(GLuint width, GLuint height)
    {
        GLuint returnTexture = -1;
        glGenTextures(1, &returnTexture);  // Generate rgb color render texture
        glBindTexture(GL_TEXTURE_2D, returnTexture);   // Bind current texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glBindTexture(GL_TEXTURE_2D, 0);
        return returnTexture;
    }

};

#endif
