#include "VoxelFactory.h"
#include "VoxelLoader.h"

VoxelFactory::VoxelFactory(std::shared_ptr<Renderer> renderer) :
	_renderer(renderer)
{
}

VoxelFactory::~VoxelFactory()
{
}

void VoxelFactory::draw()
{
	for (auto pair : _meshes)
	{
		pair.second->draw();
	}
}

const unsigned int VoxelFactory::addInstance(const std::string & fileName)
{
	return getMesh(fileName)->addInstance();
}

std::shared_ptr<VoxelData> VoxelFactory::getVoxels(const std::string & fileName)
{
	auto pair = _voxels.find(fileName);
	if (pair != _voxels.end())
		return pair->second;
	return loadVoxels(fileName);
}

std::shared_ptr<InstancedMesh> VoxelFactory::getMesh(const std::string & fileName)
{
	auto pair = _meshes.find(fileName);
	if (pair != _meshes.end())
		return pair->second;
	return loadMesh(fileName);
}

bool VoxelFactory::isLoaded(const std::string & fileName)
{
	return (_voxels.find(fileName) != _voxels.end());
}

bool VoxelFactory::isMeshed(const std::string & fileName)
{
	return (_meshes.find(fileName) != _meshes.end());
}

std::shared_ptr<VoxelData> VoxelFactory::loadVoxels(const std::string & fileName)
{
	std::shared_ptr<VoxelData> data = VoxelLoader::load(fileName);
	if (data)
	{
		_voxels[fileName] = data;
	}
	return data;
}

std::shared_ptr<InstancedMesh> VoxelFactory::loadMesh(const std::string & fileName)
{
	std::shared_ptr<InstancedMesh> mesh = std::make_shared<InstancedMesh>(_renderer);
	getVoxels(fileName)->getMeshReduced(*mesh.get(), DEFAULT_VOXEL_MESHING_WIDTH);
	_meshes[fileName] = mesh;
	return mesh;
}
