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
	EntityManager& manager,
	Physics& physics,
	VoxelFactory& voxels) 
	: EntityComponent(ownerID, "Physics")
	, _manager(manager)
	, m_physics(physics)
	, m_voxels(voxels)
	, m_shapeID(0)
	, m_shape(nullptr)
	, m_bodyID(0)
	, m_body(nullptr)
	, m_isStatic(false)
	, m_isSensor(false)
	, m_timeAccumulator(0.0)
{
	Entity* m_owner = _manager.getEntity(_ownerID);
	if (!m_owner->HasAttribute("scale"))
	{
		glm::vec3& scale = m_owner->GetAttributeDataPtr<glm::vec3>("scale");
		scale = glm::vec3(0.1f);
	}
	//if (m_owner->HasAttribute("physics"))
	//{
	//	int& pMode = m_owner->GetAttributeDataPtr<int>("physics");
		//Log::Debug("[PhysComponent] had physics type %i, owner: %i", pMode, _ownerID);
		//setPhysicsMode(pMode, false, false);
		//pMode = (int)PhysicsMode::Physics_Off;
	//}

	m_owner->GetAttributeDataPtr<int>("physics") = (int)PhysicsMode::Physics_Off;
}

PhysicsComponent::~PhysicsComponent()
{
	clearPhysics();
}

void PhysicsComponent::clearPhysics()
{
	if (m_shapeID)
	{
		m_physics.removeShape(m_shapeID);
		m_shapeID = 0;
		m_shape = nullptr;
	}
	if (m_bodyID)
	{
		m_physics.removeBodyFromWorld(m_body);
		m_physics.removeBody(m_bodyID);
		m_bodyID = 0;
		m_body = nullptr;
	}
	Log::Debug("PhysicsComponent::clearPhysics cleared for entity %i", _ownerID);
}

void PhysicsComponent::setPhysicsMode(PhysicsMode newMode, bool isStatic, bool trigger)
{
	Entity* owner = _manager.getEntity(_ownerID);
	const PhysicsMode oldMode = (PhysicsMode)owner->GetAttributeDataPtr<int>("physics");

	if (oldMode == newMode)
	{
		if (m_body)
		{
			if (isStatic != m_isStatic)
			{
				m_physics.removeBodyFromWorld(m_body);
				m_physics.removeBody(m_bodyID);
				m_bodyID = 0;
				m_body = nullptr;
				createBody(isStatic);
			}
			if (trigger != m_isSensor)
			{
				const int flags = trigger ?
					m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT :
					m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT;
					m_body->setCollisionFlags(flags);
			}
		}
		return;
	}

	if (oldMode != PhysicsMode::Physics_Off)
	{
		clearPhysics();
	}

	if (newMode == PhysicsMode::Physics_Off)
	{
		owner->GetAttributeDataPtr<int>("physics") = (int)PhysicsMode::Physics_Off;
		return;
	}

	const bool requiresObjectFile = requiresVoxelFile(newMode);

	if (requiresObjectFile && !owner->HasAttribute("objectFile"))
	{
		Log::Error("[PhysicsComponent] no object file for physics, owner %i", _ownerID);
		owner->GetAttributeDataPtr<int>("physics") = (int)PhysicsMode::Physics_Off;
		return;
	}

	createShape(newMode);
	createBody(isStatic);

	if (m_body)
	{
		if (trigger)
		{
			m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		}

		owner->GetAttributeDataPtr<int>("physics") = (int)newMode;
	}
	else
	{
		owner->GetAttributeDataPtr<int>("physics") = (int)PhysicsMode::Physics_Off;
	}

	Log::Debug("PhysicsComponent::setPhysicsMode mode %i set for entity %i", owner->GetAttributeDataPtr<int>("physics"), _ownerID);
}

void PhysicsComponent::update(const double deltaTime)
{
	Entity* owner = _manager.getEntity(_ownerID);
	const PhysicsMode physicsMode = (PhysicsMode)owner->GetAttributeDataPtr<int>("physics");
	if (physicsMode == PhysicsMode::Physics_Off || !m_body)
	{
		return;
	}
	btTransform& trans = m_body->getWorldTransform();
	const bool updatePhysics = owner->GetAttributeDataPtr<int>("ownerID") == 0 && deltaTime != 0.0;
	if (updatePhysics)
	{
		// Move entity to physics object position
		btVector3 physPos = trans.getOrigin();
		btQuaternion physRot = trans.getRotation();
		owner->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(physPos.x(), physPos.y(), physPos.z());
		// Swizzle rotation as Bullet stores it in YZWX order (get WXYZ to restore XYZW order)
		owner->GetAttributeDataPtr<glm::quat>("rotation") = glm::quat(physRot.w(), physRot.x(), physRot.y(), physRot.z());
	}
	else if (m_body)
	{
		// Force positions of physics object
		const glm::vec3 position = owner->GetAttributeDataPtr<glm::vec3>("position");
		setPosition(position);
		const glm::quat rotation = owner->GetAttributeDataPtr<glm::quat>("rotation");
		setRotation(rotation);
		const glm::vec3 velocity = owner->GetAttributeDataPtr<glm::vec3>("velocity");
		m_body->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
	}
	if (m_body && owner->GetAttributeDataPtr<bool>("generateCollisions"))
	{
		m_timeAccumulator += deltaTime;
		if (m_timeAccumulator > contactResponseFrequency)
		{
			// Test for collisions with other physics things
			m_timeAccumulator -= contactResponseFrequency;
			//ContactSensorCallback callback(*m_body, *this);
			//m_physics.getWorld()->contactTest(m_body, callback);
		}
	}
	else if (m_timeAccumulator != 0) 
	{
		// Reset collision filter
		collisionFilter.clear();
		m_timeAccumulator = 0;
	}
}

void PhysicsComponent::setPosition(const glm::vec3 & position)
{
	if (!m_body)
	{
		return;
	}
	btTransform& trans = m_body->getWorldTransform();
	btVector3& pos = trans.getOrigin();
	float epsilon = 0.0001f;
	if (fabsf(pos.x() - position.x) > epsilon ||
		fabsf(pos.y() - position.y) > epsilon ||
		fabsf(pos.z() - position.z) > epsilon) {
		trans.setOrigin(btVector3(position.x, position.y, position.z));
		m_body->activate(true);
		m_body->clearForces();
	}
}

void PhysicsComponent::setRotation(const glm::quat & rotation)
{
	if (!m_body)
	{
		return;
	}
	btTransform& trans = m_body->getWorldTransform();
	btQuaternion rot = trans.getRotation();
	float epsilon = 0.0001f;
	if (fabsf(rot.w() - rotation.x) > epsilon ||
		fabsf(rot.x() - rotation.y) > epsilon ||
		fabsf(rot.y() - rotation.z) > epsilon ||
		fabsf(rot.z() - rotation.w) > epsilon) {
		rot = btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
		trans.setRotation(rot);
		m_body->activate(true);
		m_body->clearForces();
	}
}

void PhysicsComponent::setLinearVelocity(const btVector3* newLinVel)
{
	if (m_body)
	{
		m_body->setLinearVelocity(*newLinVel);
	}
}

void PhysicsComponent::setAngularVelocity(const btVector3* newAngVel) 
{
	if (m_body)
	{
		m_body->setAngularVelocity(*newAngVel);
	}
}

void PhysicsComponent::addContactToFilter(Entity*newContact)
{
	if (std::find(collisionFilter.begin(), collisionFilter.end(), newContact) == collisionFilter.end())
	{
		collisionFilter.push_back(newContact);
	} /*    else already had that contact in filter, shouldn't be adding it twice :(  */
}

void PhysicsComponent::createShape(const PhysicsMode mode)
{
	Entity* owner = _manager.getEntity(_ownerID);
	const std::string& objectFileName = owner->GetAttributeDataPtr<std::string>("objectFile");
	const glm::vec3 scale = owner->GetAttributeDataPtr<glm::vec3>("scale");

	if (mode == PhysicsMode::Physics_Cube_Mesh)
	{
		m_shapeID = m_voxels.getVoxels(objectFileName)->getPhysicsReduced(m_physics, scale.x * DEFAULT_VOXEL_MESHING_WIDTH);
	}
	else if (mode == PhysicsMode::Physics_Cube_Blocks)
	{
		m_shapeID = m_voxels.getVoxels(objectFileName)->getPhysicsCubes(m_physics, scale.x * DEFAULT_VOXEL_MESHING_WIDTH);
	}
	else if (mode == PhysicsMode::Physics_Cube_Hull)
	{
		uint32_t hullShapeID = m_voxels.getVoxels(objectFileName)->getPhysicsHull(m_physics, scale.x * DEFAULT_VOXEL_MESHING_WIDTH);
		btConvexHullShape* hullShape = (btConvexHullShape*)m_physics.getShapeForID(hullShapeID);
		// Create a hull approximation
		btShapeHull* hull = new btShapeHull(hullShape);
		btScalar margin = hullShape->getMargin();
		hull->buildHull(margin);
		m_shapeID = m_physics.createConvexHullShape();
		m_shape = m_physics.getShapeForID(m_shapeID);
		for (int i = 0; i < hull->numVertices(); i++)
		{
			((btConvexHullShape*)m_shape)->addPoint(hull->getVertexPointer()[i]);
		}
		delete hull;
	}
	else if (mode == PhysicsMode::Physics_Cube_AABBs)
	{
		m_shapeID = m_voxels.getVoxels(objectFileName)->getPhysicsAABBs(m_physics, scale * DEFAULT_VOXEL_MESHING_WIDTH);
	}
	else if (mode == PhysicsMode::Physics_Cube_Single)
	{
		m_shapeID = m_physics.createCube(scale.x * DEFAULT_VOXEL_MESHING_WIDTH);
	}
	else if (mode == PhysicsMode::Physics_Sphere)
	{
		const float radius = owner->GetAttributeDataPtr<float>("sphereRadius");
		m_shapeID = m_physics.createSphere(radius);
	}
	m_shape = m_physics.getShapeForID(m_shapeID);
}

void PhysicsComponent::createBody(const bool isStatic)
{
	Entity* owner = _manager.getEntity(_ownerID);
	const glm::vec3 position = owner->GetAttributeDataPtr<glm::vec3>("position");
	const glm::quat rotation = owner->GetAttributeDataPtr<glm::quat>("rotation");
	const float mass = isStatic ? 0.f : 1.f;
	const float restitution = 0.25f;
	const float friction = 0.75f;
	const float rollingFriction = 0.25f;

	btVector3 fallInertia(0, 0, 0);
	if (!isStatic)
	{
		m_shape->calculateLocalInertia(mass, fallInertia);
	}
	m_bodyID = m_physics.createBody(mass, m_shape, fallInertia);
	m_body = m_physics.getBodyForID(m_bodyID);
	if (!m_body)
	{
		Log::Error("PhysicsComponent::createBody failed to create body for entity %i!", _ownerID);
		return;
	}

	m_body->setUserPointer(owner);
	m_physics.addBodyToWorld(m_body, CollisionType::Group_Entity, CollisionType::Filter_Everything);

	btTransform& trans = m_body->getWorldTransform();
	const btQuaternion rot = btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
	trans.setRotation(rot);
	trans.setOrigin(btVector3(position.x, position.y, position.z));
	m_body->setRestitution(restitution);
	m_body->setFriction(friction);
	m_body->setRollingFriction(rollingFriction);
	m_body->setLinearVelocity(btVector3(0, 0, 0));
	m_body->setAngularVelocity(btVector3(0, 0, 0));
	m_body->clearForces();
	m_body->activate(true);
}

bool PhysicsComponent::requiresVoxelFile(PhysicsMode mode)
{
	return mode == PhysicsMode::Physics_Cube_Mesh ||
		mode == PhysicsMode::Physics_Cube_Blocks ||
		mode == PhysicsMode::Physics_Cube_AABBs ||
		mode == PhysicsMode::Physics_Cube_Hull;
}
