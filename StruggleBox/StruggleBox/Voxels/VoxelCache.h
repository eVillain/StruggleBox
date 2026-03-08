#pragma once

#include "VoxelData.h"
#include <map>
#include <string>

class VoxelRenderer;

const float DEFAULT_VOXEL_MESHING_WIDTH = 0.25;
typedef uint32_t VoxelInstanceID;


struct VoxelCacheData
{
	DrawDataID drawDataID;
	VoxelData* voxelData;
	VoxelInstanceID nextInstanceID;
	std::map<VoxelInstanceID, ColoredInstanceTransform3DData> instances;
};

class VoxelCache
{
public:
	VoxelCache(VoxelRenderer& renderer, Allocator& allocator);

	void draw();

	const VoxelInstanceID addInstance(const std::string& fileName);
	const VoxelInstanceID addInstance(const std::string& fileName, const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot, const Color& color);
	void removeInstance(const std::string& fileName, const VoxelInstanceID instanceID);
	ColoredInstanceTransform3DData* getInstance(const std::string& fileName, const VoxelInstanceID instanceID);

	const VoxelData* getVoxelData(const std::string& fileName);

private:
	void load(const std::string& fileName);

	VoxelRenderer& m_renderer;
	Allocator& m_allocator;
	std::map<std::string, VoxelCacheData> m_data;
};

