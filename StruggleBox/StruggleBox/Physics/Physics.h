#ifndef PHYSICS_H
#define PHYSICS_H

#include "btBulletDynamicsCommon.h"
#include "PhysicsDebug.h"
#include <glm/glm.hpp>
#include <vector>

enum PhysicsMode {
    Physics_Off = 0,
    Physics_Rigid_Mesh = 1,
    Physics_Rigid_Blocks = 2,
    Physics_Dynamic_Mesh = 3,
    Physics_Dynamic_Blocks = 4,
    Physics_Dynamic_Hull = 5,
    Physics_Dynamic_AABBs = 6,
};
class btPairCachingGhostObject;

#define BIT(x) (1<<(x))
enum Collision_Types {
    Collision_None = 0,        //<Collide with nothing
    Collision_Terrain = BIT(0),    //<Collide with ground
    Collision_Entity = BIT(1),    //<Collide with entity
};

class Camera;

class Physics
{
public:
    static bool physicsCCD;             // Continuous collision detection
    static bool fixedTime;              // Use a fixed time step
    static int maxSubSteps;             // Maximum sub steps per physics frame
    static btScalar fixedTimeStep;      // Fixed timestep in seconds
    
    btDiscreteDynamicsWorld* dynamicsWorld;

    btCollisionShape* defaultBoxShape;  // Default box shape for voxels
    Physics();
    ~Physics();
    
    void Update( double delta );
    
    void AddStaticBox(const btVector3 & pos, const btVector3 & halfSize);
    void AddDynamicBox(const btVector3 & pos, const btVector3 & halfSize);
    void AddDynamicVoxel(const btVector3& pos);

    void Explosion( const btVector3& pos, const float radius, const float force );
    void AddImplosion( const btVector3& pos, const float radius, const float force );
    
    void SetRenderer( Renderer* renderer );
    
    static Physics* g_physics;
    static const glm::vec3 CameraCollisions(const glm::vec3& fromPos,
                                            const glm::vec3& toPos);
private:
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    
    double deltaAccumulator;            // Time accumulator
    PhysicsDebug* debugDraw;            // Debug draw interface
    
    std::vector<btPairCachingGhostObject*> explosions;
};


#endif
