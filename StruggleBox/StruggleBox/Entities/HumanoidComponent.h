//
//  HumanoidComponent.h
//  Bloxelizer
//
//  Created by The Drudgerist on 8/31/13.
//
//

#ifndef BWO_HUMANOID_COMPONENT_H
#define BWO_HUMANOID_COMPONENT_H

#include "EntityComponent.h"
#include "GFXIncludes.h"
#include <glm/gtc/quaternion.hpp>

class Cubeject;
class EntityManager;
class btDynamicsWorld;
class btKinematicCharacterController;
class btPairCachingGhostObject;
class btBroadphaseInterface;

typedef enum {
    Legs_Idle = 0,
    Legs_Sneaking = 1,
    Legs_Walking = 2,
    Legs_Running = 3,
    Legs_Jumping = 4,
} LegsState;

typedef enum {
    Arm_Idle = 0,
    Arm_Holding = 1,
    Arm_Blocking = 2,
    Arm_Swinging = 3,
    Arm_Throwing = 4,
} ArmState;

class HumanoidComponent : public EntityComponent {
private:
    EntityManager* m_manager;
    float characterHeight;
    float characterRadius;
    float wantedAngle;
    float sizeScale;
    float walkSpeed;
    
    btDynamicsWorld* physicsWorld;
    btKinematicCharacterController* character;
    btPairCachingGhostObject* ghostObject;
    btBroadphaseInterface* overlappingPairCache;

    
    // Animation states
    ArmState leftArmAnimState;
    ArmState rightArmAnimState;
    LegsState legsAnimState;
    // Animation vars
    float hipRotationAngle;
    float torsoTiltAngle;
    float torsoLeanAngle;
    float torsoBobAmount;
    
public:
    double rHandTimer;
    double lHandTimer;
    
    Entity* backPack;
    Entity* rightHandItem;
    Entity* leftHandItem;
    
    HumanoidComponent( const int ownerID, EntityManager* manager );
    virtual ~HumanoidComponent();
    virtual void Update( double delta );
    void UpdateAnimations( double delta );
    
    void SetCharacterType( const int newType );

    glm::vec3 GetPosition( void );
    glm::vec3 GetRotation( void );
    
    const void Rotate( const float rotX, const float rotY );
    void Rotate(glm::quat orientation);
    void Warp(glm::vec3 position);
    
    void TakeHit(const glm::vec3 direction);
    
    // Item interface
    void Grab( Entity* grabbedObject );
    void Store( Entity* storedObject );
    void Drop( Entity* droppedObject );
    
    void Wield( Entity* wieldObject );
    
    void Die();
    
    void ThrowStart();
    void ThrowItem( const glm::vec3 targetPos );
    void UseRightHand();
    
};

#endif
