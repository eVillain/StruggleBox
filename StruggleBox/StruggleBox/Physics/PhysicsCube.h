#pragma once

#include "GFXDefines.h"
#include "Physics.h"
#include "Color.h"
#include <memory>

class Renderer;

class PhysicsCube
{
public:
    PhysicsCube(
        const btScalar mass,
        const btVector3& pos,
        const btVector3& size,
        const uint8_t materialID,
        Physics& physics,
        bool makeSphere = false);
    ~PhysicsCube();
    
    CubeInstance getRenderInstance() const;
    SphereVertexData getFireballInstance() const;
    CubeInstanceColor getSparkInstance() const;

    btTransform getTransform() const;

    double getTimer() const { return m_timer; }
    void setTimer(double timer) { m_timer = timer; }
    bool isBodyActive() const;
    bool isSphere() const { return m_isSphere; }

    void setPos(const btVector3& pos);
    void setVelocity(const btVector3& vel);
    void setRotation(const btQuaternion& rot);

private:
	Physics& m_physics;

    uint32_t m_shapeID;
    uint32_t m_bodyID;
    btRigidBody* m_body;
    btCollisionShape* m_shape;

    float m_cubeSize;
    uint8_t m_materialID;
    double m_timer;

    bool m_isSphere;
    bool m_isSpark;
};
