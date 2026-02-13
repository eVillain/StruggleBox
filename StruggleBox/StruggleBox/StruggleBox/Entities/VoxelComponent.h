#pragma once

#include "EntityComponent.h"

class EntityManager;
class VoxelCache;
class InstancedTriangleMesh;

class VoxelComponent : public EntityComponent
{
public:
    VoxelComponent(
		const int ownerID,
		const std::string& objectName,
		EntityManager& manager,
		VoxelCache& voxels);

    virtual ~VoxelComponent();

    void loadObject();
    void unloadObject();

    virtual void update(const double delta);
    
private:
	EntityManager& _manager;
	VoxelCache& _voxels;
	int _instanceID;
};

