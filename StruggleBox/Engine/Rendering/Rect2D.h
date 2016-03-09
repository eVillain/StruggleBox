//
//  Rect2D.h
//  NeverEngine
//
//  Created by The Drudgerist on 1/19/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#ifndef BWO_RECT2D_H
#define BWO_RECT2D_H

//#include "Vector2D.h"
#include "glm/glm.hpp"

class Rect2D
{
public:
    Rect2D( float x = 0, float y = 0, float w = 0, float h = 0 );
    ~Rect2D( void ) {};
    
    inline float Left( void ) const { return x; }
    inline float Right( void ) const { return x + w; }
    inline float Top( void ) const { return y; }
    inline float Bottom( void ) const { return y + h; }
    inline glm::vec2 Origin( void ) const { return glm::vec2(x,y); }
    
    bool Contains( glm::vec2& vVec ) const;
    bool Contains( float x, float y ) const;
    bool Intersects( Rect2D r ) const;
    
    static Rect2D Empty();
    
    // Static methods below are derived from the Rect2DExtensions class
    // written in C#, released under the MSPL
    static glm::vec2 GetIntersectionDepth( const Rect2D& rectA, const Rect2D& rectB );
    static glm::vec2 GetBottomCenter( const Rect2D& rect );
    static glm::vec2 GetCenter( const Rect2D& rect );
    static float GetDistance( const Rect2D& rectA, const Rect2D& rectB);
    static glm::vec2 GetDirection( const Rect2D& rectA, const Rect2D& rectB);
    
    static Rect2D GetIntersection( const Rect2D& rectA, const Rect2D& rectB );
    
    Rect2D& operator= ( const Rect2D& r2 );
    
    bool operator== ( const Rect2D& r2 ) const;
    bool operator!= ( const Rect2D& r2 ) const;
    
public:
    float x, y, w, h;
};


#endif /* defined BWO_RECT2D_H */
