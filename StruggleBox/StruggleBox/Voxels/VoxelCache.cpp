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
	for (const auto& pair : m_data)
	{
		const VoxelCacheData& data = pair.second;
		ColoredInstanceTransform3DData* instances = m_renderer.bufferVoxelMeshInstances(data.instances.size(), data.drawDataID);
		for (const auto& instancePair : data.instances)
		{
			const VoxelInstanceID instanceID = instancePair.first;
			const ColoredInstanceTransform3DData& instance = instancePair.second;
			memcpy(&instances[instanceID], &instance, sizeof(InstanceTransformData3D));
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
	data.instances[data.nextInstanceID] = { glm::vec3(), glm::vec3(1,1,1), glm::quat(), COLOR_WHITE };
	return data.nextInstanceID;
}

const VoxelInstanceID VoxelCache::addInstance(const std::string& fileName, const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rot, const Color& color)
{
	auto it = m_data.find(fileName);
	if (it == m_data.end())
	{
		load(fileName);
	}
	VoxelCacheData& data = m_data[fileName];
	data.nextInstanceID++;
	data.instances[data.nextInstanceID] = { pos, scale, rot, color };
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
	const size_t numVoxels = voxelData->getSizeX() * voxelData->getSizeY() * voxelData->getSizeZ();
	VoxelMeshPBRVertexData* tempVerts = (VoxelMeshPBRVertexData*)m_allocator.allocate(sizeof(VoxelMeshPBRVertexData) * numVoxels * 36);
	size_t vertexCount = 0;
	voxelData->createTriangleMeshReduced(tempVerts, vertexCount, DEFAULT_VOXEL_MESHING_WIDTH, glm::vec3());
	VoxelMeshPBRVertexData* verts = m_renderer.bufferVoxelMeshVerts(vertexCount, drawDataID);
	memcpy(verts, tempVerts, sizeof(VoxelMeshPBRVertexData) * vertexCount);
	m_allocator.deallocate(tempVerts);
	m_data[fileName] = {drawDataID, voxelData, 0, {}};
}
