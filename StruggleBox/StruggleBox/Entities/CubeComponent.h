#ifndef CUBE_COMPONENT_H
#define CUBE_COMPONENT_H

#include "EntityComponent.h"

class EntityManager;
class VoxelFactory;
class InstancedMesh;

class CubeComponent : public EntityComponent
{
public:
    CubeComponent(
		const int ownerID,
		const std::string& objectName,
		std::shared_ptr<EntityManager> manager,
		std::shared_ptr<VoxelFactory> voxels);

    virtual ~CubeComponent();

    void loadObject();
    void unloadObject();

    virtual void update(const double delta);
    
private:
	std::shared_ptr<EntityManager> _manager;
	std::shared_ptr<VoxelFactory> _voxels;
	std::shared_ptr<InstancedMesh> _mesh;
	int _instanceID;
};

#endif
