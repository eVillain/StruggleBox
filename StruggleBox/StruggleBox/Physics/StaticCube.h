#ifndef STATIC_CUBE_H
#define STATIC_CUBE_H

#include "GFXDefines.h"
#include "Physics.h"
#include "Color.h"

class Renderer;
class World3D;

class StaticCube
{
    btCollisionShape* cubeShape;
    btRigidBody* cubeRigidBody;
    World3D* m_world;
    
public:
    Color color;
    float cubeSize;

    StaticCube(const btVector3 & position,
               const btVector3 & halfSize,
               World3D* world,
               const Color& color);
    ~StaticCube();
    
    void Draw(Renderer* renderer);

    void SetPos(btVector3& pos);
    void SetRotation(btQuaternion& rot);
    
    btVector3 GetPos() { return cubeRigidBody->getWorldTransform().getOrigin(); }
};

#endif
