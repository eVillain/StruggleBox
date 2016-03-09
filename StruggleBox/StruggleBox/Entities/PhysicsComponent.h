//
//  PhysicsComponent.h
//  Bloxelizer
//
//  Created by The Drudgerist on 8/29/13.
//
//

#ifndef BWO_PHYSICS_COMPONENT_H
#define BWO_PHYSICS_COMPONENT_H
#include "EntityComponent.h"
#include <vector>
class EntityManager;
class Cubeject;
class btTriangleMesh;
class btBvhTriangleMeshShape;
class btCompoundShape;
class btConvexHullShape;
class btRigidBody;
class btVector3;

class PhysicsComponent : public EntityComponent {
    int physicsMode;                                        // 0 = off, 1 = static, 2 = dynamic
    btTriangleMesh* physicsMeshTris;                        // Physics triangles array
    btBvhTriangleMeshShape* physicsMeshShape;               // Physics mesh shape
    btRigidBody* physicsMeshBody;                           // Physics mesh body
    btCompoundShape* physicsCompShape;                      // Physics cubes compound shape
    btConvexHullShape* physicsHullShape;                    // Physics cubes hull shape
    
    Cubeject* object;
    double timeAccumulator;
public:
    EntityManager* m_manager;
    const int GetOwnerID() { return m_ownerID; };
    
    std::vector<Entity*> collisionFilter;

    PhysicsComponent( const int ownerID, EntityManager* manager );
    ~PhysicsComponent();
    void Update( double delta );
    void ClearPhysics();
    void SetPhysicsMode( const int newMode, const bool trigger = false );
	void SetLinearVelocity( const btVector3* newLinVel );
    void SetAngularVelocity( const btVector3* newAngVel );
    btRigidBody* GetRigidBody() { return physicsMeshBody; };
    void AddContactToFilter(Entity*newContact);
};

#endif
