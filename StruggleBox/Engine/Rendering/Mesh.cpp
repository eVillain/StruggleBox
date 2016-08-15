#include "Mesh.h"
#include "Renderer.h"
#include "VertBuffer.h"
#include "Log.h"

Mesh::Mesh(std::shared_ptr<Renderer> renderer) :
	_renderer(renderer),
	_vertBuffer(nullptr),
	_dirty(false),
	_vertData(0)
{
	Log::Debug("[Mesh] Constructor, instance at: %p", this);
	_vertBuffer = _renderer->addVertBuffer(MeshVerts);
}

Mesh::~Mesh()
{
	Log::Debug("[Mesh] Destructor, instance at: %p", this);
}

void Mesh::buffer(
	MeshVertexData * data,
	unsigned int count)
{
	_vertData.buffer(data, count);
	_dirty = true;
}

void Mesh::resize(const unsigned int size)
{
	_vertData.resize(size);
	_dirty = true;
}

void Mesh::draw()
{
	if (_dirty)
	{
		_vertBuffer->bind();
		_vertBuffer->upload(
			_vertData.getData(),
			_vertData.getCount() * sizeof(MeshVertexData));
		_dirty = false;
	}
	_renderer->queueDeferredBuffer(
		_vertBuffer->getType(),
		_vertBuffer->getVBO(),
		_vertData.getCount());
}
