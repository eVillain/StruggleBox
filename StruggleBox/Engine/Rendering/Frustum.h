//
//  Frustum.h
//  Bloxelizer
//
//  Created by Ville-Veikko Urrila on 4/28/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#ifndef BWO_FRUSTUM_H
#define BWO_FRUSTUM_H

#include "GFXDefines.h"

namespace Frustum {
    void Extract( glm::mat4& mvp );
    bool PointInFrustum( GLfloat x, GLfloat y, GLfloat z );
    GLfloat SphereInFrustumDist( GLfloat x, GLfloat y, GLfloat z, GLfloat radius );
    int SphereInFrustum( GLfloat x, GLfloat y, GLfloat z, GLfloat radius );
    int CubeInFrustum( GLfloat x, GLfloat y, GLfloat z, GLfloat size );
    GLfloat CubeInFrustumDist( GLfloat x, GLfloat y, GLfloat z, GLfloat size );
    int BoxInFrustum( GLfloat x, GLfloat y, GLfloat z, GLfloat sizeX, GLfloat sizeY, GLfloat sizeZ );
    GLfloat BoxInFrustumDist( GLfloat x, GLfloat y, GLfloat z, GLfloat sizeX, GLfloat sizeY, GLfloat sizeZ );
}
#endif
