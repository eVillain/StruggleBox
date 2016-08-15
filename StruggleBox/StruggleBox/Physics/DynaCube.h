#ifndef DYNACUBE_H
#define DYNACUBE_H

#include "GFXDefines.h"
#include "Physics.h"
#include "Color.h"
#include <memory>

class Renderer;

class DynaCube
{
public:
    Color color;
    float cubeSize;
    double timer;   // Lifetime
    DynaCube(
		const btVector3 & pos,
		const btVector3 & halfSize,
		std::shared_ptr<Physics> physics,
		const Color& col );
    ~DynaCube();
    
    void Draw( Renderer* renderer );
    void Update( double delta );
    void SetPos(btVector3& pos);
    void SetVelocity(btVector3& vel);
    void SetRotation(btQuaternion& rot);
private:
    btCollisionShape* cubeShape;
    btRigidBody* cubeRigidBody;
	std::shared_ptr<Physics> _physics;
};

#endif
