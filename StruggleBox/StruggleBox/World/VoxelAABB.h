//
//  VoxelAABB.h
//  Ingenium
//
//  Created by The Drudgerist on 28/12/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#ifndef NGN_VOXEL_AABB_H
#define NGN_VOXEL_AABB_H

class VoxelAABB
{
public:
    glm::vec3 m_min;
    glm::vec3 m_max;
    BlockType m_voxel;
    
    VoxelAABB(const glm::vec3 min,
              const glm::vec3 max,
              const BlockType voxel) {
        m_min = min;
        m_max = max;
        m_voxel = voxel;
    }
    // Checks to see if the given voxel is encompassed by the AABB
    bool Contains(int x, int y, int z) {
        if ( ( x >= m_min.x )&&
            ( x <= m_max.x ) &&
            ( y >= m_min.y ) &&
            ( y <= m_max.y ) &&
            ( z >= m_min.z ) &&
            ( z <= m_max.z ) )
        {
            return true;
        }
        else
            return false;
    };
    
};

#endif
