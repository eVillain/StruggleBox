#ifndef STATIC_CUBE_H
#define STATIC_CUBE_H

#include "GFXDefines.h"
#include "Physics.h"
#include "Color.h"
#include <memory>

class Renderer;

class StaticCube
{
public:
    Color color;
    float cubeSize;

    StaticCube(const btVector3 & position,
               const btVector3 & halfSize,
               std::shared_ptr<Physics> physics,
               const Color& color);
    ~StaticCube();
    
    void Draw(Renderer* renderer);

    void SetPos(btVector3& pos);
    void SetRotation(btQuaternion& rot);
    
    btVector3 GetPos() { return cubeRigidBody->getWorldTransform().getOrigin(); }
private:
	btCollisionShape* cubeShape;
	btRigidBody* cubeRigidBody;
	std::shared_ptr<Physics> _physics;
};

#endif
