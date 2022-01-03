#include "VoxelFactory.h"

#include "Allocator.h"
#include "VoxelLoader.h"

VoxelFactory::VoxelFactory(Renderer& renderer, Allocator& allocator)
	: _renderer(renderer)
	, m_allocator(allocator)
{
}

VoxelFactory::~VoxelFactory()
{
	for (auto pair : _meshes)
	{
		CUSTOM_DELETE(pair.second, m_allocator);
	}
}

void VoxelFactory::draw()
{
	for (auto pair : _meshes)
	{
		pair.second->draw();
	}
}

const unsigned int VoxelFactory::addInstance(const std::string& fileName)
{
	return getMesh(fileName)->addInstance();
}

VoxelData* VoxelFactory::getVoxels(const std::string& fileName)
{
	auto pair = _voxels.find(fileName);
	if (pair != _voxels.end())
		return pair->second;
	return loadVoxels(fileName);
}

InstancedMesh* VoxelFactory::getMesh(const std::string& fileName)
{
	auto pair = _meshes.find(fileName);
	if (pair != _meshes.end())
		return pair->second;
	return loadMesh(fileName);
}

bool VoxelFactory::isLoaded(const std::string& fileName)
{
	return (_voxels.find(fileName) != _voxels.end());
}

bool VoxelFactory::isMeshed(const std::string& fileName)
{
	return (_meshes.find(fileName) != _meshes.end());
}

VoxelData*VoxelFactory::loadVoxels(const std::string& fileName)
{
	VoxelData* data = VoxelLoader::load(fileName, m_allocator);
	if (data)
	{
		_voxels[fileName] = data;
	}
	return data;
}

InstancedMesh* VoxelFactory::loadMesh(const std::string& fileName)
{
	InstancedMesh* mesh = CUSTOM_NEW(InstancedMesh, m_allocator)(_renderer, m_allocator);
	VoxelData* data = getVoxels(fileName);
	data->getMeshLinear(*mesh, DEFAULT_VOXEL_MESHING_WIDTH);
	_meshes[fileName] = mesh;
	return mesh;
}
