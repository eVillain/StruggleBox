#ifndef MESH_H
#define MESH_H

#include "GFXDefines.h"
#include "Transform.h"
#include "VertexData.h"
#include <memory>

class Allocator;
class Renderer;
class VertBuffer;

class Mesh
{
public:
	Mesh(Renderer& renderer, Allocator& allocator);

	~Mesh();

	virtual void buffer(
		MeshVertexData* data,
		unsigned int count);

	virtual void resize(const unsigned int count);

	virtual void draw();

protected:
	Renderer& _renderer;
	VertBuffer* _vertBuffer;

	VertexData<MeshVertexData> _vertData;
	bool _dirty = false;
	Transform _transform;
};

#endif