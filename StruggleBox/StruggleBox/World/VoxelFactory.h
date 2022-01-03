#ifndef VOXEL_FACTORY_H
#define VOXEL_FACTORY_H

#include "InstancedMesh.h"
#include "VoxelData.h"
#include "Renderer.h"
#include <map>
#include <string>

class Allocator;

const float DEFAULT_VOXEL_MESHING_WIDTH = 0.25;

class VoxelFactory
{
public:
	VoxelFactory(Renderer& renderer, Allocator& allocator);
	~VoxelFactory();

	void draw();

	const unsigned int addInstance(const std::string& fileName);
	VoxelData* getVoxels(const std::string& fileName);
	InstancedMesh* getMesh(const std::string& fileName);

	bool isLoaded(const std::string& fileName);
	bool isMeshed(const std::string& fileName);

private:
	VoxelData* loadVoxels(const std::string& fileName);
	InstancedMesh* loadMesh(const std::string& fileName);

	Renderer& _renderer;
	Allocator& m_allocator;

	std::map<std::string, InstancedMesh*> _meshes;
	std::map<std::string, VoxelData*> _voxels;
};

#endif // !VOXEL_FACTORY_H
