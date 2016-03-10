#ifndef DYNACUBE_H
#define DYNACUBE_H

#include "GFXDefines.h"
#include "Physics.h"
#include "Color.h"

class Renderer;
class World3D;

class DynaCube
{
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
private:
    btCollisionShape* cubeShape;
    btRigidBody* cubeRigidBody;
    World3D* m_world;
};

#endif
