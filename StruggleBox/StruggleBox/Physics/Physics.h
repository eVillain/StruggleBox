#pragma once

#include "btBulletDynamicsCommon.h"
#include "PhysicsDebug.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <functional>

class Allocator;
class PhysicsCube;

class btPairCachingGhostObject;

#define BIT(x) (1<<(x))
enum class CollisionType {
    Group_Terrain = BIT(1), // Collide with ground
    Group_Entity = BIT(2),  // Collide with entity
    Group_Camera = BIT(3),  // Camera scan collider
    Group_Fireball = BIT(4),  // Collide with fireball - todo: just for testing, move to entity
    //Filter_None = 0,
    Filter_Terrain_Entity = Group_Terrain | Group_Entity,
    Filter_Everything = Group_Terrain | Group_Entity | Group_Camera | Group_Fireball
};


class Physics
{
public:
    Physics(Allocator& allocator);
    ~Physics();
    
    void Update( double delta );
    
    uint32_t createBox(const float sizeX, const float sizeY, const float sizeZ);
    uint32_t createCube(const float size);
    uint32_t createSphere(const float size);
    uint32_t createCapsule(const float radius, const float height);
    uint32_t createCompountShape();
    uint32_t createTriangleMeshShape(btTriangleMesh* mesh, bool useQuantizedAABBs);
    uint32_t createConvexHullShape();

    btTriangleMesh* createTriangleMesh();
    void destroyTriangleMesh(btTriangleMesh* mesh);
    btPairCachingGhostObject* createGhostObject();
    void destroyGhostObject(btPairCachingGhostObject* object);
    btKinematicCharacterController* createCharacterController(btPairCachingGhostObject* object, btConvexShape* shape, float stepHeight);
    void destroyCharacterController(btKinematicCharacterController* controller);

    btCollisionShape* getShapeForID(uint32_t shapeID);

    uint32_t createBody(btScalar mass, btCollisionShape* collisionShape, const btVector3& localInertia);
    btRigidBody* getBodyForID(uint32_t bodyID);

    void addBodyToWorld(btRigidBody* body, CollisionType group, CollisionType mask);
    void removeBodyFromWorld(btRigidBody* body);

    void removeShape(uint32_t shapeID);
    void removeBody(uint32_t bodyID);

    void Explosion( const btVector3& pos, const float radius, const float force );
    
    void SetRenderer( Renderer* renderer );
    
    btDiscreteDynamicsWorld* getWorld() { return m_dynamicsWorld; }
    bool getIsUsingCCD() const { return m_physicsCCD; }

    glm::vec3 cameraCollision(const glm::vec3& fromPos, const glm::vec3& toPos);

    void setCollisionCB(const std::function<void(void*, void*, const glm::vec3&, float)>& cb) { m_collisionCallback = cb; }

private:
    Allocator& m_allocator;
    static Physics* s_instance;

    btBroadphaseInterface* m_broadphase;
    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;

    bool m_physicsCCD;             // Continuous collision detection
    bool m_fixedTime;              // Use a fixed time step
    int m_maxSubSteps;             // Maximum sub steps per physics frame
    btScalar m_fixedTimeStep;      // Fixed timestep in seconds
    double m_deltaAccumulator;     // Time accumulator
    PhysicsDebug* m_debugDraw;     // Debug draw interface
    
    uint32_t m_nextShapeID;
    uint32_t m_nextBodyID;

    std::map<uint32_t, btCollisionShape*> m_shapes;
    std::map<uint32_t, btRigidBody*> m_bodies;
    //std::vector<btPairCachingGhostObject*> m_explosions;

    btCollisionShape* m_defaultBoxShape;  // Default box shape for voxels TODO: Move out

    std::function<void(void*, void*, const glm::vec3&, float)> m_collisionCallback;

    void initialize();
    void terminate();

    static void internalTick(btDynamicsWorld* world, btScalar timeStep);
    static void* physics_alloc(size_t size);
    static void physics_dealloc(void* memblock);
};
