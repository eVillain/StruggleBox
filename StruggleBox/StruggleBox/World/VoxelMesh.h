#ifndef VOXEL_MESH_H
#define VOXEL_MESH_H

#include "Mesh.h"

class VoxelMesh : public Mesh
{
public:
	VoxelMesh(std::shared_ptr<Renderer> renderer);
	~VoxelMesh();

private:

};


#endif // !VOXEL_MESH_H
