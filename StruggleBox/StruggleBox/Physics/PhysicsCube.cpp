#include "PhysicsCube.h"

#include "Renderer.h"
#include "MathUtils.h"
#include <iostream>

PhysicsCube::PhysicsCube(
    const btScalar mass,
    const btVector3& pos,
    const btVector3& size,
    const uint8_t materialID,
    Physics& physics,
    bool makeSphere /*= false*/)
    : m_physics(physics)
    , m_shapeID(0)
    , m_shape(nullptr)
    , m_bodyID(0)
    , m_body(nullptr)
    , m_materialID(materialID)
    , m_timer(0.0)
    , m_cubeSize(size.x())
    , m_isSphere(makeSphere)
{
    if (makeSphere)
    {
        m_shapeID = m_physics.createSphere(size.x());
    }
    else
    {
        m_shapeID = m_physics.createBox(size.x(), size.y(), size.z());
    }
    m_shape = m_physics.getShapeForID(m_shapeID);

    const bool isStatic = mass <= 0.0;
    btVector3 fallInertia(0, 0, 0);
    if (!isStatic)
    {
        m_shape->calculateLocalInertia(mass, fallInertia);
    }
    m_bodyID = m_physics.createBody(mass, m_shape, fallInertia);
    m_body = m_physics.getBodyForID(m_bodyID);
    m_physics.addBodyToWorld(
        m_body,
        isStatic ? CollisionType::Group_Terrain : CollisionType::Group_Fireball,
        isStatic ? CollisionType::Filter_Everything : CollisionType::Filter_Terrain_Entity);

    m_body->setRestitution(0.25);
    m_body->setFriction(0.5);
    m_body->setRollingFriction(0.5);
    //m_body->setUserPointer(this);

    if (mass > 0.0)
    {
        m_body->setDamping(0.001f, 0.001f);
        m_body->setDeactivationTime(0.5f);
        m_body->setSleepingThresholds(5.5f, 5.5f);

        if (m_physics.getIsUsingCCD())
        {
            m_body->setCcdMotionThreshold(m_cubeSize);
            m_body->setCcdSweptSphereRadius(0.5f * m_cubeSize);
        }
    }
    setPos(pos);
}

PhysicsCube::~PhysicsCube()
{
	m_physics.removeBodyFromWorld(m_body);
    m_physics.removeShape(m_shapeID);
    m_physics.removeBody(m_bodyID);
}

CubeInstance PhysicsCube::getRenderInstance() const
{
    btTransform trans;
    if (m_body->getMotionState())
    {
        m_body->getMotionState()->getWorldTransform(trans);
    }
    else
    {
        trans = m_body->getWorldTransform();
    }
    return {
        trans.getOrigin().x(),
        trans.getOrigin().y(),
        trans.getOrigin().z(),
        m_cubeSize,
        trans.getRotation().w(),
        trans.getRotation().x(),
        trans.getRotation().y(),
        trans.getRotation().z(),
        MaterialData::texOffsetX(m_materialID), MaterialData::texOffsetY(m_materialID)
    };
}

SphereVertexData PhysicsCube::getFireballInstance() const
{
    btTransform trans = getTransform();
    return {
        trans.getOrigin().x(),
        trans.getOrigin().y(),
        trans.getOrigin().z(),
        m_cubeSize * 2.f,
        0.f, 0.f
    };
}

CubeInstanceColor PhysicsCube::getSparkInstance() const
{
    static Color HOT_COLOR = RGBAColor(3.f, 1.f, 1.f, 1.f);
    static Color COLD_COLOR = RGBAColor(0.25f, 0.f, 0.f, 1.f);
    const double heatRatio = std::min(m_timer, 1.0);

    btTransform trans = getTransform();
    return {
        trans.getOrigin().x(),
        trans.getOrigin().y(),
        trans.getOrigin().z(),
        m_cubeSize,
        trans.getRotation().w(),
        trans.getRotation().x(),
        trans.getRotation().y(),
        trans.getRotation().z(),
        MathUtils::Lerp(COLD_COLOR.r, HOT_COLOR.r, heatRatio),
        MathUtils::Lerp(COLD_COLOR.g, HOT_COLOR.g, heatRatio),
        MathUtils::Lerp(COLD_COLOR.b, HOT_COLOR.b, heatRatio),
        1.f,
        1.f,
        1.f,
        1.f,
    };
}

btTransform PhysicsCube::getTransform() const
{
    btTransform trans;
    if (m_body->getMotionState())
    {
        m_body->getMotionState()->getWorldTransform(trans);
    }
    else
    {
        trans = m_body->getWorldTransform();
    }
    return trans;
}

bool PhysicsCube::isBodyActive() const
{
    return m_body ? m_body->isActive() : false;
}

void PhysicsCube::setPos(const btVector3& pos)
{
    btTransform cubeTrans;
	cubeTrans.setIdentity();
	cubeTrans.setOrigin(pos);
    m_body->setWorldTransform(cubeTrans);
    m_body->clearForces();
    m_body->setLinearVelocity( btVector3(0,0,0) );
    m_body->setAngularVelocity( btVector3(0,0,0) );
}

void PhysicsCube::setVelocity(const btVector3& vel)
{
    m_body->setLinearVelocity(vel);
}

void PhysicsCube::setRotation(const btQuaternion& rot)
{
    btTransform cubeTrans;
	cubeTrans.setIdentity();
	cubeTrans.setRotation(rot);
    m_body->setWorldTransform(cubeTrans);
    m_body->clearForces();
    m_body->setLinearVelocity( btVector3(0,0,0) );
    m_body->setAngularVelocity( btVector3(0,0,0) );
}

