#include "PhysicsComponent.h"
#include "VoxelFactory.h"
#include "EntityManager.h"
#include "Physics.h"
#include "VoxelData.h"
#include "Log.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
//#include "ItemComponent.h" // For damage calculation

struct ContactSensorCallback : public btCollisionWorld::ContactResultCallback {

	//! Constructor, pass whatever context you want to have available when processing contacts
	/*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
	*  (supplied by the superclass) for needsCollision() */
	ContactSensorCallback(btRigidBody& tgtBody, PhysicsComponent& context)
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
		const btCollisionObjectWrapper* colObj0, int partId0, int index0,
		const btCollisionObjectWrapper* colObj1, int partId1, int index1)
	{
		btVector3 pt; // will be set to point of collision relative to body
		if (colObj0->m_collisionObject == &body) {
			//			pt = cp.m_localPointA;
			pt = cp.m_positionWorldOnA;
		}
		else {
			assert(colObj1->m_collisionObject == &body && "body does not match either collision object");
			//			pt = cp.m_localPointB;
			pt = cp.m_positionWorldOnB;
		}
		Entity* entityA = NULL;
		Entity* entityB = NULL;
		if (colObj0->m_collisionObject) {
			entityA = (Entity*)colObj0->m_collisionObject->getUserPointer();
		}
		if (colObj1->m_collisionObject) {
			entityB = (Entity*)colObj1->m_collisionObject->getUserPointer();
		}
		//        Entity* m_owner = _manager->getEntity(m_ownerID);
		//        ctxt.m_owner->world->numContacts = ctxt.m_owner->world->numContacts+1; // Add to counter for contacts generated this frame

		// Filter for previous collision objects here
		if (std::find(ctxt.collisionFilter.begin(), ctxt.collisionFilter.end(), entityB) != ctxt.collisionFilter.end()) {
			return 0;
		}

		if (entityA != NULL && entityB != NULL) {
			//      std::string aName = entityA->GetAttributeDataPtr<std::string>("name");
			//      std::string bName = entityB->GetAttributeDataPtr<std::string>("name");
			int aType = entityA->GetAttributeDataPtr<int>("type");
			if (aType == ENTITY_ITEM) {   // Item collided with entity
				btVector3 velA = colObj0->m_collisionObject->getInterpolationLinearVelocity();
				const float velFactor = velA.length();
				//                const float velFactor = cp.m_appliedImpulse;
				if (velFactor > 1.0f) {
					ctxt.addContactToFilter(entityB);
					//ItemComponent* itemA = (ItemComponent*) ctxt._manager->getComponent(ctxt.GetOwnerID(),
					//                                                                     "Item");
					//if ( itemA ) {
					//    btVector3 normal = cp.m_normalWorldOnB;
					//    itemA->hitEntity(entityB,
					//                     glm::vec3(normal.x(),normal.y(),normal.z())*velFactor,
					//                     glm::vec3(pt.x(),pt.y(),pt.z()));
					//}
				}
			}   // entityA is an item
		}   // got entityA and entityB
		return 0; // not actually sure if return value is used for anything...?
	}
};

const float contactResponseFrequency = 1.0f / 6.0f;

PhysicsComponent::PhysicsComponent(
	const int ownerID,
	std::shared_ptr<EntityManager> manager,
	std::shared_ptr<Physics> physics,
	std::shared_ptr<VoxelFactory> voxels) :
	EntityComponent(ownerID, "Physics"),
	_manager(manager),
	_physics(physics),
	_voxels(voxels)
{
	physicsMeshBody = NULL;
	physicsMeshShape = NULL;
	physicsMeshTris = NULL;
	physicsCompShape = NULL;
	physicsHullShape = NULL;
	timeAccumulator = 0.0;
	Entity* m_owner = _manager->getEntity(_ownerID);
	if (!m_owner->HasAttribute("scale")) {
		glm::vec3& scale = m_owner->GetAttributeDataPtr<glm::vec3>("scale");
		scale = glm::vec3(0.1f);
	}
	if (m_owner->HasAttribute("physics"))
	{
		int pMode = m_owner->GetAttributeDataPtr<int>("physics");
		Log::Debug("[PhysComponent] had physics type %i, owner: %i", pMode, _ownerID);
		setPhysicsMode(pMode);
	}
	else
	{
		m_owner->GetAttributeDataPtr<int>("physics") = Physics_Off;
	}
}

PhysicsComponent::~PhysicsComponent()
{
	clearPhysics();
}

void PhysicsComponent::clearPhysics()
{
	if (physicsMeshBody)
	{
		_physics->dynamicsWorld->removeRigidBody(physicsMeshBody);
		delete physicsMeshBody;
		physicsMeshBody = NULL;
	}
	if (physicsMeshShape)
	{
		delete physicsMeshShape;
		physicsMeshShape = NULL;
	}
	if (physicsMeshTris)
	{
		delete physicsMeshTris;
		physicsMeshTris = NULL;
	}
	if (physicsCompShape)
	{
		delete physicsCompShape;
		physicsCompShape = NULL;
	}
	if (physicsHullShape)
	{
		delete physicsHullShape;
		physicsHullShape = NULL;
	}
}

void PhysicsComponent::setPhysicsMode(
	const int newMode,
	const bool trigger)
{
	clearPhysics();

	if (newMode == Physics_Off)
		return;

	Entity* owner = _manager->getEntity(_ownerID);
	if (!owner->HasAttribute("objectFile"))
	{
		Log::Error("[PhysicsComponent] no object file for physics, owner %i", _ownerID);
		return;
	}
	if (!owner->HasAttribute("instanceID"))
	{
		Log::Error("[PhysicsComponent] no object file for physics, owner %i", _ownerID);
		return;
	}
	const std::string& objectFileName = owner->GetAttributeDataPtr<std::string>("objectFile");

	const glm::vec3 position = owner->GetAttributeDataPtr<glm::vec3>("position");
	const glm::quat rotation = owner->GetAttributeDataPtr<glm::quat>("rotation");
	const glm::vec3 scale = owner->GetAttributeDataPtr<glm::vec3>("scale");
	//owner->GetAttributeDataPtr<int>("instanceID") = _voxels->getMesh(objFileAttr)->addInstance(position);

	const float restitution = 0.3f;
	const float friction = 0.8f;

	if (newMode == Physics_Rigid_Mesh)
	{
		btVector3* p_verts = nullptr;
		unsigned int numPVerts = 0;                                          // Number of physics verts
		_voxels->getVoxels(objectFileName)->getPhysicsReduced(p_verts, numPVerts, DEFAULT_VOXEL_MESHING_WIDTH);
		physicsMeshTris = new btTriangleMesh();
		int numTris = numPVerts / 3;
		for (int i = 0; i<numTris; i++) { // Copy triangle data into new physics mesh
			physicsMeshTris->addTriangle(p_verts[i * 3 + 0],
				p_verts[i * 3 + 1],
				p_verts[i * 3 + 2]);
		}
		//Create a new btBvhTriangleMeshShape from the btTriangleMesh
		const bool useQuantizedAABB = true;
		physicsMeshShape = new btBvhTriangleMeshShape(physicsMeshTris, useQuantizedAABB);
		physicsMeshBody = new btRigidBody(0.0, 0, physicsMeshShape, btVector3());
		_physics->dynamicsWorld->addRigidBody(physicsMeshBody);
	}
	else if (newMode == Physics_Rigid_Blocks)
	{
		physicsCompShape = new btCompoundShape();
		_voxels->getVoxels(objectFileName)->getPhysicsCubes(physicsCompShape, DEFAULT_VOXEL_MESHING_WIDTH);
		physicsMeshBody = new btRigidBody(0.0, 0, physicsCompShape, btVector3());
		_physics->dynamicsWorld->addRigidBody(physicsMeshBody);
	}
	else if (newMode == Physics_Dynamic_Mesh)
	{
		btVector3* p_verts = nullptr;
		unsigned int numPVerts = 0;                                          // Number of physics verts
		_voxels->getVoxels(objectFileName)->getPhysicsReduced(p_verts, numPVerts, DEFAULT_VOXEL_MESHING_WIDTH);
		physicsMeshTris = new btTriangleMesh();
		int numTris = numPVerts / 3;
		for (int i = 0; i<numTris; i++) { // Copy triangle data into new physics mesh
			physicsMeshTris->addTriangle(p_verts[i * 3 + 0],
				p_verts[i * 3 + 1],
				p_verts[i * 3 + 2]);
		}
		//Create a new btBvhTriangleMeshShape from the btTriangleMesh
		const bool useQuantizedAABB = false;
		physicsMeshShape = new btBvhTriangleMeshShape(physicsMeshTris, useQuantizedAABB);
		btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btScalar mass = 0.2f;
		btVector3 fallInertia(0, 0, 0);
		physicsMeshShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, physicsMeshShape, fallInertia);
		physicsMeshBody = new btRigidBody(fallRigidBodyCI);
		_physics->dynamicsWorld->addRigidBody(physicsMeshBody);
	}
	else if (newMode == Physics_Dynamic_Blocks)
	{
		physicsCompShape = new btCompoundShape();
		_voxels->getVoxels(objectFileName)->getPhysicsCubes(physicsCompShape, DEFAULT_VOXEL_MESHING_WIDTH);
		btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btScalar mass = 0.2f;
		btVector3 fallInertia(0, 0, 0);
		physicsCompShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, physicsCompShape, fallInertia);
		physicsMeshBody = new btRigidBody(fallRigidBodyCI);
		_physics->dynamicsWorld->addRigidBody(physicsMeshBody);
	}
	else if (newMode == Physics_Dynamic_Hull)
	{
		btConvexHullShape* tmpConvexShape = new btConvexHullShape();
		_voxels->getVoxels(objectFileName)->getPhysicsHull(tmpConvexShape, DEFAULT_VOXEL_MESHING_WIDTH);
		btVector3* p_verts = nullptr;
		unsigned int numPVerts = 0;                                          // Number of physics verts
		_voxels->getVoxels(objectFileName)->getPhysicsReduced(p_verts, numPVerts, DEFAULT_VOXEL_MESHING_WIDTH);
		for (unsigned int i = 0; i<numPVerts; i++)
		{
			tmpConvexShape->addPoint(p_verts[i]);
		}
		// Create a hull approximation
		btShapeHull* hull = new btShapeHull(tmpConvexShape);
		btScalar margin = tmpConvexShape->getMargin();
		hull->buildHull(margin);
		physicsHullShape = new btConvexHullShape();
		for (int i = 0; i<hull->numVertices(); i++)
		{
			physicsHullShape->addPoint(hull->getVertexPointer()[i]);
		}
		delete tmpConvexShape;
		delete hull;
		btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btScalar mass = 0.2f;
		btVector3 fallInertia(0, 0, 0);
		physicsHullShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, physicsHullShape, fallInertia);
		physicsMeshBody = new btRigidBody(fallRigidBodyCI);
		_physics->dynamicsWorld->addRigidBody(physicsMeshBody);
	}
	else if (newMode == Physics_Dynamic_AABBs)
	{
		physicsCompShape = new btCompoundShape();
		_voxels->getVoxels(objectFileName)->getPhysicsAABBs(physicsCompShape, scale*DEFAULT_VOXEL_MESHING_WIDTH);
		btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btScalar mass = 0.2f;
		btVector3 fallInertia(0, 0, 0);
		physicsCompShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, physicsCompShape, fallInertia);
		physicsMeshBody = new btRigidBody(fallRigidBodyCI);
		_physics->dynamicsWorld->addRigidBody(physicsMeshBody);
	}
	if (physicsMeshBody)
	{
		btTransform& trans = physicsMeshBody->getWorldTransform();
		btQuaternion rot = trans.getRotation();
		rot = btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
		trans.setRotation(rot);
		trans.setOrigin(btVector3(position.x, position.y, position.z));
		physicsMeshBody->setRestitution(restitution);
		physicsMeshBody->setFriction(friction);
		physicsMeshBody->setRollingFriction(0.5);
		physicsMeshBody->activate(true);
		physicsMeshBody->clearForces();
		physicsMeshBody->setLinearVelocity(btVector3(0, 0, 0));
		physicsMeshBody->setAngularVelocity(btVector3(0, 0, 0));
		physicsMeshBody->setUserPointer(owner);
		if (trigger)
		{
			physicsMeshBody->setCollisionFlags(
				physicsMeshBody->getCollisionFlags() |
				btCollisionObject::CF_NO_CONTACT_RESPONSE);
		}
	}
	owner->GetAttributeDataPtr<int>("physics") = newMode;
}
void PhysicsComponent::update(const double deltaTime)
{
	Entity* owner = _manager->getEntity(_ownerID);
	const int physicsMode = owner->GetAttributeDataPtr<int>("physics");
	if (physicsMode != Physics_Off)
	{
		Entity* m_owner = _manager->getEntity(_ownerID);
		btTransform& trans = physicsMeshBody->getWorldTransform();
		bool updatePhysics = (m_owner->GetAttributeDataPtr<int>("ownerID") == -1);
		if (updatePhysics) {    // Move entity to physics object position
			btVector3 physPos = trans.getOrigin();
			btQuaternion physRot = trans.getRotation();
			m_owner->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(physPos.x(), physPos.y(), physPos.z());
			// Swizzle rotation as Bullet stores it in YZWX order (get WXYZ to restore XYZW order)
			m_owner->GetAttributeDataPtr<glm::quat>("rotation") = glm::quat(physRot.w(), physRot.x(), physRot.y(), physRot.z());
		}
		else {        // Force positions of physics object
			const glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
			setPosition(position);
			const glm::quat rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
			setRotation(rotation);
			const glm::vec3 velocity = m_owner->GetAttributeDataPtr<glm::vec3>("velocity");
			physicsMeshBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
		}
		if (m_owner->GetAttributeDataPtr<bool>("generateCollisions")) {
			timeAccumulator += deltaTime;
			if (timeAccumulator > contactResponseFrequency) { // Test for collisions with other physics things
				timeAccumulator -= contactResponseFrequency;
				ContactSensorCallback callback(*physicsMeshBody, *this);
				_physics->dynamicsWorld->contactTest(physicsMeshBody, callback);
			}
		}
		else if (timeAccumulator != 0) {   // Reset collision filter
			collisionFilter.clear();
			timeAccumulator = 0;
		}
	}
}

void PhysicsComponent::setPosition(const glm::vec3 & position)
{
	btTransform& trans = physicsMeshBody->getWorldTransform();
	btVector3& pos = trans.getOrigin();
	float epsilon = 0.0001f;
	if (fabsf(pos.x() - position.x) > epsilon ||
		fabsf(pos.y() - position.y) > epsilon ||
		fabsf(pos.z() - position.z) > epsilon) {
		trans.setOrigin(btVector3(position.x, position.y, position.z));
		physicsMeshBody->activate(true);
		physicsMeshBody->clearForces();
	}
}

void PhysicsComponent::setRotation(const glm::quat & rotation)
{
	btTransform& trans = physicsMeshBody->getWorldTransform();
	btQuaternion rot = trans.getRotation();
	float epsilon = 0.0001f;
	if (fabsf(rot.x() - rotation.x) > epsilon ||
		fabsf(rot.y() - rotation.y) > epsilon ||
		fabsf(rot.z() - rotation.z) > epsilon ||
		fabsf(rot.w() - rotation.w) > epsilon) {
		rot = btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
		trans.setRotation(rot);
		physicsMeshBody->activate(true);
		physicsMeshBody->clearForces();
	}
}

void PhysicsComponent::setLinearVelocity(const btVector3* newLinVel)
{
	if (physicsMeshBody)
	{
		physicsMeshBody->setLinearVelocity(*newLinVel);
	}
}

void PhysicsComponent::setAngularVelocity(const btVector3* newAngVel) 
{
	if (physicsMeshBody)
	{
		physicsMeshBody->setAngularVelocity(*newAngVel);
	}
}

void PhysicsComponent::addContactToFilter(Entity*newContact)
{
	if (std::find(collisionFilter.begin(), collisionFilter.end(), newContact) == collisionFilter.end())
	{
		collisionFilter.push_back(newContact);
	} /*    else already had that contact in filter, shouldn't be adding it twice :(  */
};
