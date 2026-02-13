#pragma once

#include "VoxelData.h"
#include "Renderer3DDeferred.h"
#include <map>
#include <string>

class Allocator;

const float DEFAULT_VOXEL_MESHING_WIDTH = 0.25;

class VoxelFactory
{
public:
	VoxelFactory(Renderer3DDeferred& renderer, Allocator& allocator);
	~VoxelFactory();

	void draw();

	const unsigned int addInstance(const std::string& fileName);
	VoxelData* getVoxels(const std::string& fileName);
	DrawDataID getMesh(const std::string& fileName);

	bool isLoaded(const std::string& fileName);
	bool isMeshed(const std::string& fileName);

private:
	VoxelData* loadVoxels(const std::string& fileName);
	DrawDataID loadMesh(const std::string& fileName);

	Renderer3DDeferred& m_renderer;
	Allocator& m_allocator;

	std::map<std::string, DrawDataID> m_meshes;
	std::map<std::string, VoxelData*> m_voxels;
	std::map<std::string, std::vector<InstanceTransformData>> m_instances;
};
