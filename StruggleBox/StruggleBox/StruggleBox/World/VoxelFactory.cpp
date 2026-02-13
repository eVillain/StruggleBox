#include "VoxelFactory.h"

#include "Allocator.h"
#include "ArenaOperators.h"
#include "VoxelLoader.h"

VoxelFactory::VoxelFactory(Renderer3DDeferred& renderer, Allocator& allocator)
	: m_renderer(renderer)
	, m_allocator(allocator)
{
}

VoxelFactory::~VoxelFactory()
{

}

void VoxelFactory::draw()
{
	//for (auto pair : m_meshes)
	//{
	//	pair.second->draw();
	//}
}

const unsigned int VoxelFactory::addInstance(const std::string& fileName)
{
	auto it = m_instances.find(fileName);
	if (it == m_instances.end())
	{
		m_instances[fileName] = {};
	}
	return getMesh(fileName)->addInstance();
}

VoxelData* VoxelFactory::getVoxels(const std::string& fileName)
{
	auto pair = m_voxels.find(fileName);
	if (pair != m_voxels.end())
		return pair->second;
	return loadVoxels(fileName);
}

DrawDataID VoxelFactory::getMesh(const std::string& fileName)
{
	auto pair = m_meshes.find(fileName);
	if (pair != m_meshes.end())
		return pair->second;
	return loadMesh(fileName);
}

bool VoxelFactory::isLoaded(const std::string& fileName)
{
	return (m_voxels.find(fileName) != m_voxels.end());
}

bool VoxelFactory::isMeshed(const std::string& fileName)
{
	return (m_meshes.find(fileName) != m_meshes.end());
}

VoxelData*VoxelFactory::loadVoxels(const std::string& fileName)
{
	VoxelData* data = VoxelLoader::load(fileName, m_allocator);
	if (data)
	{
		m_voxels[fileName] = data;
	}
	return data;
}

DrawDataID VoxelFactory::loadMesh(const std::string& fileName)
{
	DrawDataID drawDataID = m_renderer.getInstanceDrawData(fileName);	
	VoxelData* data = getVoxels(fileName);
	uint32_t vertCount = 0;
	data->createTriangleMesh(m_renderer, drawDataID, DEFAULT_VOXEL_MESHING_WIDTH);
	m_meshes[fileName] = drawDataID;
	return drawDataID;
}
