#ifndef AABB3D_H
#define AABB3D_H

#include "GFXDefines.h"

struct AABB3D
{
    glm::vec3 m_min;
    glm::vec3 m_max;
    
    AABB3D( const glm::vec3 min=glm::vec3(), const glm::vec3 max=glm::vec3() );
    ~AABB3D();
    void Add( const glm::vec3 limits );     // Sets new max/min automatically
    void Move( const glm::vec3 move );      // Moves selection by amount
    void Clear();                           // Clear min and max
    bool IsClear() const;                   // Check if min and max are clear
    bool HasVolume() const;                 // Check if min == max
    bool Contains( glm::vec3 point ) const; // Test if the given point is encompassed by the AABB
    bool Contains( const float x, const float y, const float z) const;
    glm::vec3 GetVolume() const;            // Get max-min
    glm::vec3 GetCenter() const;            // Get min+(volume/2)
};


#endif /* defined(NGN_AABB3D_H) */
