#include "VoxelCache.h"

#include "Allocator.h"
#include "VoxelLoader.h"
#include "VoxelRenderer.h"

VoxelCache::VoxelCache(VoxelRenderer& renderer, Allocator& allocator)
	: m_renderer(renderer)
	, m_allocator(allocator)
{
}

void VoxelCache::draw()
{
	for (const auto pair : m_data)
	{
		DrawParameters drawParams;
		drawParams.textureCount = 0;
		//drawParams.shaderID = m_voxelInstancesShaderID;
		drawParams.blendMode = BLEND_MODE_DISABLED;
		drawParams.depthMode = DEPTH_MODE_DEFAULT;
		const VoxelCacheData& data = pair.second;
		//ColoredInstanceTransform3DData* instances = m_renderer.bufferVoxelMeshInstances(data.instances.size(), data.drawDataID);
		//uint32_t instanceID = 0;
		for (auto it = data.instances.begin(); it != data.instances.end(); it++)
		{
			//const InstanceTransformData3D& instance = it->second;
			//memcpy(&instances[instanceID], &instance, sizeof(InstanceTransformData3D));
		}
	}
}

const VoxelInstanceID VoxelCache::addInstance(const std::string& fileName)
{
	auto it = m_data.find(fileName);
	if (it == m_data.end())
	{
		load(fileName);
	}
	VoxelCacheData& data = m_data[fileName];
	data.nextInstanceID++;
	data.instances[data.nextInstanceID] = {};
	return data.nextInstanceID;
}

void VoxelCache::removeInstance(const std::string& fileName, const VoxelInstanceID instanceID)
{
	auto it = m_data.find(fileName);
	if (it == m_data.end())
	{
		return;
	}
	VoxelCacheData& data = m_data[fileName];
	data.instances.erase(instanceID);
	// TODO: Clean up voxel data if there are no instances left
}

ColoredInstanceTransform3DData* VoxelCache::getInstance(const std::string& fileName, const VoxelInstanceID instanceID)
{
	auto it = m_data.find(fileName);
	if (it == m_data.end())
	{
		return nullptr;
	}
	VoxelCacheData& data = m_data[fileName];
	if (data.instances.find(instanceID) == data.instances.end())
	{
		return nullptr;
	}
	return &data.instances[instanceID];
}

const VoxelData* VoxelCache::getVoxelData(const std::string& fileName)
{
	auto it = m_data.find(fileName);
	if (it == m_data.end())
	{
		load(fileName);
	}
	VoxelCacheData& data = m_data[fileName];
	return data.voxelData;
}

void VoxelCache::load(const std::string& fileName)
{
	DrawDataID drawDataID = m_renderer.createVoxelMeshDrawData();
	VoxelData* voxelData = VoxelLoader::load(fileName, m_allocator);
	voxelData->createTriangleMesh(m_renderer, drawDataID, DEFAULT_VOXEL_MESHING_WIDTH);
	m_data[fileName] = {drawDataID, voxelData, 0, {}};
}
