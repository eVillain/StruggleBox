//
//  PhysicsComponent.cpp
//  Bloxelizer
//
//  Created by The Drudgerist on 8/29/13.
//
//

#include "PhysicsComponent.h"
#include "World3D.h"
#include "EntityManager.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "Physics.h"
#include "Cubeject.h"

#include "ItemComponent.h" // For damage calculation

struct ContactSensorCallback : public btCollisionWorld::ContactResultCallback {
    
	//! Constructor, pass whatever context you want to have available when processing contacts
	/*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
	 *  (supplied by the superclass) for needsCollision() */
	ContactSensorCallback(btRigidBody& tgtBody , PhysicsComponent& context )
    : btCollisionWorld::ContactResultCallback(), body(tgtBody), ctxt(context) { }
    
	btRigidBody& body; //!< The body the sensor is monitoring
	PhysicsComponent& ctxt; //!< External information for contact processing
    
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
//			pt = cp.m_localPointA;
            pt = cp.m_positionWorldOnA;
		} else {
			assert(colObj1->m_collisionObject==&body && "body does not match either collision object");
//			pt = cp.m_localPointB;
            pt = cp.m_positionWorldOnB;
		}
        Entity* entityA = NULL;
        Entity* entityB = NULL;
        if( colObj0->m_collisionObject ) {
            entityA = (Entity*)colObj0->m_collisionObject->getUserPointer();
        }
        if( colObj1->m_collisionObject ) {
            entityB = (Entity*)colObj1->m_collisionObject->getUserPointer();
        }
//        Entity* m_owner = m_manager->GetEntity(m_ownerID);
//        ctxt.m_owner->world->numContacts = ctxt.m_owner->world->numContacts+1; // Add to counter for contacts generated this frame

        // Filter for previous collision objects here
        if ( std::find(ctxt.collisionFilter.begin(), ctxt.collisionFilter.end(), entityB ) != ctxt.collisionFilter.end() ) {
            return 0;
        }
        
        if ( entityA != NULL && entityB != NULL ) {
            //      std::string aName = entityA->GetAttributeDataPtr<std::string>("name");
            //      std::string bName = entityB->GetAttributeDataPtr<std::string>("name");
            int aType = entityA->GetAttributeDataPtr<int>("type");
            if ( aType == ENTITY_ITEM ) {   // Item collided with entity
                btVector3 velA = colObj0->m_collisionObject->getInterpolationLinearVelocity();
                const float velFactor = velA.length();
//                const float velFactor = cp.m_appliedImpulse;
                if ( velFactor > 1.0f ) {
                    ctxt.AddContactToFilter(entityB);
                    ItemComponent* itemA = (ItemComponent*) ctxt.m_manager->GetComponent(ctxt.GetOwnerID(),
                                                                                         "Item");
                    if ( itemA ) {
                        btVector3 normal = cp.m_normalWorldOnB;
                        itemA->HitEntity(entityB,
                                         glm::vec3(normal.x(),normal.y(),normal.z())*velFactor,
                                         glm::vec3(pt.x(),pt.y(),pt.z()));
                    }
                }
            }   // entityA is an item
        }   // got entityA and entityB
		return 0; // not actually sure if return value is used for anything...?
	}
};

const float contactResponseFrequency = 1.0f/6.0f;

PhysicsComponent::PhysicsComponent( const int ownerID, EntityManager* manager ) :
EntityComponent(ownerID) {
    m_family = "Physics";
    m_manager = manager;
    physicsMeshBody = NULL;
    physicsMeshShape = NULL;
    physicsMeshTris = NULL;
    physicsCompShape = NULL;
    physicsHullShape = NULL;
    physicsMode = Physics_Off;
    timeAccumulator = 0.0;
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
    if ( !m_owner->HasAttribute("scale")) {
        glm::vec3& scale = m_owner->GetAttributeDataPtr<glm::vec3>("scale");
        scale = glm::vec3(0.1f);
    }

    if ( m_owner->HasAttribute("objectFile")) {
        printf("[PhysComp] had object\n");
        std::string objFileAttr = m_owner->GetAttributeDataPtr<std::string>("objectFile");
        object = m_manager->world->LoadObject(objFileAttr);
    } else {
        printf("[PhysComp] NO object!!!\n");
    }
    if ( m_owner->HasAttribute("instanceID")) {
        printf("[PhysComp] had instance\n");
    } else {
        printf("[PhysComp] new instance\n");
        m_owner->GetAttributeDataPtr<int>("instanceID") = object->AddInstance(position);
    }
    if ( m_owner->HasAttribute("physics")) {
        int pMode = m_owner->GetAttributeDataPtr<int>("physics");
        printf("[PhysComp] had physics type %i\n", pMode);
        SetPhysicsMode(pMode);
    } else {
        printf("[PhysComp] no physics type\n");
        m_owner->GetAttributeDataPtr<int>("physics") = physicsMode;
    }
}

PhysicsComponent::~PhysicsComponent() {
    ClearPhysics();
}
void PhysicsComponent::ClearPhysics() {
    if ( physicsMeshBody ) {
        m_manager->world->worldPhysics->dynamicsWorld->removeRigidBody(physicsMeshBody);
        delete physicsMeshBody;
        physicsMeshBody=NULL;
    }
    if ( physicsMode == Physics_Rigid_Mesh ||
        physicsMode == Physics_Dynamic_Mesh ) {
        delete physicsMeshShape;
        delete physicsMeshTris;
        physicsMeshShape=NULL;
        physicsMeshTris=NULL;
    } else if ( physicsMode == Physics_Rigid_Blocks ||
               physicsMode == Physics_Dynamic_Blocks ||
               physicsMode == Physics_Dynamic_AABBs ) {
        delete physicsCompShape;
        physicsCompShape = NULL;
    } else if ( physicsMode == Physics_Dynamic_Hull ) {
        delete physicsHullShape;
        physicsHullShape = NULL;
    }
}
void PhysicsComponent::SetPhysicsMode( const int newMode, const bool trigger ) {
//    if ( physicsMode == newMode ) return;
    ClearPhysics();
    physicsMode = newMode;
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    m_owner->GetAttributeDataPtr<int>("physics") = physicsMode;
    const glm::vec3 scale = m_owner->GetAttributeDataPtr<glm::vec3>("scale");
    const glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
    const glm::quat rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
    float restitution = 0.3f;
    float friction = 0.8f;

    if ( physicsMode == Physics_Rigid_Mesh ) {
        int numCubes = object->cubes->GetNumCubes();
        int maxVerts = numCubes*36;
        btVector3* p_verts = new btVector3[maxVerts];
        unsigned int numPVerts = 0;                                          // Number of physics verts
        object->cubes->GetPhysicsReduced(p_verts, numPVerts, scale.x);
        physicsMeshTris = new btTriangleMesh();
        int numTris = numPVerts/3;
        for (int i=0; i<numTris; i++) { // Copy triangle data into new physics mesh
            physicsMeshTris->addTriangle(p_verts[i*3+0],
                                         p_verts[i*3+1],
                                         p_verts[i*3+2]);
        }
        //Create a new btBvhTriangleMeshShape from the btTriangleMesh
        const bool useQuantizedAABB = true;
        physicsMeshShape = new btBvhTriangleMeshShape(physicsMeshTris, useQuantizedAABB);
        physicsMeshBody = new btRigidBody(0.0, 0, physicsMeshShape, btVector3());
        m_manager->world->worldPhysics->dynamicsWorld->addRigidBody(physicsMeshBody);
    } else if ( physicsMode == Physics_Rigid_Blocks ) {
        physicsCompShape = new btCompoundShape();
        object->cubes->GetPhysicsCubes(physicsCompShape, scale.x);
        physicsMeshBody = new btRigidBody(0.0, 0, physicsCompShape, btVector3());
        m_manager->world->worldPhysics->dynamicsWorld->addRigidBody(physicsMeshBody);
    } else if ( physicsMode == Physics_Dynamic_Mesh ) {
        int numCubes = object->cubes->GetNumCubes();
        int maxVerts = numCubes*36;
        btVector3* p_verts = new btVector3[maxVerts];
        unsigned int numPVerts = 0;                                          // Number of physics verts
        object->cubes->GetPhysicsReduced(p_verts, numPVerts, scale.x);
        physicsMeshTris = new btTriangleMesh();
        int numTris = numPVerts/3;
        for (int i=0; i<numTris; i++) { // Copy triangle data into new physics mesh
            physicsMeshTris->addTriangle(p_verts[i*3+0],
                                         p_verts[i*3+1],
                                         p_verts[i*3+2]);
        }
        //Create a new btBvhTriangleMeshShape from the btTriangleMesh
        const bool useQuantizedAABB = false;
        physicsMeshShape = new btBvhTriangleMeshShape(physicsMeshTris, useQuantizedAABB);
        btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0) ));
        btScalar mass = 0.2f;
        btVector3 fallInertia(0,0,0);
        physicsMeshShape->calculateLocalInertia(mass,fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,physicsMeshShape,fallInertia);
        physicsMeshBody = new btRigidBody(fallRigidBodyCI);
        m_manager->world->worldPhysics->dynamicsWorld->addRigidBody(physicsMeshBody);
    } else if ( physicsMode == Physics_Dynamic_Blocks ) {
        physicsCompShape = new btCompoundShape();
        object->cubes->GetPhysicsCubes(physicsCompShape, scale.x);
        btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0) ));
        btScalar mass = 0.2f;
        btVector3 fallInertia(0,0,0);
        physicsCompShape->calculateLocalInertia(mass,fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,physicsCompShape,fallInertia);
        physicsMeshBody = new btRigidBody(fallRigidBodyCI);
        m_manager->world->worldPhysics->dynamicsWorld->addRigidBody(physicsMeshBody);
    } else if ( physicsMode == Physics_Dynamic_Hull ) {
        btConvexHullShape* tmpConvexShape = new btConvexHullShape();
        object->cubes->GetPhysicsHull(tmpConvexShape, scale.x);
        int numCubes = object->cubes->GetNumCubes();
        int maxVerts = numCubes*36;
        btVector3* p_verts = new btVector3[maxVerts];
        unsigned int numPVerts = 0;                                          // Number of physics verts
        object->cubes->GetPhysicsReduced(p_verts, numPVerts, scale.x);
        for (int i=0; i<numPVerts; i++) {
            tmpConvexShape->addPoint(p_verts[i]);
        }
		// Create a hull approximation
		btShapeHull* hull = new btShapeHull(tmpConvexShape);
		btScalar margin = tmpConvexShape->getMargin();
		hull->buildHull(margin);
        physicsHullShape = new btConvexHullShape();
		for (int i=0;i<hull->numVertices();i++) {
			physicsHullShape->addPoint(hull->getVertexPointer()[i]);
		}        
		delete tmpConvexShape;
		delete hull;
        btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0) ));
        btScalar mass = 0.2f;
        btVector3 fallInertia(0,0,0);
        physicsHullShape->calculateLocalInertia(mass,fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,physicsHullShape,fallInertia);
        physicsMeshBody = new btRigidBody(fallRigidBodyCI);
        m_manager->world->worldPhysics->dynamicsWorld->addRigidBody(physicsMeshBody);
    } else if ( physicsMode == Physics_Dynamic_AABBs ) {
        physicsCompShape = new btCompoundShape();
        object->cubes->GetPhysicsAABBs(physicsCompShape, scale.x);
        btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0) ));
        btScalar mass = 0.2f;
        btVector3 fallInertia(0,0,0);
        physicsCompShape->calculateLocalInertia(mass,fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,physicsCompShape,fallInertia);
        physicsMeshBody = new btRigidBody(fallRigidBodyCI);
        m_manager->world->worldPhysics->dynamicsWorld->addRigidBody(physicsMeshBody);
    }
    if ( physicsMeshBody ) {
        btTransform& trans = physicsMeshBody->getWorldTransform();
        btQuaternion rot = trans.getRotation();
        rot = btQuaternion(rotation.x,rotation.y,rotation.z,rotation.w);
        trans.setRotation(rot);
        trans.setOrigin( btVector3(position.x, position.y, position.z) );
        physicsMeshBody->setRestitution(restitution);
        physicsMeshBody->setFriction(friction);
        physicsMeshBody->setRollingFriction(0.5);
        physicsMeshBody->activate(true);
        physicsMeshBody->clearForces();
        physicsMeshBody->setLinearVelocity( btVector3(0,0,0) );
        physicsMeshBody->setAngularVelocity( btVector3(0,0,0) );
        physicsMeshBody->setUserPointer(m_owner);
        if ( trigger ) {
            physicsMeshBody->setCollisionFlags(physicsMeshBody->getCollisionFlags() |
                                               btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }
    }
}
void PhysicsComponent::Update( double delta ) {

    if ( physicsMode != Physics_Off ) {
        Entity* m_owner = m_manager->GetEntity(m_ownerID);
        btTransform& trans = physicsMeshBody->getWorldTransform();
        bool updatePhysics = true;
        if ( m_manager->world->paused ||
            m_owner->GetAttributeDataPtr<int>("ownerID") != -1 ) {
            updatePhysics = false;
        }
        if ( updatePhysics ) {    // Move entity to physics object position
            btVector3 physPos = trans.getOrigin();
            btQuaternion physRot = trans.getRotation();
            m_owner->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(physPos.x(),physPos.y(),physPos.z());
            // Swizzle rotation as Bullet stores it in YZWX order (get WXYZ to restore XYZW order)
            m_owner->GetAttributeDataPtr<glm::quat>("rotation") = glm::quat(physRot.w(), physRot.x(), physRot.y(), physRot.z());
        } else {        // Force positions of physics object
            const glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
            btVector3& pos = trans.getOrigin();
            float epsilon = 0.0001f;
            if ( fabsf(pos.x()-position.x) > epsilon ||
                 fabsf(pos.y()-position.y) > epsilon ||
                 fabsf(pos.z()-position.z) > epsilon ) {
                trans.setOrigin( btVector3(position.x, position.y, position.z) );
                physicsMeshBody->activate(true);
                physicsMeshBody->clearForces();
//                physicsMeshBody->setLinearVelocity( btVector3(0,0,0) );
//                physicsMeshBody->setAngularVelocity( btVector3(0,0,0) );
            }
            const glm::quat rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
            btQuaternion rot = trans.getRotation();
            if ( fabsf(rot.x()-rotation.x)>epsilon ||
                 fabsf(rot.y()-rotation.y)>epsilon ||
                 fabsf(rot.z()-rotation.z)>epsilon ||
                 fabsf(rot.w()-rotation.w)>epsilon ) {
                rot = btQuaternion(rotation.x,rotation.y,rotation.z,rotation.w);
                trans.setRotation(rot);
                physicsMeshBody->activate(true);
                physicsMeshBody->clearForces();
//                physicsMeshBody->setLinearVelocity( btVector3(0,0,0) );
//                physicsMeshBody->setAngularVelocity( btVector3(0,0,0) );
            }
            const glm::vec3 velocity = m_owner->GetAttributeDataPtr<glm::vec3>("velocity");
            physicsMeshBody->setLinearVelocity( btVector3(velocity.x,velocity.y,velocity.z) );
//            physicsMeshBody->applyCentralImpulse(btVector3(force.x,force.y,force.z));
        }
        if (  m_owner->GetAttributeDataPtr<bool>("generateCollisions") ) {
            timeAccumulator += delta;
            if ( timeAccumulator > contactResponseFrequency ) { // Test for collisions with other physics things
                timeAccumulator -= contactResponseFrequency;
                ContactSensorCallback callback(*physicsMeshBody, *this);
                m_manager->world->worldPhysics->dynamicsWorld->contactTest(physicsMeshBody,callback);
            }
        } else if ( timeAccumulator != 0 ) {   // Reset collision filter
            collisionFilter.clear();
            timeAccumulator = 0;
        }
    }
}
void PhysicsComponent::SetLinearVelocity( const btVector3* newLinVel ) {
    if ( physicsMeshBody ) {
        physicsMeshBody->setLinearVelocity( *newLinVel );
    }
}
void PhysicsComponent::SetAngularVelocity( const btVector3* newAngVel ) {
    if ( physicsMeshBody ) {
        physicsMeshBody->setAngularVelocity( *newAngVel );
    }
}
void PhysicsComponent::AddContactToFilter(Entity*newContact) {
    if (std::find(collisionFilter.begin(), collisionFilter.end(), newContact) == collisionFilter.end() ) {
        collisionFilter.push_back(newContact);
    } /*    else already had that contact in filter, shouldn't be adding it twice :(  */
};
