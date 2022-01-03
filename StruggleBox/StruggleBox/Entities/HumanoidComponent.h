#ifndef HUMANOID_COMPONENT_H
#define HUMANOID_COMPONENT_H

#include "EntityComponent.h"
#include "CoreIncludes.h"
#include <glm/gtc/quaternion.hpp>

class EntityManager;
class Physics;
class VoxelFactory;

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

enum class ArmState {
    Arm_Idle = 0,
    Arm_Holding = 1,
    Arm_Blocking = 2,
    Arm_Swinging = 3,
    Arm_Throwing = 4,
};

class HumanoidComponent : public EntityComponent
{
public:
    double rHandTimer;
    double lHandTimer;
    
    HumanoidComponent(
		const int ownerID,
		EntityManager& entityManager,
		Physics& physics,
		VoxelFactory& voxels);
    virtual ~HumanoidComponent();

    virtual void update(const double delta);
    void updateAnimations(const double delta);
    
    void setCharacterType( const int newType );

    const void Rotate( const float rotX, const float rotY );
    void Rotate(glm::quat orientation);
    void Warp(glm::vec3 position);
    
    //void TakeHit(const glm::vec3 direction);
    
    // Item interface
    void Grab( Entity* grabbedObject );

    void Store( Entity* storedObject );
    void Drop( Entity* droppedObject );
    void Wield( Entity* wieldObject );
    
    void Die();
    
    void ThrowStart();
    void ThrowItem(const glm::vec3 targetPos);
    void UseRightHand();

    void WieldNextItemFromBackpack();
    

    ArmState getRightArmState() const { return rightArmAnimState; }

private:
	EntityManager& _entityManager;
	Physics& _physics;
	VoxelFactory& _voxels;

    float characterHeight;
    float characterRadius;
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

	Entity* m_backpack;
	Entity* m_rightHandItem;
	Entity* m_leftHandItem;
	Entity* m_headAccessoryItem;

    void removeAllVoxelMeshInstances(bool spawnAsDebris);
};

#endif
