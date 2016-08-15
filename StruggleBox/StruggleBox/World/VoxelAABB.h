#ifndef VOXEL_AABB_H
#define VOXEL_AABB_H

class VoxelAABB
{
public:
    glm::vec3 m_min;
    glm::vec3 m_max;
    uint8_t m_voxel;
    
    VoxelAABB(const glm::vec3 min,
              const glm::vec3 max,
              const uint8_t voxel)
	{
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
