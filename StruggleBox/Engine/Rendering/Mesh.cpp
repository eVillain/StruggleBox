#include "Mesh.h"

#include "Allocator.h"
#include "Renderer.h"
#include "VertBuffer.h"
#include "Log.h"

Mesh::Mesh(Renderer& renderer, Allocator& allocator) 
	: _renderer(renderer)
	, _vertBuffer(renderer.addVertBuffer(VertexDataType::MeshVerts))
	, _vertData(0, VertexDataType::MeshVerts, allocator)
	, _dirty(false)
{
	Log::Debug("[Mesh] Constructor, instance at: %p", this);
}

Mesh::~Mesh()
{
	Log::Debug("[Mesh] Destructor, instance at: %p", this);
}

void Mesh::buffer(MeshVertexData * data, unsigned int count)
{
	if (count > 0)
	{
		_vertData.buffer(data, count);
	}
	else
	{
		_vertData.clear();
	}
	_dirty = true;
}

void Mesh::resize(const unsigned int size)
{
	_vertData.resize(size);
	_dirty = true;
}

void Mesh::draw()
{
	//if (_dirty)
	//{
	//	_vertBuffer->bind();
	//	_vertBuffer->upload(
	//		_vertData.getData(),
	//		_vertData.getCount() * sizeof(MeshVertexData));
	//	_dirty = false;
	//}
	//_renderer.queueDeferredBuffer(
	//	_vertBuffer->getType(),
	//	_vertBuffer->getVBO(),
	//	_vertData.getCount());
}
