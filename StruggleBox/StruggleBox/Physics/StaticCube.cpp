#include "StaticCube.h"
#include "Renderer.h"
#include "MaterialData.h"
#include <iostream>

StaticCube::StaticCube(
	const btVector3 & pos,
	const btVector3 & halfSize,
	std::shared_ptr<Physics> physics,
	const Color& col) :
	_physics(physics)
{
    cubeShape = new btBoxShape(halfSize);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, 0, cubeShape);
    cubeRigidBody = new btRigidBody(rbInfo);
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(pos);
    cubeRigidBody->setWorldTransform(trans);
    _physics->dynamicsWorld->addRigidBody(cubeRigidBody);
    
    color = col;
    cubeSize = halfSize.x();
}

StaticCube::~StaticCube()
{
    _physics->dynamicsWorld->removeRigidBody(cubeRigidBody);
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
		MaterialData::texOffset(20)
    };
    renderer->bufferCubes(&cube, 1);
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

