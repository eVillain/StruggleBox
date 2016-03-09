//
//  3DAABB.cpp
//  Ingenium
//
//  Created by The Drudgerist on 21/02/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "AABB3D.h"

AABB3D::AABB3D( const glm::vec3 max, const glm::vec3 min ) :
m_min(min), m_max(max) {
    
}

AABB3D::~AABB3D() {
    
}
void AABB3D::Add( const glm::vec3 limits ) {
    if ( IsClear() ) Move(limits);
    else {
        if ( limits.x < m_min.x ) { m_min.x = limits.x; }
        else if ( limits.x > m_max.x ) { m_max.x = limits.x; }
        if ( limits.y < m_min.y ) { m_min.y = limits.y; }
        else if ( limits.y > m_max.y ) { m_max.y = limits.y; }
        if ( limits.z < m_min.z ) { m_min.z = limits.z; }
        else if ( limits.z > m_max.z ) { m_max.z = limits.z; }
    }
}
void AABB3D::Move( const glm::vec3 move ) {
    m_min += move;
    m_max += move;
}
void AABB3D::Clear() {
    m_min = glm::vec3();
    m_max = glm::vec3();
}
bool AABB3D::IsClear() const {
    if ( m_min == glm::vec3() &&
         m_max == glm::vec3() )
        return true;
    else
        return false;
}
bool AABB3D::HasVolume() const {
    if ( m_min == m_max )
        return false;
    else
        return true;
}
bool AABB3D::Contains( glm::vec3 point ) const {
    if ( ( point.x >= m_min.x ) && ( point.x <= m_max.x ) &&
         ( point.y >= m_min.y ) && ( point.y <= m_max.y ) &&
         ( point.z >= m_min.z ) && ( point.z <= m_max.z ) )
        return true;
    else
        return false;
}
bool AABB3D::Contains(const float x, const float y, const float z) const {
    if ( ( x >= m_min.x ) && ( x <= m_max.x ) &&
         ( y >= m_min.y ) && ( y <= m_max.y ) &&
         ( z >= m_min.z ) && ( z <= m_max.z ) )
        return true;
    else
        return false;
}

glm::vec3 AABB3D::GetVolume() const {
    return (m_max-m_min);
}

glm::vec3 AABB3D::GetCenter() const {
    return m_min+((m_max-m_min)*0.5f);
}


