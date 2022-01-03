#pragma once

#include "EntityComponent.h"
#include <vector>

class EntityManager;
class Physics;
class VoxelFactory;

class btTriangleMesh;
class btBvhTriangleMeshShape;
class btCompoundShape;
class btConvexHullShape;
class btRigidBody;
class btVector3;
class btCollisionShape;

enum class PhysicsMode {
    Physics_Off = 0,
    Physics_Cube_Mesh = 1,
    Physics_Cube_Blocks = 2,
    Physics_Cube_Hull = 3,
    Physics_Cube_AABBs = 4,
    Physics_Cube_Single = 5,
    Physics_Sphere = 6
};

class PhysicsComponent : public EntityComponent
{
public:    
    std::vector<Entity*> collisionFilter;

	PhysicsComponent(
		const int ownerID,
		EntityManager& manager,
		Physics& physics,
		VoxelFactory& voxels);
    ~PhysicsComponent();

    void update(const double delta);

	void setPosition(const glm::vec3& position);
	void setRotation(const glm::quat& rotation);

    void clearPhysics();

    void setPhysicsMode(PhysicsMode newMode, bool isStatic, bool trigger = false);

	void setLinearVelocity( const btVector3* newLinVel );

    void setAngularVelocity( const btVector3* newAngVel );

    btRigidBody* getRigidBody() { return m_body; };

    void addContactToFilter(Entity*newContact);
    
	EntityManager& _manager;	// UGLY but right now its needed for the contact sensor callback

private:
	Physics& m_physics;
	VoxelFactory& m_voxels;

    uint32_t m_shapeID;
    btCollisionShape* m_shape;
    uint32_t m_bodyID;
    btRigidBody* m_body;

    bool m_isStatic;
    bool m_isSensor;
    double m_timeAccumulator;

    void createShape(const PhysicsMode mode);
    void createBody(const bool isStatic);
    static bool requiresVoxelFile(PhysicsMode mode);
};
