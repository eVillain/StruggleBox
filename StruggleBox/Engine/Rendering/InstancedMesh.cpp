#include "InstancedMesh.h"
#include "Renderer.h"
#include "VertBuffer.h"
#include "Log.h"

InstancedMesh::InstancedMesh(Renderer& renderer, Allocator& allocator)
	: Mesh(renderer, allocator)
	, _instanceBuffer(renderer.addVertBuffer(VertexDataType::InstanceVerts))
	, _dirtyInstances(false)
	, _instanceData(0, VertexDataType::InstanceVerts, allocator)
	, _nextInstanceID(1)
{
	Log::Debug("[InstancedMesh] Constructor, instance at: %p", this);
}

InstancedMesh::~InstancedMesh()
{
}

const unsigned int InstancedMesh::addInstance(
	const glm::vec3& position,
	const glm::quat& rotation,
	const glm::vec3& scale)
{
	// Prepare storage
	if (_instanceData.getSize() == _instanceData.getCount())
	{
		const unsigned int previousStorageSize = _instanceData.getSize();
		unsigned int newStorageSize = previousStorageSize ? previousStorageSize * 2 : 1;
		_instanceData.resize(newStorageSize, true);
		Log::Debug("[InstancedMesh] resizing storage from %i to %i instances", previousStorageSize, newStorageSize);
	}
	InstanceIDType instanceID = _nextInstanceID++;
	InstanceTransformData instance = {
		position, rotation, scale
	};
	_instanceData.buffer(&instance, 1);
	_instanceMap[instanceID] = _instanceData.getCount()-1;
	Log::Debug("[InstancedMesh] added instance %i mapped to %i",
		instanceID, _instanceMap[instanceID]);
	_dirtyInstances = true;
	return instanceID;
}

void InstancedMesh::removeInstance(const InstanceIDType instanceID)
{
	if (_instanceMap.find(instanceID) == _instanceMap.end())
	{
		Log::Error("[InstancedMesh] failed to remove %i, instance not found!",
			instanceID);
		return;
	}
	const unsigned int removedDataIndex = _instanceMap[instanceID];
	const unsigned int lastDataIndex = _instanceData.getCount() - 1;

	_instanceMap.erase(instanceID);
	
	if (removedDataIndex == lastDataIndex)
	{
		_instanceData.remove(lastDataIndex);
		return;
	}

	for (auto pair : _instanceMap)
	{
		if (pair.second == lastDataIndex)
		{
			const unsigned int lastDataID = pair.first;
			_instanceMap[lastDataID] = removedDataIndex;
			_instanceData.remove(lastDataIndex);

			Log::Debug("[InstancedMesh] removed %i(%i), moved %i(%i)",
				instanceID, removedDataIndex, lastDataID, lastDataIndex);
			_dirtyInstances = true;
			return;
		}
	}
	Log::Error("[InstancedMesh] failed to remove %i(%i), couldn't move from %i",
		instanceID, removedDataIndex, lastDataIndex);
}

void InstancedMesh::setPosition(const glm::vec3 & position, const InstanceIDType instanceID)
{
	_instanceData.getData()[_instanceMap[instanceID]].position = position;
	_dirtyInstances = true;
}

void InstancedMesh::setRotation(const glm::quat & rotation, const InstanceIDType instanceID)
{
	_instanceData.getData()[_instanceMap[instanceID]].rotation = rotation;
	_dirtyInstances = true;
}

void InstancedMesh::setScale(const glm::vec3 & scale, const InstanceIDType instanceID)
{
	_instanceData.getData()[_instanceMap[instanceID]].scale = scale;
	_dirtyInstances = true;
}

void InstancedMesh::setInstance(const InstanceTransformData & data, const InstanceIDType instanceID)
{
	_instanceData.getData()[_instanceMap[instanceID]] = data;
	_dirtyInstances = true;
}

const glm::vec3 & InstancedMesh::getPosition(const InstanceIDType instanceID)
{
	return _instanceData.getData()[_instanceMap[instanceID]].position;
}

const glm::quat & InstancedMesh::getRotation(const InstanceIDType instanceID)
{
	return _instanceData.getData()[_instanceMap[instanceID]].rotation;
}

const glm::vec3 & InstancedMesh::getScale(const InstanceIDType instanceID)
{
	return _instanceData.getData()[_instanceMap[instanceID]].scale;
}

const std::vector<InstanceIDType> InstancedMesh::getAllIDs()
{
	std::vector<InstanceIDType> allIDs;
	for (auto pair : _instanceMap)
	{
		allIDs.push_back(pair.first);
	}
	return allIDs;
}

void InstancedMesh::draw()
{
	if (_vertData.getCount() == 0 || _instanceData.getCount() == 0)
		return;

	if (_dirty)
	{
		_vertBuffer->bind();
		_vertBuffer->upload(
			_vertData.getData(),
			_vertData.getCount() * sizeof(MeshVertexData));
		_dirty = false;
	}
	if (_dirtyInstances)
	{
		_instanceBuffer->bind();
		_instanceBuffer->upload(
			_instanceData.getData(),
			_instanceMap.size() * sizeof(InstanceTransformData));
		_dirtyInstances = false;
	}

	_renderer.queueDeferredInstances(
		_instanceBuffer->getVBO(),
		_instanceMap.size(),
		_vertBuffer->getType(),
		_vertBuffer->getVBO(),
		0,
		_vertData.getCount(),
		0,
		BlendMode{ 0, 0, false },
		DepthMode{ true, true });
}
