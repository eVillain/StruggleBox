#include "Frustum.h"

namespace Frustum
{
    GLdouble frustum[6][4];
    
    //========================================================================
    // ExtractFrustum() - Frustum extraction function
    //========================================================================
    void Extract( glm::mat4& mvp ) {
        GLfloat t;
        
        /* Extract the numbers for the RIGHT plane */
        frustum[0][0] = mvp[0][3] - mvp[0][0];
        frustum[0][1] = mvp[1][3] - mvp[1][0];
        frustum[0][2] = mvp[2][3] - mvp[2][0];
        frustum[0][3] = mvp[3][3] - mvp[3][0];
        /* Normalize the result */
        t = sqrt( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
        frustum[0][0] /= t;
        frustum[0][1] /= t;
        frustum[0][2] /= t;
        frustum[0][3] /= t;
        
        /* Extract the numbers for the LEFT plane */
        frustum[1][0] = mvp[0][3] + mvp[0][0];
        frustum[1][1] = mvp[1][3] + mvp[1][0];
        frustum[1][2] = mvp[2][3] + mvp[2][0];
        frustum[1][3] = mvp[3][3] + mvp[3][0];

        /* Normalize the result */
        t = sqrt( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
        frustum[1][0] /= t;
        frustum[1][1] /= t;
        frustum[1][2] /= t;
        frustum[1][3] /= t;
        
        /* Extract the BOTTOM plane */
        frustum[2][0] = mvp[0][3] + mvp[0][1];
        frustum[2][1] = mvp[1][3] + mvp[1][1];
        frustum[2][2] = mvp[2][3] + mvp[2][1];
        frustum[2][3] = mvp[3][3] + mvp[3][1];
        /* Normalize the result */
        t = sqrt( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
        frustum[2][0] /= t;
        frustum[2][1] /= t;
        frustum[2][2] /= t;
        frustum[2][3] /= t;
        
        /* Extract the TOP plane */
        frustum[3][0] = mvp[0][3] - mvp[0][1];
        frustum[3][1] = mvp[1][3] - mvp[1][1];
        frustum[3][2] = mvp[2][3] - mvp[2][1];
        frustum[3][3] = mvp[3][3] - mvp[3][1];
        
        /* Normalize the result */
        t = sqrt( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
        frustum[3][0] /= t;
        frustum[3][1] /= t;
        frustum[3][2] /= t;
        frustum[3][3] /= t;
        
        /* Extract the FAR plane */
        frustum[4][0] = mvp[0][3] - mvp[0][2];
        frustum[4][1] = mvp[1][3] - mvp[1][2];
        frustum[4][2] = mvp[2][3] - mvp[2][2];
        frustum[4][3] = mvp[3][3] - mvp[3][2];
        /* Normalize the result */
        t = sqrt( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
        frustum[4][0] /= t;
        frustum[4][1] /= t;
        frustum[4][2] /= t;
        frustum[4][3] /= t;
        
        /* Extract the NEAR plane */
        frustum[5][0] = mvp[0][3] + mvp[0][2];
        frustum[5][1] = mvp[1][3] + mvp[1][2];
        frustum[5][2] = mvp[2][3] + mvp[2][2];
        frustum[5][3] = mvp[3][3] + mvp[3][2];
        /* Normalize the result */
        t = sqrt( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
        frustum[5][0] /= t;
        frustum[5][1] /= t;
        frustum[5][2] /= t;
        frustum[5][3] /= t;
    }
    //========================================================================
    // PointInFrustum() - Test if point is in frustum
    //========================================================================
    bool PointInFrustum( GLfloat x, GLfloat y, GLfloat z ) {
        int p;
        for( p = 0; p < 6; p++ )
            if( frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3] <= 0 )
                return false;
        return true;
    }
    //========================================================================
    // SphereInFrustumS() - Test if sphere is in frustum
    //========================================================================
    bool SphereInFrustumS( GLfloat x, GLfloat y, GLfloat z, GLfloat radius ) {
        int p;
        for( p = 0; p < 6; p++ ) {
            if( frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3] <= -radius )
            { return false; }
        }
        return true;
    }
    //========================================================================
    // SphereInFrustumDist() - Get distance of circle from frustum
    //========================================================================
    GLfloat SphereInFrustumDist( GLfloat x, GLfloat y, GLfloat z, GLfloat radius ) {
        int p;
        GLfloat d = 0.0;
        for ( p = 0; p < 6; p++ ) {
            d = frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3];
            if( d <= -radius )
                return 0;
        }
        return d + radius;
    }
    //========================================================================
    // SphereInFrustum() - Test if sphere is in frustum, return 2 if fully in
    //========================================================================
    int SphereInFrustum( GLfloat x, GLfloat y, GLfloat z, GLfloat radius ) {
        int p;
        int c = 0;
        float d;
        for ( p = 0; p < 6; p++ ) {
            d = frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3];
            if( d <= -radius )
            { return 0; }
            if( d > radius )
            { c++; }
        }
        return (c == 6) ? 2 : 1;
    }
    //========================================================================
    // CubeInFrustum() - Test if cube is in frustum, return 2 if fully in
    //========================================================================
    int CubeInFrustum( GLfloat x, GLfloat y, GLfloat z, GLfloat size ) {
        int p;
        int c;
        int c2 = 0;
        for ( p = 0; p < 6; p++ ) {
            c = 0;
            if( frustum[p][0] * (x - size) + frustum[p][1] * (y - size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + size) + frustum[p][1] * (y - size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - size) + frustum[p][1] * (y + size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - size) + frustum[p][1] * (y - size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + size) + frustum[p][1] * (y - size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
                c++;
            if( c == 0 )
            { return 0; }
            if( c == 8 )
            { c2++; }
        }
        return (c2 == 6) ? 2 : 1;
    }
    //========================================================================
    // CubeInFrustumDist() - Test if cube is in frustum, return 2 if fully in
    //========================================================================
    GLfloat CubeInFrustumDist( GLfloat x, GLfloat y, GLfloat z, GLfloat size ) {
        int p;
        int c;
        int c2 = 0;
        for ( p = 0; p < 6; p++ ) {
            c = 0;
            if( frustum[p][0] * (x - size) + frustum[p][1] * (y - size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + size) + frustum[p][1] * (y - size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - size) + frustum[p][1] * (y + size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - size) + frustum[p][1] * (y - size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + size) + frustum[p][1] * (y - size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
                c++;
            if( c == 0 )
            { return 0; }
            if( c == 8 )
            { c2++; }
        }
        GLfloat d = frustum[5][0] * x + frustum[5][1] * y + frustum[5][2] * z + frustum[5][3];
        return d+size;
    }
    //========================================================================
    // BoxInFrustum() - Test if cube is in frustum, return 2 if fully in
    //========================================================================
    int BoxInFrustum( GLfloat x, GLfloat y, GLfloat z, GLfloat sizeX, GLfloat sizeY, GLfloat sizeZ ) {
        int p;
        int c;
        int c2 = 0;
        for ( p = 0; p < 6; p++ ) {
            c = 0;
            if( frustum[p][0] * (x - sizeX) + frustum[p][1] * (y - sizeY) + frustum[p][2] * (z - sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + sizeX) + frustum[p][1] * (y - sizeY) + frustum[p][2] * (z - sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - sizeX) + frustum[p][1] * (y + sizeY) + frustum[p][2] * (z - sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + sizeX) + frustum[p][1] * (y + sizeY) + frustum[p][2] * (z - sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - sizeX) + frustum[p][1] * (y - sizeY) + frustum[p][2] * (z + sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + sizeX) + frustum[p][1] * (y - sizeY) + frustum[p][2] * (z + sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - sizeX) + frustum[p][1] * (y + sizeY) + frustum[p][2] * (z + sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + sizeX) + frustum[p][1] * (y + sizeY) + frustum[p][2] * (z + sizeZ) + frustum[p][3] > 0 )
                c++;
            if( c == 0 )
            { return 0; }
            if( c == 8 )
            { c2++; }
        }
        return (c2 == 6) ? 2 : 1;
    }
    //========================================================================
    // CubeInFrustumDist() - Test if cube is in frustum, return 2 if fully in
    //========================================================================
    GLfloat CubeInFrustumDist( GLfloat x, GLfloat y, GLfloat z, GLfloat sizeX, GLfloat sizeY, GLfloat sizeZ ) {
        int p;
        int c;
        int c2 = 0;
        for ( p = 0; p < 6; p++ ) {
            c = 0;
            if( frustum[p][0] * (x - sizeX) + frustum[p][1] * (y - sizeY) + frustum[p][2] * (z - sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + sizeX) + frustum[p][1] * (y - sizeY) + frustum[p][2] * (z - sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - sizeX) + frustum[p][1] * (y + sizeY) + frustum[p][2] * (z - sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + sizeX) + frustum[p][1] * (y + sizeY) + frustum[p][2] * (z - sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - sizeX) + frustum[p][1] * (y - sizeY) + frustum[p][2] * (z + sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + sizeX) + frustum[p][1] * (y - sizeY) + frustum[p][2] * (z + sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x - sizeX) + frustum[p][1] * (y + sizeY) + frustum[p][2] * (z + sizeZ) + frustum[p][3] > 0 )
                c++;
            if( frustum[p][0] * (x + sizeX) + frustum[p][1] * (y + sizeY) + frustum[p][2] * (z + sizeZ) + frustum[p][3] > 0 )
                c++;
            if( c == 0 )
            { return 0; }
            if( c == 8 )
            { c2++; }
        }
        GLfloat d = frustum[5][0] * x + frustum[5][1] * y + frustum[5][2] * z + frustum[5][3];
        return d+((sizeX+sizeY+sizeZ)/3.0f);
    }

}



