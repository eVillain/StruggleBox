#pragma once

#include "glm/glm.hpp"

class Rect2D
{
public:
    Rect2D(float x = 0, float y = 0, float w = 0, float h = 0);
    Rect2D(const glm::vec2& center, const glm::vec2& size);
    ~Rect2D() {};
    
    float left() const { return x; }
    float right() const { return x + w; }
    float top() const { return y + h; }
    float bottom() const { return y; }
    glm::vec2 origin() const { return glm::vec2(x, y); }
    glm::vec2 size() const { return glm::vec2(w, h); }

    bool contains( const glm::vec2& vVec ) const;
    bool contains( float x, float y ) const;
    bool intersects( Rect2D r ) const;
    
    static Rect2D Empty();
    
    // Static methods below are derived from the Rect2DExtensions class
    // written in C#, released under the MSPL
    static glm::vec2 getIntersectionDepth(const Rect2D& rectA, const Rect2D& rectB);
    static glm::vec2 getBottomCenter(const Rect2D& rect);
    static glm::vec2 getCenter(const Rect2D& rect);
    static float getDistance(const Rect2D& rectA, const Rect2D& rectB);
    static glm::vec2 getDirection(const Rect2D& rectA, const Rect2D& rectB);
    
    static Rect2D getIntersection(const Rect2D& rectA, const Rect2D& rectB);
    
    Rect2D& operator= (const Rect2D& r2);
    
    bool operator== (const Rect2D& r2) const;
    bool operator!= (const Rect2D& r2) const;
    
public:
    float x, y, w, h;
};
