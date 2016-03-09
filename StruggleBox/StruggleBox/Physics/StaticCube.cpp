#include <iostream>
#include "StaticCube.h"
#include "World3D.h"
#include "Renderer.h"

StaticCube::StaticCube(const btVector3 & pos,
                       const btVector3 & halfSize,
                       World3D* world,
                       const Color& col)
{
    m_world = world;

    cubeShape = new btBoxShape(halfSize);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, 0, cubeShape);
    cubeRigidBody = new btRigidBody(rbInfo);
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(pos);
    cubeRigidBody->setWorldTransform(trans);
    m_world->worldPhysics->dynamicsWorld->addRigidBody(cubeRigidBody);
    
    color = col;
    cubeSize = halfSize.x();
}

StaticCube::~StaticCube()
{
    m_world->worldPhysics->dynamicsWorld->removeRigidBody(cubeRigidBody);
    delete cubeRigidBody;
    delete cubeShape;
}

void StaticCube::Draw(Renderer* renderer)
{
    btTransform trans = cubeRigidBody->getWorldTransform();
    CubeInstance cube = {
        trans.getOrigin().x(),
        trans.getOrigin().y(),
        trans.getOrigin().z(),
        cubeSize,
        trans.getRotation().x(),
        trans.getRotation().y(),
        trans.getRotation().z(),
        trans.getRotation().w(),
        color.r,
        color.g,
        color.b,
        color.a,
        color.r,
        color.g,
        color.b,
        1.0f
    };
    renderer->Buffer3DCube(cube);
}

void StaticCube::SetPos(btVector3& pos)
{
    btTransform cubeTrans;
	cubeTrans.setIdentity();
	cubeTrans.setOrigin(pos);
    cubeRigidBody->setWorldTransform(cubeTrans);
    cubeRigidBody->clearForces();
    cubeRigidBody->setLinearVelocity( btVector3(0,0,0) );
    cubeRigidBody->setAngularVelocity( btVector3(0,0,0) );
}

void StaticCube::SetRotation(btQuaternion& rot)
{
    btTransform cubeTrans;
	cubeTrans.setIdentity();
	cubeTrans.setRotation(rot);
    cubeRigidBody->setWorldTransform(cubeTrans);
    cubeRigidBody->clearForces();
    cubeRigidBody->setLinearVelocity( btVector3(0,0,0) );
    cubeRigidBody->setAngularVelocity( btVector3(0,0,0) );
}

