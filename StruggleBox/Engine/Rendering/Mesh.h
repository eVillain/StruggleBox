#ifndef MESH_H
#define MESH_H

#include "GFXDefines.h"
#include "Transform.h"
#include "VertexData.h"
#include <memory>

class Renderer;
class VertBuffer;

class Mesh
{
public:
	Mesh(std::shared_ptr<Renderer> renderer);

	~Mesh();

	virtual void buffer(
		MeshVertexData* data,
		unsigned int count);

	virtual void resize(const unsigned int count);

	virtual void draw();

protected:
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<VertBuffer> _vertBuffer;

	VertexData<MeshVertexData> _vertData;
	bool _dirty = false;
	Transform _transform;
};

#endif