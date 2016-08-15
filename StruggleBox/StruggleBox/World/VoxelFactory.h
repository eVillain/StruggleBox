#ifndef VOXEL_FACTORY_H
#define VOXEL_FACTORY_H

#include "InstancedMesh.h"
#include "VoxelData.h"
#include "Renderer.h"
#include <map>
#include <string>
const float DEFAULT_VOXEL_MESHING_WIDTH = 0.25;

class VoxelFactory
{
public:
	VoxelFactory(std::shared_ptr<Renderer> renderer);
	~VoxelFactory();

	void draw();

	const unsigned int addInstance(const std::string& fileName);
	std::shared_ptr<VoxelData> getVoxels(const std::string& fileName);
	std::shared_ptr<InstancedMesh> getMesh(const std::string& fileName);

	bool isLoaded(const std::string& fileName);
	bool isMeshed(const std::string& fileName);

private:
	std::shared_ptr<VoxelData> loadVoxels(const std::string& fileName);
	std::shared_ptr<InstancedMesh> loadMesh(const std::string& fileName);

	std::shared_ptr<Renderer> _renderer;
	std::map<std::string, std::shared_ptr<InstancedMesh>> _meshes;
	std::map<std::string, std::shared_ptr<VoxelData>> _voxels;
};

#endif // !VOXEL_FACTORY_H
