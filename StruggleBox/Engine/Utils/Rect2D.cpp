#include "Rect2D.h"
#include <stdlib.h>

Rect2D::Rect2D(float _x, float _y, float _w, float _h)
    : x(_x)
    , y(_y)
    , w(_w)
    , h(_h)
{
}

Rect2D::Rect2D(const glm::vec2& center, const glm::vec2& size)
    : x(center.x - size.x * 0.5f)
    , y(center.y - size.y * 0.5f)
    , w(size.x)
    , h(size.y)
{
}


// Check if rectangle contains a 2D vector
bool Rect2D::contains(const glm::vec2& vVec) const
{
    if ((vVec.x >= x) &&
        (vVec.x <= x + w) &&
        (vVec.y >= y) &&
        (vVec.y <= y + h))
    { return true; }
    else
    { return false; }
}

// Check if rectangle contains a set of coords
bool Rect2D::contains(float x, float y) const
{
    if ((x >= this->x) &&
        (x <= this->x + this->w) &&
        (y >= this->y) &&
        (y <= this->y + this->h))
    {
        return true;
    }
    else
        return false;
}

// Check if rectangle intersects (or contains) another rectangle
bool Rect2D::intersects(Rect2D r) const
{
    if ((r.x + r.w < x) ||
        (r.x > x + w) ||
        (r.y + r.h < y) ||
        (r.y > y + h))
    {
        return false;
    }
    else
        return true;
}

// Return an empty rectangle
Rect2D Rect2D::Empty()
{
    return Rect2D();
}

// Get intersection depth between two rectangles
glm::vec2 Rect2D::getIntersectionDepth(const Rect2D& rectA, const Rect2D& rectB)
{
    // Calculate half sizes.
    float halfWidthA = rectA.w / 2.0f;
    float halfHeightA = rectA.h / 2.0f;
    float halfWidthB = rectB.w / 2.0f;
    float halfHeightB = rectB.h / 2.0f;
    
    // Calculate centers.
    glm::vec2 centerA(rectA.x + halfWidthA, rectA.y + halfHeightA);
    glm::vec2 centerB(rectB.x + halfWidthB, rectB.y + halfHeightB);
    
    // Calculate current and minimum-non-intersecting distances between centers.
    float distanceX = centerA.x - centerB.x;
    float distanceY = centerA.y - centerB.y;
    float minDistanceX = halfWidthA + halfWidthB;
    float minDistanceY = halfHeightA + halfHeightB;
    
    // If we are not intersecting at all, return (0, 0).
    if (std::abs(distanceX) >= minDistanceX ||
        std::abs(distanceY) >= minDistanceY)
        return glm::vec2();
    
    // Calculate and return intersection depths.
    float depthX = distanceX > 0 ? minDistanceX - distanceX : -minDistanceX - distanceX;
    float depthY = distanceY > 0 ? minDistanceY - distanceY : -minDistanceY - distanceY;
    return glm::vec2(depthX, depthY);
}

// Get the position of the center of the bottom edge of the rectangle.
glm::vec2 Rect2D::getBottomCenter(const Rect2D& rect) {
    return glm::vec2((float)(rect.x + rect.w / 2.0f), (float)(rect.y + rect.h));
}

// Get the position of the center point of a rectangle
glm::vec2 Rect2D::getCenter(const Rect2D& rect) {
    return glm::vec2((float)(rect.x + rect.w / 2.0f), (float)(rect.y + rect.h / 2.0f));
}

// Get the floating point distance between the center point
// of one rectangle and the center point of another.
float Rect2D::getDistance(const Rect2D& rectA, const Rect2D& rectB)
{
    return glm::distance(getCenter(rectA), getCenter(rectB));
}

// Get the unit vector from one rectangle to another
glm::vec2 Rect2D::getDirection(const Rect2D& rectA, const Rect2D& rectB)
{
    glm::vec2 direction = getCenter(rectA) - getCenter(rectB);
    glm::normalize(direction);
    return direction;
}

Rect2D Rect2D::getIntersection(const Rect2D& rectA, const Rect2D& rectB)
{
    // Find rightmost left edge
    float left = rectA.x > rectB.x ? rectA.x : rectB.x;
    // Leftmost right edge
    float right = rectA.x+rectA.w < rectB.x+rectB.w ? rectA.x+rectA.w : rectB.x+rectB.w;
    float bottom = rectA.y > rectB.y ? rectA.y : rectB.y;
    float top = rectA.y+rectA.h < rectB.y+rectB.h ? rectA.y+rectA.h : rectB.y+rectB.h;
    return Rect2D(left, bottom, right-left, top-bottom);
}

Rect2D& Rect2D::operator= (const Rect2D& r2)
{
    if (this == &r2)
        return *this;
    
    x = r2.x;
    y = r2.y;
    w = r2.w;
    h = r2.h;
    
    return *this;
}

bool Rect2D::operator== (const Rect2D& r2) const
{
    return ((w == r2.w) && (h == r2.h));
}

bool Rect2D::operator!= (const Rect2D& r2) const
{
    return !((w == r2.w) && (h == r2.h));
}
