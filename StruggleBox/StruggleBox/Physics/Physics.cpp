#include "Physics.h"
#include "CollisionDispatcher.h"
#include "Camera.h"
#include "Log.h"

#include "btGhostObject.h"
#include <iostream>

bool Physics::physicsCCD=false;
bool Physics::fixedTime=false;
int Physics::maxSubSteps=4;
btScalar Physics::fixedTimeStep=btScalar(1.)/btScalar(60.); // try to simulate physics at 60fps
Physics* Physics::g_physics = NULL;

Physics::Physics()
{
	Log::Info("[Physics] constructor, instance at %p", this);

    // Bullet physics engine setup
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);   // Default dispatcher
//    dispatcher = new CollisionDispatcher(collisionConfiguration);   // Custom dispatcher
    solver = new btSequentialImpulseConstraintSolver;
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
//    ghostPairCallback = new btGhostPairCallback();
//    dynamicsWorld->getPairCache()->setInternalGhostPairCallback(ghostPairCallback);

    dynamicsWorld->getDispatchInfo().m_useContinuous = physicsCCD;

    dynamicsWorld->setGravity(btVector3(0,-9.8f,0));
    deltaAccumulator = 0.0;
    
    debugDraw = new PhysicsDebug();
    debugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

    dynamicsWorld->setDebugDrawer(debugDraw);
    g_physics = this;
    
    defaultBoxShape = new btBoxShape(btVector3(0.25f,0.25f,0.25f));
    btScalar mass = 1;
    btVector3 fallInertia(0,0,0);
    defaultBoxShape->calculateLocalInertia(mass,fallInertia);
    // Speed up optimizations
    btContactSolverInfo& info = dynamicsWorld->getSolverInfo();
    info.m_numIterations = 2;
    info.m_solverMode = (info.m_solverMode|SOLVER_ENABLE_FRICTION_DIRECTION_CACHING);
}

Physics::~Physics() {
    g_physics = NULL;
    // Cleanup in the reverse order of creation/initialization
    // Remove the rigidbodies from the dynamics world and delete them
    for (int i=dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--) {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        dynamicsWorld->removeCollisionObject( obj );
        delete obj;
    }
    
    delete dynamicsWorld;
    dynamicsWorld = NULL;
    delete solver;
    solver = NULL;
    delete dispatcher;
    dispatcher = NULL;
    delete collisionConfiguration;
    collisionConfiguration = NULL;
    delete broadphase;
    broadphase = NULL;
    
    delete debugDraw;
    debugDraw = NULL;
}

void Physics::Update( double delta ) {
    if ( dynamicsWorld ) {
        if ( physicsCCD != dynamicsWorld->getDispatchInfo().m_useContinuous) {
            dynamicsWorld->getDispatchInfo().m_useContinuous= physicsCCD;
        }
        if ( fixedTime ) {
            deltaAccumulator += delta;
            while ( deltaAccumulator > fixedTimeStep ) {
                dynamicsWorld->stepSimulation( fixedTimeStep, maxSubSteps, fixedTimeStep );
                deltaAccumulator -= fixedTimeStep;
            }
        } else {
			//if (delta < 0.1) {
				dynamicsWorld->stepSimulation(delta, 0, fixedTimeStep);
			//}
        }
        // Get collision data

        
//        // Update explosions
//        for (int i=0; i<explosions.size(); i++) {
//            btPairCachingGhostObject* ghostObject = explosions[i];
//            btSphereShape* explosionSphere = (btSphereShape*)ghostObject->getCollisionShape();
//            btVector3 minAabb, maxAabb;
//            explosionSphere->getAabb(ghostObject->getWorldTransform(), minAabb, maxAabb);
//            dynamicsWorld->getBroadphase()->setAabb(ghostObject->getBroadphaseHandle(),
//                                                    minAabb, maxAabb,
//                                                    dispatcher);
//            
//            dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghostObject->getOverlappingPairCache(),
//                                                                      dynamicsWorld->getDispatchInfo(),
//                                                                      dynamicsWorld->getDispatcher());
//            
//            
//            
//            
//            btManifoldArray   manifoldArray;
//            btBroadphasePairArray& pairArray = ghostObject->getOverlappingPairCache()->getOverlappingPairArray();
//            int numPairs = pairArray.size();
//            
//            for (int i=0;i<numPairs;i++)
//            {
//                manifoldArray.clear();
//                
//                const btBroadphasePair& pair = pairArray[i];
//                
//                //unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
//                btBroadphasePair* collisionPair = dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
//                if (!collisionPair)
//                    continue;
//                
//                if (collisionPair->m_algorithm)
//                    collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
//                
//                for (int j=0;j<manifoldArray.size();j++)
//                {
//                    btPersistentManifold* manifold = manifoldArray[j];
////                    btScalar directionSign = manifold->getBody0() == ghostObject ? btScalar(-1.0) : btScalar(1.0);
//                    for (int p=0;p<manifold->getNumContacts();p++)
//                    {
//                        const btManifoldPoint&pt = manifold->getContactPoint(p);
//                        if (pt.getDistance()<0.f)
//                        {
//                            const btVector3& ptA = pt.getPositionWorldOnA();
//                            const btVector3& ptB = pt.getPositionWorldOnB();
////                            const btVector3& normalOnB = pt.m_normalWorldOnB;
//                            // EXPLOSION CALCULATION
//                            const btScalar dist = ptA.length();
//                            const btScalar radius = explosionSphere->getRadius();
//                            if ( dist < radius ) {
//                                int bodyType = manifold->getBody1()->getInternalType();
//                                if ( bodyType == btCollisionObject::CO_RIGID_BODY ) {
//                                    btRigidBody* body = (btRigidBody*)manifold->getBody1();
//                                    float impulseF = ((radius-dist)/radius) * 100.0f;
//                                    btVector3 impulse = ptA.normalized() * impulseF;
//                                    body->applyImpulse(impulse, ptB);
//                                }
//                            } else {
//                                printf("Explosion distance bigger than radius\n");
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        explosions.clear();
    }
}

void Physics::AddStaticBox(const btVector3 & pos, const btVector3 & halfSize) {
    btCollisionShape * colShape = new btBoxShape(halfSize);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f,0,colShape);
    btRigidBody * body = new btRigidBody(rbInfo);
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(pos);
    body->setWorldTransform(trans);
    dynamicsWorld->addRigidBody(body);
}

void Physics::AddDynamicBox(const btVector3& pos, const btVector3 & halfSize ) {
    btCollisionShape * colShape = new btBoxShape(halfSize);
    btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));
    btScalar mass = 1;
    btVector3 fallInertia(0,0,0);
    colShape->calculateLocalInertia(mass,fallInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,colShape,fallInertia);
    btRigidBody * body = new btRigidBody(fallRigidBodyCI);
    body->setRestitution(0.8f);
    dynamicsWorld->addRigidBody(body);
}
void Physics::AddDynamicVoxel(const btVector3& pos) {
    btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(1.0f,fallMotionState,defaultBoxShape,btVector3(0,0,0));
    btRigidBody * body = new btRigidBody(fallRigidBodyCI);
    body->setRestitution(0.8f);
    dynamicsWorld->addRigidBody(body);
}

void Physics::SetRenderer( Renderer *renderer ) {
    debugDraw->m_renderer = renderer;
}

const glm::vec3 Physics::CameraCollisions(const glm::vec3& fromPos,
                                          const glm::vec3& toPos)
{
    if ( g_physics == NULL ) return toPos;
    
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
    cb.m_collisionFilterMask = btBroadphaseProxy::StaticFilter;
    g_physics->dynamicsWorld->convexSweepTest(&cameraSphere,cameraFrom,cameraTo,cb);
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

struct ExplosionInfo {
    float radius, force;
};

struct ExplosionSensorCallback : public btCollisionWorld::ContactResultCallback {
    
	//! Constructor, pass whatever context you want to have available when processing contacts
	/*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
	 *  (supplied by the superclass) for needsCollision() */
	ExplosionSensorCallback(btRigidBody& tgtBody , ExplosionInfo& context )
    : btCollisionWorld::ContactResultCallback(), body(tgtBody), ctxt(context) { }
    
	btRigidBody& body; //!< The body the sensor is monitoring
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
                                     const btCollisionObjectWrapper* colObj0,int partId0,int index0,
                                     const btCollisionObjectWrapper* colObj1,int partId1,int index1)
	{
		btVector3 pt; // will be set to point of collision relative to body
		if(colObj0->m_collisionObject==&body) {
            pt = cp.m_localPointA;
//            pt = cp.m_positionWorldOnA;
            btScalar dist = pt.length();
            btScalar distRatio = ((dist)/ctxt.radius);
            btScalar impulseForce = ctxt.force*distRatio;
            btVector3 impulse = pt.normalized() * impulseForce;
            if ( ctxt.force > 0.0f ) {
                ((btRigidBody*)colObj1->m_collisionObject)->applyImpulse(impulse, cp.m_positionWorldOnB);
            } else {
                ((btRigidBody*)colObj1->m_collisionObject)->applyCentralImpulse(impulse);
            }
		} else {
			assert(colObj1->m_collisionObject==&body && "body does not match either collision object");
            pt = cp.m_localPointB;
//            pt = cp.m_positionWorldOnB;
            btScalar dist = pt.length();
            btScalar distRatio = ((dist)/ctxt.radius);
            btScalar impulseForce = ctxt.force*distRatio;
            btVector3 impulse = pt.normalized() * impulseForce;
            if ( ctxt.force > 0.0f ) {
                ((btRigidBody*)colObj0->m_collisionObject)->applyImpulse(impulse, cp.m_positionWorldOnB);
            } else {
                ((btRigidBody*)colObj0->m_collisionObject)->applyCentralImpulse(impulse);
            }
		}
		return 0; // not actually sure if return value is used for anything...?
	}
};

void Physics::Explosion( const btVector3 &pos, const float radius, const float force ) {
    btCollisionShape * colShape = new btSphereShape(radius);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f,0,colShape);
    btRigidBody * body = new btRigidBody(rbInfo);
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(pos);
    body->setWorldTransform(trans);
    ExplosionInfo explInfo = {radius,force};
    ExplosionSensorCallback callback(*body, explInfo);
    dynamicsWorld->contactTest(body,callback);
    delete body;
    delete colShape;
}

