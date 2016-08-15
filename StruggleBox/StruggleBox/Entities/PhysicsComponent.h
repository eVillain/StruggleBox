#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H

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

class PhysicsComponent : public EntityComponent
{
public:    
    std::vector<Entity*> collisionFilter;

	PhysicsComponent(
		const int ownerID,
		std::shared_ptr<EntityManager> manager,
		std::shared_ptr<Physics> physics,
		std::shared_ptr<VoxelFactory> voxels);
    ~PhysicsComponent();

    void update(const double delta);

	void setPosition(const glm::vec3& position);
	void setRotation(const glm::quat& rotation);

    void clearPhysics();

    void setPhysicsMode(const int newMode,
                        const bool trigger = false);

	void setLinearVelocity( const btVector3* newLinVel );

    void setAngularVelocity( const btVector3* newAngVel );

    btRigidBody* getRigidBody() { return physicsMeshBody; };

    void addContactToFilter(Entity*newContact);
    
	std::shared_ptr<EntityManager> _manager;	// UGLY but right now its needed for the contact sensor callback
private:
	std::shared_ptr<Physics> _physics;
	std::shared_ptr<VoxelFactory> _voxels;

    btTriangleMesh* physicsMeshTris;                        // Physics triangles array
    btBvhTriangleMeshShape* physicsMeshShape;               // Physics mesh shape
    btRigidBody* physicsMeshBody;                           // Physics mesh body
    btCompoundShape* physicsCompShape;                      // Physics cubes compound shape
    btConvexHullShape* physicsHullShape;                    // Physics cubes hull shape
    
 //   Cubeject* object;
    double timeAccumulator;
};

#endif /* PHYSICS_COMPONENT_H */
