#pragma once

#include "EntityComponent.h"

class EntityManager;
class VoxelFactory;
class InstancedMesh;

class VoxelComponent : public EntityComponent
{
public:
    VoxelComponent(
		const int ownerID,
		const std::string& objectName,
		EntityManager& manager,
		VoxelFactory& voxels);

    virtual ~VoxelComponent();

    void loadObject();
    void unloadObject();

    virtual void update(const double delta);
    
private:
	EntityManager& _manager;
	VoxelFactory& _voxels;
	InstancedMesh* _mesh;
	int _instanceID;
};

