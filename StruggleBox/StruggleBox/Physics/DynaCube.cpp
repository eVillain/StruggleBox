//
//  DynaCube.cpp
//  Bloxelizer
//
//  Created by Ville-Veikko Urrila on 5/14/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#include <iostream>
#include "DynaCube.h"
#include "World3D.h"
#include "Renderer.h"

DynaCube::DynaCube( const btVector3 & pos, const btVector3 & halfSize, World3D* world, const Color& col ) {
    m_world = world;

    cubeShape = new btBoxShape(halfSize);
    btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),pos));
    btScalar mass = 1;
    btVector3 fallInertia(0,0,0);
    cubeShape->calculateLocalInertia(mass,fallInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,cubeShape,fallInertia);
    cubeRigidBody = new btRigidBody(fallRigidBodyCI);
    cubeRigidBody->setRestitution(0.0f);
    cubeRigidBody->setFriction(1.0);
    cubeRigidBody->setRollingFriction(1.0);
    m_world->worldPhysics->dynamicsWorld->addRigidBody(cubeRigidBody);
    color = col;
    cubeSize = halfSize.x();
    if ( Physics::physicsCCD ) {
        cubeRigidBody->setCcdMotionThreshold(cubeSize);
        cubeRigidBody->setCcdSweptSphereRadius(0.9*cubeSize);
    }
//    cubeRigidBody->setDamping(0.05f, 0.85f);
    cubeRigidBody->setDeactivationTime(0.1f);
    cubeRigidBody->setSleepingThresholds(5.5f, 5.5f);
    timer = 0.0;
}

DynaCube::~DynaCube() {
    m_world->worldPhysics->dynamicsWorld->removeRigidBody(cubeRigidBody);
    delete cubeRigidBody->getMotionState();
    delete cubeRigidBody;
    delete cubeShape;
}

void DynaCube::Draw( Renderer* renderer ) {
    btTransform trans;
    cubeRigidBody->getMotionState()->getWorldTransform(trans);
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

void DynaCube::Update( double delta ) {
    if ( timer != 0.0 || !cubeRigidBody->isActive() ) {
        timer -= delta;
        if ( timer < 1.0 ) {
            color.a = timer;
            if ( timer < 0.0 ) {
                // Cube is dead
                m_world->RemoveDynaCube(this);
            }
        }
    }
}
void DynaCube::SetPos(btVector3& pos) {
    btTransform cubeTrans;
	cubeTrans.setIdentity();
	cubeTrans.setOrigin(pos);
    cubeRigidBody->setWorldTransform(cubeTrans);
    cubeRigidBody->clearForces();
    cubeRigidBody->setLinearVelocity( btVector3(0,0,0) );
    cubeRigidBody->setAngularVelocity( btVector3(0,0,0) );
}
void DynaCube::SetVelocity(btVector3& vel) {
    cubeRigidBody->setLinearVelocity(vel);
}
void DynaCube::SetRotation(btQuaternion& rot) {
    btTransform cubeTrans;
	cubeTrans.setIdentity();
	cubeTrans.setRotation(rot);
    cubeRigidBody->setWorldTransform(cubeTrans);
    cubeRigidBody->clearForces();
    cubeRigidBody->setLinearVelocity( btVector3(0,0,0) );
    cubeRigidBody->setAngularVelocity( btVector3(0,0,0) );
}

