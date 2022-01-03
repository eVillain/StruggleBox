#ifndef INSTANCED_MESH_H
#define INSTANCED_MESH_H

#include "Mesh.h"
#include <memory>
#include <map>
#include <vector>

class Allocator;
class Renderer;
class VertBuffer;

typedef unsigned int InstanceIDType;

class InstancedMesh : public Mesh
{
public:
	InstancedMesh(Renderer& renderer, Allocator& allocator);

	~InstancedMesh();

	const unsigned int addInstance(
		const glm::vec3& position = glm::vec3(0),
		const glm::quat& rotation = glm::quat(),
		const glm::vec3& scale = glm::vec3(1));

	void removeInstance(const InstanceIDType instanceID);

	void setPosition(const glm::vec3& position, const InstanceIDType instanceID);
	void setRotation(const glm::quat& rotation, const InstanceIDType instanceID);
	void setScale(const glm::vec3& scale, const InstanceIDType instanceID);
	void setInstance(const InstanceTransformData& data, InstanceIDType instanceID);
	const glm::vec3& getPosition(const InstanceIDType instanceID);
	const glm::quat& getRotation(const InstanceIDType instanceID);
	const glm::vec3& getScale(const InstanceIDType instanceID);
	const std::vector<InstanceIDType> getAllIDs();
	const size_t getInstanceCount() { return _instanceMap.size(); }

	void draw();

private:
	VertBuffer* _instanceBuffer;
	VertexData<InstanceTransformData> _instanceData;
	bool _dirtyInstances;
	unsigned int _nextInstanceID;
	std::map<InstanceIDType, unsigned int> _instanceMap;	// maps IDs to data positions
};

#endif // !INSTANCED_MESH_H
