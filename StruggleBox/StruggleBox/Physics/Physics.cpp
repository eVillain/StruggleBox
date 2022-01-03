#include "Physics.h"

#include "ArenaOperators.h"
#include "Allocator.h"
#include "CollisionDispatcher.h"
#include "Log.h"
#include "PhysicsCube.h"

#include "btGhostObject.h"
#include <iostream>

Physics* Physics::s_instance = nullptr;

Physics::Physics(Allocator& allocator)
    : m_allocator(allocator)
    , m_broadphase(nullptr)
    , m_collisionConfiguration(nullptr)
    , m_dispatcher(nullptr)
    , m_solver(nullptr)
    , m_dynamicsWorld(nullptr)
    , m_physicsCCD(true)
    , m_fixedTime(false)
    , m_maxSubSteps(2)
    , m_fixedTimeStep(btScalar(1.f) / btScalar(60.f))
    , m_deltaAccumulator(0.0)
    , m_debugDraw(nullptr)
    , m_nextShapeID(0)
    , m_nextBodyID(0)
{
	Log::Info("[Physics] constructor, instance at %p", this);
    s_instance = this;

    initialize();

    m_debugDraw = CUSTOM_NEW(PhysicsDebug, m_allocator)();
    m_debugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    m_dynamicsWorld->setDebugDrawer(m_debugDraw);
    
    uint32_t defBoxID = createCube(0.25f);
    m_defaultBoxShape = getShapeForID(defBoxID);
    //const btScalar mass = 1;
    //btVector3 fallInertia(0, 0, 0);
    //m_defaultBoxShape->calculateLocalInertia(mass, fallInertia);
}

Physics::~Physics()
{
    terminate();
    btAlignedAllocSetCustom(nullptr, nullptr);
    s_instance = nullptr;
}

void Physics::Update(double delta) 
{
    if (!m_dynamicsWorld)
    {
        return;
    }
    if (m_physicsCCD != m_dynamicsWorld->getDispatchInfo().m_useContinuous)
    {
        m_dynamicsWorld->getDispatchInfo().m_useContinuous = m_physicsCCD;
    }
    if (m_fixedTime)
    {
        m_deltaAccumulator += delta;
        while (m_deltaAccumulator > m_fixedTimeStep)
        {
            m_dynamicsWorld->stepSimulation(m_fixedTimeStep, m_maxSubSteps, m_fixedTimeStep);
            m_deltaAccumulator -= m_fixedTimeStep;
        }
    }
    else
    {
        m_dynamicsWorld->stepSimulation(delta, m_maxSubSteps, m_fixedTimeStep);
    }

    // Update explosions
    //for (btPairCachingGhostObject* ghostObject : m_explosions)
    //{
    //    btSphereShape* explosionSphere = (btSphereShape*)ghostObject->getCollisionShape();
    //    btVector3 minAabb, maxAabb;
    //    explosionSphere->getAabb(ghostObject->getWorldTransform(), minAabb, maxAabb);
    //    m_dynamicsWorld->getBroadphase()->setAabb(ghostObject->getBroadphaseHandle(),
    //        minAabb, maxAabb,
    //        m_dispatcher);

    //    m_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghostObject->getOverlappingPairCache(),
    //        m_dynamicsWorld->getDispatchInfo(),
    //        m_dynamicsWorld->getDispatcher());

    //    btManifoldArray   manifoldArray;
    //    btBroadphasePairArray& pairArray = ghostObject->getOverlappingPairCache()->getOverlappingPairArray();
    //    int numPairs = pairArray.size();

    //    for (int i = 0; i < numPairs; i++)
    //    {
    //        manifoldArray.clear();

    //        const btBroadphasePair& pair = pairArray[i];

    //        //unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
    //        btBroadphasePair* collisionPair = m_dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
    //        if (!collisionPair)
    //            continue;

    //        if (collisionPair->m_algorithm)
    //            collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

    //        for (int j = 0; j < manifoldArray.size(); j++)
    //        {
    //            btPersistentManifold* manifold = manifoldArray[j];
    //            //                    btScalar directionSign = manifold->getBody0() == ghostObject ? btScalar(-1.0) : btScalar(1.0);
    //            for (int p = 0; p < manifold->getNumContacts(); p++)
    //            {
    //                const btManifoldPoint& pt = manifold->getContactPoint(p);
    //                if (pt.getDistance() < 0.f)
    //                {
    //                    const btVector3& ptA = pt.getPositionWorldOnA();
    //                    const btVector3& ptB = pt.getPositionWorldOnB();
    //                    const btVector3 diff = ptA - ptB;
    //                    //                            const btVector3& normalOnB = pt.m_normalWorldOnB;
    //                                                // EXPLOSION CALCULATION
    //                    const btScalar dist = diff.length();
    //                    const btScalar radius = explosionSphere->getRadius();
    //                    if (dist < radius)
    //                    {
    //                        int bodyType = manifold->getBody1()->getInternalType();
    //                        if (bodyType == btCollisionObject::CO_RIGID_BODY)
    //                        {
    //                            btRigidBody* body = (btRigidBody*)manifold->getBody1();
    //                            float impulseF = ((radius - dist) / radius);
    //                            btVector3 impulse = ptA.normalized() * impulseF;
    //                            body->applyImpulse(impulse, ptB);
    //                        }
    //                    }
    //                    //else
    //                    //{
    //                    //    Log::Debug("Explosion distance bigger than radius\n");
    //                    //}
    //                }
    //            }
    //        }
    //    }
    //}
    //m_explosions.clear();
}

uint32_t Physics::createBox(const float sizeX, const float sizeY, const float sizeZ)
{
    btBoxShape* box = CUSTOM_NEW(btBoxShape, m_allocator)(btVector3(sizeX, sizeY, sizeZ));
    m_nextShapeID++;
    m_shapes[m_nextShapeID] = box;
    return m_nextShapeID;
}

uint32_t Physics::createCube(const float size)
{
    return createBox(size, size, size);
}

uint32_t Physics::createSphere(const float size)
{
    btSphereShape* sphere = CUSTOM_NEW(btSphereShape, m_allocator)(size);
    m_nextShapeID++;
    m_shapes[m_nextShapeID] = sphere;
    return m_nextShapeID;
}

uint32_t Physics::createCapsule(const float radius, const float height)
{
    btCapsuleShape* capsule = CUSTOM_NEW(btCapsuleShape, m_allocator)(radius, height);
    m_nextShapeID++;
    m_shapes[m_nextShapeID] = capsule;
    return m_nextShapeID;
}

uint32_t Physics::createCompountShape()
{
    btCompoundShape* shape = CUSTOM_NEW(btCompoundShape, m_allocator)();
    m_nextShapeID++;
    m_shapes[m_nextShapeID] = shape;
    return m_nextShapeID;
}

uint32_t Physics::createTriangleMeshShape(btTriangleMesh* mesh, bool useQuantizedAABBs)
{
    btBvhTriangleMeshShape* shape = CUSTOM_NEW(btBvhTriangleMeshShape, m_allocator)(mesh, useQuantizedAABBs);
    m_nextShapeID++;
    m_shapes[m_nextShapeID] = shape;
    return m_nextShapeID;
}

void Physics::destroyTriangleMesh(btTriangleMesh* mesh)
{
    CUSTOM_DELETE(mesh, m_allocator);
}

btPairCachingGhostObject* Physics::createGhostObject()
{
    btPairCachingGhostObject* object = CUSTOM_NEW(btPairCachingGhostObject, m_allocator)();
    return object;
}

void Physics::destroyGhostObject(btPairCachingGhostObject* object)
{
    CUSTOM_DELETE(object, m_allocator);
}

btKinematicCharacterController* Physics::createCharacterController(btPairCachingGhostObject* object, btConvexShape* shape, float stepHeight)
{
    btKinematicCharacterController* controller = CUSTOM_NEW(btKinematicCharacterController, m_allocator)(object, shape, stepHeight);
    return controller;
}

void Physics::destroyCharacterController(btKinematicCharacterController* controller)
{
    CUSTOM_DELETE(controller, m_allocator);
}

uint32_t Physics::createConvexHullShape()
{
    btConvexHullShape* shape = CUSTOM_NEW(btConvexHullShape, m_allocator)();
    m_nextShapeID++;
    m_shapes[m_nextShapeID] = shape;
    return m_nextShapeID;
}

btTriangleMesh* Physics::createTriangleMesh() 
{
    btTriangleMesh* triangleMesh = CUSTOM_NEW(btTriangleMesh, m_allocator)();
    return triangleMesh;
}

btCollisionShape* Physics::getShapeForID(uint32_t shapeID)
{
    if (m_shapes.find(shapeID) == m_shapes.end())
    {
        return nullptr;
    }
    return m_shapes.at(shapeID);
}

uint32_t Physics::createBody(btScalar mass, btCollisionShape* collisionShape, const btVector3& localInertia)
{
    btDefaultMotionState* motionState = mass > 0.f ? CUSTOM_NEW(btDefaultMotionState, m_allocator)(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0))) : nullptr;
    btRigidBody* body = CUSTOM_NEW(btRigidBody, m_allocator)(mass, motionState, collisionShape, localInertia);
    m_nextBodyID++;
    m_bodies[m_nextBodyID] = body;
    return m_nextBodyID;
}

btRigidBody* Physics::getBodyForID(uint32_t bodyID)
{
    if (m_bodies.find(bodyID) == m_bodies.end())
    {
        return nullptr;
    }
    return m_bodies.at(bodyID);
}

void Physics::addBodyToWorld(btRigidBody* body, CollisionType group, CollisionType mask)
{
    m_dynamicsWorld->addRigidBody(body, (short)group, (short)mask);
}

void Physics::removeBodyFromWorld(btRigidBody* body)
{
    m_dynamicsWorld->removeRigidBody(body);
}

void Physics::removeShape(uint32_t shapeID)
{
    auto it = m_shapes.find(shapeID);
    if (it == m_shapes.end())
    {
        btCollisionShape* shape = it->second;
        CUSTOM_DELETE(shape, m_allocator);
        return;
    }
    m_shapes.erase(it);
}

void Physics::removeBody(uint32_t bodyID)
{
    auto it = m_bodies.find(bodyID);
    if (it == m_bodies.end())
    {
        btRigidBody* body = it->second;
        if (body->getMotionState())
        {
            CUSTOM_DELETE(body->getMotionState(), m_allocator);
        }
        CUSTOM_DELETE(body, m_allocator);
        return;
    }
    m_bodies.erase(it);
}

void Physics::SetRenderer(Renderer *renderer)
{
    m_debugDraw->m_renderer = renderer;
}

glm::vec3 Physics::cameraCollision(const glm::vec3& fromPos, const glm::vec3& toPos)
{    
    btVector3 cameraPosition = btVector3(fromPos.x,
                                         fromPos.y,
                                         fromPos.z);
    // OPTIONAL
    //use the convex sweep test to find a safe position for the camera (not blocked by static geometry)
    btSphereShape cameraSphere(0.1f);
    btTransform cameraFrom,cameraTo;
    cameraFrom.setIdentity();
    cameraFrom.setOrigin(cameraPosition);
    cameraTo.setIdentity();
    cameraTo.setOrigin(btVector3(toPos.x,
                                 toPos.y,
                                 toPos.z));
    // Check if path to wanted new position is clear
    btCollisionWorld::ClosestConvexResultCallback cb(cameraFrom.getOrigin(),
                                                     cameraTo.getOrigin());
    cb.m_collisionFilterGroup = (short)CollisionType::Group_Camera;
    cb.m_collisionFilterMask = (short)CollisionType::Group_Terrain;
    m_dynamicsWorld->convexSweepTest(&cameraSphere, cameraFrom, cameraTo, cb);
    if (cb.hasHit())
    {
        // Path is blocked, try to move camera closer to wanted position instead
        btScalar minFraction = btMax(btScalar(0.25),cb.m_closestHitFraction);
        cameraPosition.setInterpolate3(cameraFrom.getOrigin(),
                                       cameraTo.getOrigin(),
                                       minFraction);
        return glm::vec3(cameraPosition.x(),
                         cameraPosition.y(),
                         cameraPosition.z());
    }
    return toPos;
}

void colliderTick(btDynamicsWorld* world, btScalar timeStep)
{

}

void Physics::initialize()
{
    btAlignedAllocSetCustom(physics_alloc, physics_dealloc);

    m_broadphase = CUSTOM_NEW(btDbvtBroadphase, m_allocator)();
    m_collisionConfiguration = CUSTOM_NEW(btDefaultCollisionConfiguration, m_allocator)();
    m_dispatcher = CUSTOM_NEW(btCollisionDispatcher, m_allocator)(m_collisionConfiguration);   // Default dispatcher
//    dispatcher = CUSTOM_NEW(CollisionDispatcher, m_allocator)(collisionConfiguration);   // Custom dispatcher
    m_solver = CUSTOM_NEW(btSequentialImpulseConstraintSolver, m_allocator);
    m_dynamicsWorld = CUSTOM_NEW(btDiscreteDynamicsWorld, m_allocator)(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
    m_dynamicsWorld->setInternalTickCallback(internalTick);
//    ghostPairCallback = CUSTOM_NEW(btGhostPairCallback, m_allocator)();
//    dynamicsWorld->getPairCache()->setInternalGhostPairCallback(ghostPairCallback);

    m_dynamicsWorld->getDispatchInfo().m_useContinuous = m_physicsCCD;
    m_dynamicsWorld->setGravity(btVector3(0, -9.8f, 0));

    // Speed up optimizations
    btContactSolverInfo& info = m_dynamicsWorld->getSolverInfo();
    info.m_numIterations = 2;
    info.m_solverMode = (info.m_solverMode | SOLVER_ENABLE_FRICTION_DIRECTION_CACHING);
}

void Physics::terminate()
{
    // Cleanup in the reverse order of creation/initialization
    // Remove the rigidbodies from the dynamics world and delete them
    for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
        m_dynamicsWorld->removeCollisionObject(obj);
        CUSTOM_DELETE(obj, m_allocator);
    }

    CUSTOM_DELETE(m_dynamicsWorld, m_allocator);
    CUSTOM_DELETE(m_solver, m_allocator);
    CUSTOM_DELETE(m_dispatcher, m_allocator);
    CUSTOM_DELETE(m_collisionConfiguration, m_allocator);
    CUSTOM_DELETE(m_broadphase, m_allocator);
    CUSTOM_DELETE(m_debugDraw, m_allocator);
}

struct ExplosionInfo {
    float radius, force;
};

struct ExplosionSensorCallback : public btCollisionWorld::ContactResultCallback {
    
	//! Constructor, pass whatever context you want to have available when processing contacts
	/*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
	 *  (supplied by the superclass) for needsCollision() */
	ExplosionSensorCallback(btCollisionObject& tgtBody , ExplosionInfo& context )
    : btCollisionWorld::ContactResultCallback(), body(tgtBody), ctxt(context) { }
    
    btCollisionObject& body; //!< The body the sensor is monitoring
	ExplosionInfo& ctxt; //!< External information for contact processing
    
	//! If you don't want to consider collisions where the bodies are joined by a constraint, override needsCollision:
	/*! However, if you use a btCollisionObject for #body instead of a btRigidBody,
	 *  then this is unnecessary—checkCollideWithOverride isn't available */
    //	virtual bool needsCollision(btBroadphaseProxy* proxy) const {
    //		// superclass will check m_collisionFilterGroup and m_collisionFilterMask
    //		if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
    //			return false;
    //		// if passed filters, may also want to avoid contacts between constraints
    //		return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
    //	}
    //
	//! Called with each contact for your own processing (e.g. test if contacts fall in within sensor parameters)
	virtual btScalar addSingleResult(btManifoldPoint& cp,
                                     const btCollisionObjectWrapper* colObj0, int partId0, int index0,
                                     const btCollisionObjectWrapper* colObj1, int partId1, int index1)
	{
        btRigidBody* colBody;
		btVector3 pt; // will be set to point of collision relative to body
        btVector3 impulsePos;
        if(colObj0->m_collisionObject == &body)
        {
            pt = cp.m_localPointB;
            colBody = (btRigidBody*)colObj1->m_collisionObject;
            impulsePos = cp.m_positionWorldOnB;
		}
        else
        {
			assert(colObj1->m_collisionObject == &body); // otherwise body does not match either collision object
            pt = cp.m_localPointA;
            colBody = (btRigidBody*)colObj0->m_collisionObject;
            impulsePos = cp.m_positionWorldOnA;
		}
        btScalar dist = pt.length();
        btScalar distRatio = 1.f - (dist / ctxt.radius);
        Log::Debug("Explosion ratio: %f", distRatio);
        btScalar impulseForce = ctxt.force * (distRatio * distRatio);
        btVector3 impulse = pt.normalized() * impulseForce;
        if (ctxt.force > 0.f)
        {
            colBody->applyImpulse(impulse, impulsePos);
        }
        else
        {
            colBody->applyCentralImpulse(impulse);
        }
        colBody->activate(true);

		return 0; // not actually sure if return value is used for anything...?
	}
};

void Physics::Explosion(const btVector3& center, const float radius, const float force)
{
    btCollisionShape* sphereShape = CUSTOM_NEW(btSphereShape, m_allocator)(radius);
    btCollisionObject* sphere = CUSTOM_NEW(btCollisionObject, m_allocator)();
    sphere->setCollisionShape(sphereShape);
    sphere->getWorldTransform().setOrigin(center);
    ExplosionInfo explInfo = {radius, force};
    ExplosionSensorCallback callback(*sphere, explInfo);
    m_dynamicsWorld->contactTest(sphere, callback);
    CUSTOM_DELETE(sphere, m_allocator);
    CUSTOM_DELETE(sphereShape, m_allocator);
}

void Physics::internalTick(btDynamicsWorld* world, btScalar timeStep)
{
    if (!s_instance || !s_instance->m_collisionCallback)
    {
        Log::Error("Physics internal tick callback without static instance!");
        return;
    }

    const int numManifolds = world->getDispatcher()->getNumManifolds();
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);

        const btCollisionObject* objA = contactManifold->getBody0();
        const btCollisionObject* objB = contactManifold->getBody1();
        if (objA->getUserPointer() != nullptr ||
            objB->getUserPointer() != nullptr)
        {
            float force = 0.f;
            glm::vec3 contactPos;
            const int numContacts = contactManifold->getNumContacts();
            for (int j = 0; j < numContacts; j++)
            {
                btManifoldPoint& pt = contactManifold->getContactPoint(j);
                if (pt.getAppliedImpulse() < force)
                {
                    continue;
                }
                force = pt.getAppliedImpulse();
                //const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& ptB = pt.getPositionWorldOnB();
                //const btVector3& normalOnB = pt.m_normalWorldOnB;
                contactPos = glm::vec3(ptB.x(), ptB.y(), ptB.z());
            }

            s_instance->m_collisionCallback(objA->getUserPointer(), objB->getUserPointer(), contactPos, force);
        }
    }
}

void* Physics::physics_alloc(size_t size)
{
    if (!s_instance)
    {
        Log::Error("Physics allocation without static instance!");
        return nullptr;
    }
    return s_instance->m_allocator.allocate(size);
}

void Physics::physics_dealloc(void* memblock)
{
    if (!s_instance)
    {
        Log::Error("Physics deallocation without static instance!");
        return;
    }
    s_instance->m_allocator.deallocate(memblock);
}