//
//  DynaCube.h
//  Bloxelizer
//
//  Created by Ville-Veikko Urrila on 5/14/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#ifndef NGN_DYNACUBE_H
#define NGN_DYNACUBE_H
#include "GFXDefines.h"
#include "Physics.h"

class Renderer;
class World3D;

class DynaCube {
    btCollisionShape* cubeShape;
    btRigidBody* cubeRigidBody;
    World3D* m_world;
    
public:
    Color color;
    float cubeSize;
    double timer;   // Lifetime
    DynaCube( const btVector3 & pos, const btVector3 & halfSize, World3D* world, const Color& col );
    ~DynaCube();
    
    void Draw( Renderer* renderer );
    void Update( double delta );
    void SetPos(btVector3& pos);
    void SetVelocity(btVector3& vel);
    void SetRotation(btQuaternion& rot);

};

#endif
