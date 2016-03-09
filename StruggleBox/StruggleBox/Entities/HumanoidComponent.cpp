#include "HumanoidComponent.h"
#include "World3D.h"
#include "EntityManager.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Renderer.h"
#include "SysCore.h"
#include "PhysicsComponent.h"
#include "ParticleComponent.h"
#include "ItemComponent.h"
#include "InventoryComponent.h"
#include "CubeComponent.h"
#include "ExplosiveComponent.h"
#include "Light3DComponent.h"
#include "HealthComponent.h"
#include "Cubeject.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "Timer.h"
#include <math.h>       /* sin */

const bool useCapsuleShape = true;

HumanoidComponent::HumanoidComponent(const int ownerID,
                                     EntityManager* manager) :
EntityComponent(ownerID)
{
    m_family = "Humanoid";
    m_manager = manager;
    
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    
    physicsWorld = m_manager->world->worldPhysics->dynamicsWorld;
    
    sizeScale           = 0.1f;
    float cubeRadius = BLOCK_RADIUS*sizeScale;
    characterRadius     = cubeRadius*16.0f;
    characterHeight     = cubeRadius*64.0f-(characterRadius*2.0f);
    walkSpeed           = 0.125f;
//    throwTimer          = 0.0;
    rHandTimer          = 0.0;
    lHandTimer          = 0.0;
    
    btTransform startTransform;
	startTransform.setIdentity ();
    const glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
	startTransform.setOrigin (btVector3(position.x, position.y, position.z));
    
    ghostObject = new btPairCachingGhostObject();
	ghostObject->setWorldTransform(startTransform);
    ghostObject->setUserPointer(m_owner);
    if ( useCapsuleShape ) {
        btConvexShape* capsule = new btCapsuleShape(characterRadius, characterHeight);
        ghostObject->setCollisionShape (capsule);
        ghostObject->setCollisionFlags (btCollisionObject::CF_CHARACTER_OBJECT);
        btScalar stepHeight = btScalar(0.5f);
        character = new btKinematicCharacterController(ghostObject,capsule,stepHeight);
    } else {
        btConvexShape* boxShape = new btBoxShape(btVector3(characterRadius,characterHeight,characterRadius));
        ghostObject->setCollisionShape (boxShape);
        ghostObject->setCollisionFlags (btCollisionObject::CF_CHARACTER_OBJECT);
        btScalar stepHeight = btScalar(0.5f);
        character = new btKinematicCharacterController(ghostObject,boxShape,stepHeight);
    }

	character->setFallSpeed(50.0f);
	character->setJumpSpeed(12.0f);
	character->setMaxJumpHeight(1.0f);
    
    physicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    // only collide with static for now (no interaction with dynamic objects)
	physicsWorld->addCollisionObject(ghostObject,
                                     btBroadphaseProxy::CharacterFilter,
                                     btBroadphaseProxy::AllFilter);
    //                                     btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
	physicsWorld->addAction(character);
    
    // Variables
    glm::quat rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
    Rotate(rotation);
    // Load player model files
    const int charType = m_owner->GetAttributeDataPtr<int>("type");
    SetCharacterType(charType);
    
    m_owner->GetAttributeDataPtr<int>("rightHandItemID") = -1;
    m_owner->GetAttributeDataPtr<int>("leftHandItemID") = -1;
    rightHandItem = NULL;
    leftHandItem = NULL;
    m_owner->GetAttributeDataPtr<int>("ownerID") = -1;

    glm::vec3& bb = m_owner->GetAttributeDataPtr<glm::vec3>("bb");
    bb = glm::vec3(characterRadius,characterHeight,characterRadius);
    
    leftArmAnimState = Arm_Idle;
    rightArmAnimState = Arm_Idle;
    legsAnimState = Legs_Idle;
    
    hipRotationAngle = 0.0f;
    torsoTiltAngle = 0.0f;
    torsoLeanAngle = 0.0f;
    torsoBobAmount = 0.0f;
    
    backPack = NULL;
    
    if ( !m_owner->HasAttribute("health") ) {
        m_owner->GetAttributeDataPtr<int>("health") = 100;
    }
}
HumanoidComponent::~HumanoidComponent() {
    //cleanup in the reverse order of creation/initialization
	if (character) {
		physicsWorld->removeCollisionObject(ghostObject);
        physicsWorld->removeAction(character);
	}

//    delete backPack;
//    backPack = NULL;
}
void HumanoidComponent::SetCharacterType( const int newType ) {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);

    float sizeH = sizeScale;
    float sizeV = sizeScale;
    glm::vec3 scale = glm::vec3(sizeH,sizeV,sizeH);
    
    float scaleCube = BLOCK_RADIUS*sizeScale;
    glm::vec3 torsoPos  = glm::vec3( 0.0f,  0.0f, 0.0f );
    glm::vec3 headPos   = glm::vec3( 0.0f, 26*scaleCube, 0.0f );
    glm::vec3 footLPos  = glm::vec3( 8*scaleCube, -14*scaleCube, 0.0f );
    glm::vec3 footRPos  = glm::vec3(-8*scaleCube, -14*scaleCube, 0.0f );
    glm::vec3 handLPos  = glm::vec3( 14*scaleCube, 0.0f, 0.0f );
    glm::vec3 handRPos  = glm::vec3(-14*scaleCube, 0.0f, 0.0f );
    int& torsoID = m_owner->GetAttributeDataPtr<int>("torsoID");
    int& headID = m_owner->GetAttributeDataPtr<int>("headID");
    int& headAccessoryID = m_owner->GetAttributeDataPtr<int>("headAccessoryID");
    int& leftFootID = m_owner->GetAttributeDataPtr<int>("leftFootID");
    int& rightFootID = m_owner->GetAttributeDataPtr<int>("rightFootID");
    int& leftArmID = m_owner->GetAttributeDataPtr<int>("leftArmID");
    int& rightArmID = m_owner->GetAttributeDataPtr<int>("rightArmID");

    if ( torsoID != -1 ) { m_manager->world->RemoveObject(torsoID); }
    if ( headID != -1 ) { m_manager->world->RemoveObject(headID); }
    if ( headAccessoryID != -1 ) { m_manager->world->RemoveObject(headAccessoryID); }
    if ( leftFootID != -1 ) { m_manager->world->RemoveObject(leftFootID); }
    if ( rightFootID != -1 ) { m_manager->world->RemoveObject(rightFootID); }
    if ( leftArmID != -1 ) { m_manager->world->RemoveObject(leftArmID); }
    if ( rightArmID != -1 ) { m_manager->world->RemoveObject(rightArmID); }

    std::string& torsoObject = m_owner->GetAttributeDataPtr<std::string>("torsoObject");
    std::string& headObject = m_owner->GetAttributeDataPtr<std::string>("headObject");
    std::string& headAccessoryObject = m_owner->GetAttributeDataPtr<std::string>("headAccessoryObject");
    std::string& leftFootObject = m_owner->GetAttributeDataPtr<std::string>("leftFootObject");
    std::string& rightFootObject = m_owner->GetAttributeDataPtr<std::string>("rightFootObject");
    std::string& leftArmObject = m_owner->GetAttributeDataPtr<std::string>("leftArmObject");
    std::string& rightArmObject = m_owner->GetAttributeDataPtr<std::string>("rightArmObject");

    if ( newType == ENTITY_SKELETON ) {
        torsoObject = "Skeleton_Torso.bwo";
        headObject = "Skull.bwo";
        headAccessoryObject = "";
        leftFootObject = "FootLSkeleton.bwo";
        rightFootObject = "FootRSkeleton.bwo";
        leftArmObject = "HandLSkeleton.bwo";
        rightArmObject = "HandRSkeleton.bwo";
        torsoID = m_manager->world->AddObject(torsoObject, torsoPos, scale);
        headID = m_manager->world->AddObject(headObject, headPos, scale);
        headAccessoryID = -1;
        leftFootID = m_manager->world->AddObject(leftFootObject, footLPos, scale);
        rightFootID = m_manager->world->AddObject(rightFootObject, footRPos, scale);
        leftArmID = m_manager->world->AddObject(leftArmObject, handLPos, scale);
        rightArmID = m_manager->world->AddObject(rightArmObject, handRPos, scale);

    } else if ( newType == ENTITY_HUMANOID ) {
        torsoObject = "Naked_Torso.bwo";
        headObject = "Head1.bwo";
//        headAccessoryObject = "Helmet_Iron.bwo";
        headAccessoryObject = "";
        leftFootObject = "Naked_FootL.bwo";
        rightFootObject = "Naked_FootR.bwo";
        leftArmObject = "Naked_HandL.bwo";
        rightArmObject = "Naked_HandR.bwo";
        // Load the object instances
        torsoID         = m_manager->world->AddObject(torsoObject, torsoPos, scale);
        headID          = m_manager->world->AddObject(headObject, headPos, scale);
//        headAccessoryID = m_manager->world->AddObject(headAccessoryObject, headPos, scale);
        headAccessoryID = -1;
        leftFootID      = m_manager->world->AddObject(leftFootObject, footLPos, scale);
        rightFootID     = m_manager->world->AddObject(rightFootObject, footRPos, scale);
        leftArmID       = m_manager->world->AddObject(leftArmObject, handLPos, scale);
        rightArmID      = m_manager->world->AddObject(rightArmObject, handRPos, scale);
    }
}
// The update funtion takes input data and picks wanted animation states
void HumanoidComponent::Update( double delta ) {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);

    const int health = m_owner->GetAttributeDataPtr<int>("health");
    if ( health <= 0 ) {
        Die();
        return;
    }
    bool& jumping = m_owner->GetAttributeDataPtr<bool>("jumping");
    bool& running = m_owner->GetAttributeDataPtr<bool>("running");
    bool& sneaking = m_owner->GetAttributeDataPtr<bool>("sneaking");
    glm::vec3& moveInput = m_owner->GetAttributeDataPtr<glm::vec3>("moveInput");        // First person movement input
    glm::vec3& direction = m_owner->GetAttributeDataPtr<glm::vec3>("direction");        // Third person movement input
    
    glm::quat newRot = glm::angleAxis(wantedAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    btScalar walkFactor = 1.0f;
    if ( running && sneaking ) walkFactor *= 0.25f;
    else if ( running ) walkFactor *= 2.0f;
    else if ( sneaking ) walkFactor *= 0.5f;

    // Update object positions and rotations based on physics transform
    if ( moveInput.x || moveInput.z ) {
        // First person controls !!!UNTESTED!!!
        btQuaternion playerRotation = ghostObject->getWorldTransform().getRotation();
        glm::quat oldRot = glm::quat(playerRotation.w(),playerRotation.x(),playerRotation.y(),playerRotation.z());
        glm::vec3 moveDir = moveInput*oldRot;
        character->setWalkDirection(btVector3(moveDir.x,moveDir.y,moveDir.z)*walkFactor*walkSpeed);
    } else {
        // Third person controls
        btTransform trans = ghostObject->getWorldTransform();
        btVector3 playerPosition = trans.getOrigin();
        btQuaternion playerRotation = trans.getRotation();
        torsoLeanAngle = 0.0f;
        
        // Calculate new wanted angle from movement direction
        wantedAngle = atan2(direction.x, direction.z);
        bool grounded = character->onGround();
        if ( !grounded ) {
            legsAnimState = Legs_Jumping;
            walkFactor *= 0.8f; // Slower air control
        } else {
            if (direction.x == 0.0f &&
                direction.z == 0.0f ) {
                // No movement input, not moving, set legs to idle
                legsAnimState = Legs_Idle;
            } else {
                if ( jumping ) { character->jump(); }
                // If running slow down turn and lean forward
                if (running && !sneaking) {
                    float torsoLeanFactor = toRads(30.0f);
                    // Amount of rotation slerp, controls speed of rotation
                    float runRotSlerp = 0.4f;
                    if ( running ) {
                        runRotSlerp = 0.2f;
                        torsoLeanFactor = toRads(90.0f);
                    }
                    glm::quat oldRot = glm::quat(playerRotation.w(),playerRotation.x(),playerRotation.y(),playerRotation.z());
                    glm::quat wantedRot = glm::angleAxis(wantedAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                    newRot = glm::slerp(oldRot, wantedRot, runRotSlerp);
                    // If running and turning we can also lean to a side
                    glm::quat conjugate = glm::conjugate(oldRot);
                    glm::quat difference = newRot*conjugate;
                    torsoLeanAngle = -(difference.y * difference.w)*torsoLeanFactor;
                    
                    legsAnimState = Legs_Running;
                } else if (sneaking) {
                    legsAnimState = Legs_Sneaking;
                } else {
                    legsAnimState = Legs_Walking;
                }
            }
        }
        
        if ( !m_manager->world->paused ) {        // Update movement and rotation
            btQuaternion newRotation = btQuaternion(newRot.x, newRot.y, newRot.z, newRot.w);
			ghostObject->getWorldTransform().setRotation(newRotation);
			//printf("Player rotation:%f,%f,%f,%f\n", newRot.x,newRot.y,newRot.z,newRot.w);
            glm::vec3 walkDir = newRot*glm::vec3(0.0,0.0,glm::length(glm::vec2(direction.x,direction.z)));
            // Move physics character
            character->setWalkDirection(btVector3(walkDir.x, walkDir.y, walkDir.z)*walkFactor*walkSpeed);
        } else {    // Game is paused, force move player instead of letting physics do the moving
            const float epsilon = 0.00001f;
            const glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
            if ( fabsf(position.x - playerPosition.x()) > epsilon ||
                 fabsf(position.y - playerPosition.y()) > epsilon ||
                 fabsf(position.z - playerPosition.z()) > epsilon ) {
                playerPosition = btVector3(position.x, position.y, position.z);
                character->warp(playerPosition);
            }
            const glm::quat rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
            if ( fabsf( rotation.x - playerRotation.x() ) > epsilon ||
                 fabsf( rotation.y - playerRotation.y() ) > epsilon ||
                 fabsf( rotation.z - playerRotation.z() ) > epsilon ||
                 fabsf( rotation.w - playerRotation.w() ) > epsilon )  {
                trans.setRotation(btQuaternion(rotation.x,rotation.y,rotation.z, rotation.w));
            }
        }
    }
    UpdateAnimations(delta);
}

void HumanoidComponent::UpdateAnimations(double delta)
{
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    int torsoID = m_owner->GetAttributeDataPtr<int>("torsoID");
    int headID = m_owner->GetAttributeDataPtr<int>("headID");
    int headAccessoryID = m_owner->GetAttributeDataPtr<int>("headAccessoryID");
    int leftFootID = m_owner->GetAttributeDataPtr<int>("leftFootID");
    int rightFootID = m_owner->GetAttributeDataPtr<int>("rightFootID");
    int leftHandID = m_owner->GetAttributeDataPtr<int>("leftArmID");
    int rightHandID = m_owner->GetAttributeDataPtr<int>("rightArmID");
    std::string torsoObject = m_owner->GetAttributeDataPtr<std::string>("torsoObject");
    std::string headObject = m_owner->GetAttributeDataPtr<std::string>("headObject");
    std::string headAccessoryObject = m_owner->GetAttributeDataPtr<std::string>("headAccessoryObject");
    std::string leftFootObject = m_owner->GetAttributeDataPtr<std::string>("leftFootObject");
    std::string rightFootObject = m_owner->GetAttributeDataPtr<std::string>("rightFootObject");
    std::string leftHandObject = m_owner->GetAttributeDataPtr<std::string>("leftArmObject");
    std::string rightHandObject = m_owner->GetAttributeDataPtr<std::string>("rightArmObject");
    
    Cubeject* torsoObj = m_manager->world->GetObject(torsoObject);
    Cubeject* headObj = m_manager->world->GetObject(headObject);
    Cubeject* leftFootObj = m_manager->world->GetObject(leftFootObject);
    Cubeject* rightFootObj = m_manager->world->GetObject(rightFootObject);
    Cubeject* leftHandObj = m_manager->world->GetObject(leftHandObject);
    Cubeject* rightHandObj = m_manager->world->GetObject(rightHandObject);
    
    InstanceData* torso       = torsoObj->GetInstance(torsoID);
    InstanceData* head        = headObj->GetInstance(headID);
    InstanceData* leftFoot    = leftFootObj->GetInstance(leftFootID);
    InstanceData* rightFoot   = rightFootObj->GetInstance(rightFootID);
    InstanceData* leftHand    = leftHandObj->GetInstance(leftHandID);
    InstanceData* rightHand   = rightHandObj->GetInstance(rightHandID);

    const float animationSpeed = 20.0f;
    const float scaleCube = BLOCK_RADIUS*sizeScale;

    // Update object positions and rotations based on physics transform
    btTransform trans = ghostObject->getWorldTransform();
    btVector3 playerPosition = trans.getOrigin();
    glm::quat torsoRotation = glm::quat(trans.getRotation().w(),trans.getRotation().x(),trans.getRotation().y(),trans.getRotation().z());

    double time = Timer::Seconds();

    // Walkangles
    float leftFootAngle = std::sin(time*animationSpeed);
    float rightFootAngle = std::sin(-time*animationSpeed);
    torsoBobAmount = 0.0f;
    hipRotationAngle = 0.0f;
    torsoTiltAngle = 0.0f;

    // Look at general body motion first ( = what the legs are doing )
    if (legsAnimState == Legs_Idle) {
        torsoBobAmount = std::sin(time*10.0f)*0.01f;
        float newF_ratio = 1.0f*delta*10.0f;
        float oldF_ratio = 1.0f - newF_ratio;
        leftFootAngle = (leftFoot->rotation.x*oldF_ratio);
        rightFootAngle = (rightFoot->rotation.x*oldF_ratio);
    } else if ( legsAnimState == Legs_Sneaking ) {
        leftFootAngle *= toRads(15.0f);
        rightFootAngle *= toRads(15.0f);
        hipRotationAngle = std::sin(time*animationSpeed*0.25f)*toRads(5.0f);
        torsoBobAmount += std::sin(time*animationSpeed)*0.01f;
    } else if ( legsAnimState == Legs_Walking ) {
        torsoTiltAngle = toRads(2.5f);
        leftFootAngle *= toRads(45.0f);
        rightFootAngle *= toRads(45.0f);
        hipRotationAngle = std::sin(time*animationSpeed*0.5f)*toRads(5.0f);
        torsoBobAmount += std::sin(time*animationSpeed)*0.02f;
    } else if ( legsAnimState == Legs_Running ) {
        torsoTiltAngle = toRads(10.0f);
        leftFootAngle *= toRads(60.0f);
        rightFootAngle *= toRads(60.0f);
        hipRotationAngle = std::sin(time*animationSpeed)*toRads(5.0f);
        torsoBobAmount += std::sin(time*animationSpeed)*0.03f;
    } else if ( legsAnimState == Legs_Jumping ) {
        float newF_ratio = 0.9f *delta*10.0f;
        float oldF_ratio = 1.0f - newF_ratio;
        leftFootAngle = (leftFoot->rotation.x*oldF_ratio)+(toRads(60.0f)*newF_ratio);
        rightFootAngle = (rightFoot->rotation.x*oldF_ratio)+(toRads(60.0f)*newF_ratio);
    }
    torsoRotation = torsoRotation* glm::angleAxis(torsoLeanAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::quat leftFootRot = glm::angleAxis(leftFootAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat rightFootRot = glm::angleAxis(rightFootAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat leftHandRot = glm::angleAxis(-0.25f*leftFootAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat rightHandRot = glm::angleAxis(-0.25f*rightFootAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    
    float leftHandSlerp = 1.0f;
    float rightHandSlerp = 1.0f;
    // Now find parameters for the arms
    if ( leftArmAnimState == Arm_Idle ) {
        bool& running = m_owner->GetAttributeDataPtr<bool>("running");
        if ( running ) {
            leftHandSlerp = 0.3f;
            glm::quat leftHandLift = glm::angleAxis(toRads(35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            leftHandRot = leftHandRot*leftHandLift;
        } else { // Regular slerp of rotation
            leftHandSlerp = 0.5f;
        }
    } else if ( leftArmAnimState == Arm_Holding ) {
        
    } else if ( leftArmAnimState == Arm_Blocking ) {
        
    } else if ( leftArmAnimState == Arm_Swinging ) {
        
    } else if ( leftArmAnimState == Arm_Throwing ) {
        
    }
    
    if ( rightArmAnimState == Arm_Idle ) {
        bool& running = m_owner->GetAttributeDataPtr<bool>("running");
        if ( running ) {
            rightHandSlerp = 0.3f;
            glm::quat rightHandLift = glm::angleAxis(toRads(-35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            rightHandRot = rightHandRot*rightHandLift;
        } else { // Regular slerp of rotation
            rightHandSlerp = 0.5f;
        }
    } else if ( rightArmAnimState == Arm_Holding ) {
        float swingTime = fminf(1.0f, (time-rHandTimer)*5.0f);
        if ( swingTime == 1.0f ) {
            rightArmAnimState = Arm_Idle;
            rHandTimer = 0.0f;
            if ( rightHandItem ) {
                if ( rightHandItem->GetAttributeDataPtr<int>("itemType") == Item_Potion_Health ) {
                    // Use potion and delete it from inventory
                    int potionID = rightHandItem->GetAttributeDataPtr<int>("ID");
                    int health = rightHandItem->GetAttributeDataPtr<int>("health");
                    HealthComponent* hc = (HealthComponent*)m_manager->GetComponent(potionID, "Health");
                    if ( hc ) hc->AddHealth( health );
                    m_manager->KillEntity(potionID);
                    rightHandItem = NULL;
                }
            }
        } else {
            float swingAngle = toRads(135.0f)+std::sin(1.0-swingTime)*toRads(225.0f);
            glm::quat rightHandSwing = glm::angleAxis(swingAngle, glm::vec3(1.0f, 0.0f, 0.0f));
            rightHandRot = rightHandRot*rightHandSwing;
        }
    } else if ( rightArmAnimState == Arm_Blocking ) {
        
    } else if ( rightArmAnimState == Arm_Swinging ) {
        float swingTime = fminf(1.0f, (time-rHandTimer)*4.0f);
//        swingTime *= swingTime;
        if ( swingTime == 1.0f ) {
            rightArmAnimState = Arm_Idle;
            rHandTimer = 0.0f;
            if ( rightHandItem ) {
                rightHandItem->GetAttributeDataPtr<glm::vec3>("velocity") = glm::vec3(0.0f,0.0f,0.0f);
                rightHandItem->GetAttributeDataPtr<bool>("generateCollisions") = false;
            }
        } else {
            float swingAngle = toRads(135.0f)+std::sin(swingTime)*toRads(225.0f);
            glm::quat rightHandSwing = glm::angleAxis(swingAngle, glm::vec3(1.0f, 0.0f, 0.0f));
            rightHandRot = rightHandRot*rightHandSwing;
        }
    } else if ( rightArmAnimState == Arm_Throwing ) {
        float swingTime = fminf(1.0f, (time-rHandTimer)*8.0f);
        float swingAngle = toRads(135.0f)+std::sin(1.0-swingTime)*toRads(225.0f);
        glm::quat rightHandSwing = glm::angleAxis(swingAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        rightHandRot = rightHandRot*rightHandSwing;
    }
    
    leftFoot->rotation = torsoRotation*leftFootRot;
    rightFoot->rotation = torsoRotation*rightFootRot;

    glm::quat hipRot = glm::angleAxis(hipRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat tiltRot = glm::angleAxis(torsoTiltAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    
    torso->rotation = torsoRotation*hipRot*tiltRot;
    
    // Slerp hand rotations
    leftHand->rotation = glm::slerp(leftHand->rotation, torsoRotation*hipRot*leftHandRot, leftHandSlerp);
    rightHand->rotation = glm::slerp(rightHand->rotation, torsoRotation*hipRot*rightHandRot, rightHandSlerp);

//    leftHand->rotation = torsoRotation*hipRot*leftHandRot;
//    rightHand->rotation = torsoRotation*hipRot*rightHandRot;
    
    m_owner->GetAttributeDataPtr<glm::quat>("rotation") = torsoRotation;

    m_owner->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(playerPosition.x(), playerPosition.y(), playerPosition.z() );
    
    glm::vec3 centerPos = glm::vec3(playerPosition.x(), playerPosition.y(), playerPosition.z());
    glm::vec3 torsoPos  = glm::vec3( 0.0f,  0.0f+torsoBobAmount, 0.0f );
    glm::vec3 headPos   = glm::vec3( 0.0f, 30*scaleCube+torsoBobAmount, 0.0f );
    glm::vec3 footLPos  = glm::vec3( 8*scaleCube, -14*scaleCube, 0.0f );
    glm::vec3 footRPos  = glm::vec3(-8*scaleCube, -14*scaleCube, 0.0f );
    glm::vec3 handLPos  = glm::vec3( 14*scaleCube, torsoBobAmount, 0.0f );
    glm::vec3 handRPos  = glm::vec3(-14*scaleCube, torsoBobAmount, 0.0f );
    
    headPos = (torsoRotation*tiltRot)*headPos;
    footLPos = torsoRotation*footLPos;
    footRPos = torsoRotation*footRPos;
    handLPos = torsoRotation*handLPos;
    handRPos = torsoRotation*handRPos;
    
    head->position = glm::vec3(centerPos+headPos);
    
    // Head accessory (hats, hair, etc)
    if ( headAccessoryID != -1 ) {
        Cubeject* headAccObj = m_manager->world->GetObject(headAccessoryObject);
        if ( headAccObj ) {
            InstanceData* headAccessory = headAccObj->GetInstance(headAccessoryID);
            if ( headAccessory ) {
                headAccessory->position = glm::vec3(centerPos+headPos+glm::vec3(0.0f,2*scaleCube,0.0f));
                headAccessory->rotation = torsoRotation;
            }
        }
    }
    torso->position = centerPos+torsoPos;
    leftFoot->position = glm::vec3(centerPos+footLPos);
    rightFoot->position = glm::vec3(centerPos+footRPos);
    leftHand->position = glm::vec3(centerPos+handLPos);
    rightHand->position = glm::vec3(centerPos+handRPos);
    if ( rightHandItem ) {
        glm::vec3 rHandItemPos = glm::vec3( -14*scaleCube, torsoBobAmount, 0.0f );
        rHandItemPos += glm::vec3( -2*scaleCube, -12*scaleCube, 22*scaleCube );
        rHandItemPos = rightHand->rotation*rHandItemPos;
        rightHandItem->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(centerPos+rHandItemPos);
        glm::quat itemrot = glm::angleAxis(toRads(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        rightHandItem->GetAttributeDataPtr<glm::quat>("rotation") = rightHand->rotation*itemrot;
    }
    // Head looking (slightly) in camera direction
    head->rotation = torsoRotation;
    
    if ( backPack != NULL
        ) {
        backPack->GetAttributeDataPtr<glm::quat>("rotation") = torsoRotation*hipRot*tiltRot;
        backPack->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(centerPos+torsoPos);
    }
}



const void HumanoidComponent::Rotate( const float rotX, const float rotY ) {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    glm::quat& rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
    glm::quat xRot = glm::angleAxis(rotX, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yRot = glm::angleAxis(rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = rotation*xRot;
    rotation = rotation*yRot;
    Rotate(rotation);
}
void HumanoidComponent::Rotate( glm::quat orientation ) {
    btQuaternion rotation = btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w);
    ghostObject->getWorldTransform().setRotation(rotation);
}
void HumanoidComponent::Warp( glm::vec3 position ) {
    character->warp(btVector3(position.x,position.y,position.z));
}


void HumanoidComponent::Wield( Entity *wieldObject ) {
    if ( rightHandItem ) { Store(rightHandItem); rightHandItem = NULL; }
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    if ( !wieldObject  || wieldObject == m_owner ) return; // Can't grab yourself
    int wieldObjectID = wieldObject->GetAttributeDataPtr<int>("ID");
    CubeComponent* cubeComp = (CubeComponent*)m_manager->GetComponent(wieldObjectID, "Cube");
    if ( cubeComp ) cubeComp->LoadObject();
    PhysicsComponent* pComp = (PhysicsComponent*)m_manager->GetComponent(wieldObjectID, "Physics");
    if ( pComp ) { pComp->SetPhysicsMode( Physics_Dynamic_AABBs, true ); }
    ParticleComponent* particleComp = (ParticleComponent*)m_manager->GetComponent(wieldObjectID, "Particle");
    if ( particleComp ) particleComp->Activate();
    Light3DComponent* light3DComp = (Light3DComponent*)m_manager->GetComponent(wieldObjectID, "Light3D");
    if ( light3DComp ) light3DComp->Activate();
    
    rightHandItem = wieldObject;
    const int ownerID = m_owner->GetAttributeDataPtr<int>("ID");
    const int itemID = rightHandItem->GetAttributeDataPtr<int>("ID");
    rightHandItem->GetAttributeDataPtr<int>("ownerID") = ownerID;
    m_owner->GetAttributeDataPtr<int>("rightHandItemID") = itemID;
}
void HumanoidComponent::Grab( Entity* grabbedObject ) {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    if ( !grabbedObject  || grabbedObject == m_owner ) return; // Can't grab yourself
    // Check for known types of items to grab
    if ( grabbedObject->GetAttributeDataPtr<int>("type") != ENTITY_ITEM ) { return; }
    const int itemType = grabbedObject->GetAttributeDataPtr<int>("itemType");
    if ( itemType == Item_Backpack && backPack == NULL ) {      // Grabbed backpack
        backPack = grabbedObject;
        const int backPackID = backPack->GetAttributeDataPtr<int>("ID");
        backPack->GetAttributeDataPtr<int>("ownerID") = m_ownerID;
        PhysicsComponent* pComp = (PhysicsComponent*)m_manager->GetComponent(backPackID, "Physics");
        if ( pComp ) { pComp->SetPhysicsMode( Physics_Off ); }
    } else {
        if ( !backPack ) {
            if ( !rightHandItem ) Wield(grabbedObject);
        } else {
            Store(grabbedObject);
        }
    }
}
void HumanoidComponent::Store( Entity *storedObject ) {
    if ( backPack ) { // Put item in backpack
        const int backPackID = backPack->GetAttributeDataPtr<int>("ID");
        InventoryComponent* inventoryC = (InventoryComponent*)m_manager->GetComponent(backPackID, "Inventory");
        if ( inventoryC ) {
            const int objectID = storedObject->GetAttributeDataPtr<int>("ID");
            PhysicsComponent* pComp = (PhysicsComponent*)m_manager->GetComponent(objectID, "Physics");
            if ( pComp ) { pComp->SetPhysicsMode( Physics_Off ); }
            storedObject->GetAttributeDataPtr<int>("ownerID") = m_ownerID;
            CubeComponent* cubeComp = (CubeComponent*)m_manager->GetComponent(objectID, "Cube");
            if ( cubeComp ) cubeComp->UnloadObject();
            ParticleComponent* particleComp = (ParticleComponent*)m_manager->GetComponent(objectID, "Particle");
            if ( particleComp ) particleComp->DeActivate();
            Light3DComponent* light3DComp = (Light3DComponent*)m_manager->GetComponent(objectID, "Light3D");
            if ( light3DComp ) light3DComp->DeActivate();
            inventoryC->AddItem(storedObject);
        }
    }
}
void HumanoidComponent::ThrowStart() {
    rHandTimer = Timer::Seconds();
    rightArmAnimState = Arm_Throwing;
}
void HumanoidComponent::ThrowItem( const glm::vec3 targetPos ) {
    if ( rHandTimer == 0.0 ) return;
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    const double timeNow = Timer::Seconds();
    const float strength = fminf(timeNow-rHandTimer, 1.0f)*100.0f;
    int rightArmID = m_owner->GetAttributeDataPtr<int>("rightArmID");
    std::string rightArmObject = m_owner->GetAttributeDataPtr<std::string>("rightArmObject");
    Cubeject* rightArmObj = m_manager->world->GetObject(rightArmObject);
    InstanceData* rightHand = rightArmObj->GetInstance(rightArmID);
    glm::vec3 pos = rightHand->position;
    
    glm::vec3 dir = targetPos-pos;
    glm::vec3 vel = glm::normalize(dir);
    btVector3 newVel = btVector3(vel.x,vel.y,vel.z);
    btVector3 newPos = btVector3(pos.x,pos.y,pos.z)+newVel*2.0f;
    newVel *= strength;
    if ( rightHandItem ) {  // Throw item in hand

        rightHandItem->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(newPos.x(),newPos.y(),newPos.z());
        const int rhiID = rightHandItem->GetAttributeDataPtr<int>("ID");
        PhysicsComponent* itemPhysComp = (PhysicsComponent*)m_manager->GetComponent(rhiID, "Physics");
        if ( itemPhysComp ) {
            itemPhysComp->SetPhysicsMode( Physics_Dynamic_AABBs, false );
            itemPhysComp->SetLinearVelocity(&newVel);
        }
        // If weapon type is axe or knife, add rotation
        if ( rightHandItem->GetAttributeDataPtr<int>("itemType") == Item_Weapon_Axe ) {
            btVector3 angVel = btVector3(strength,0.0f,0.0f)*strength;
            if ( itemPhysComp ) itemPhysComp->SetAngularVelocity(&angVel);
        } else if ( rightHandItem->GetAttributeDataPtr<int>("itemType") == Item_Grenade ) {
            ExplosiveComponent* explosive = (ExplosiveComponent*)m_manager->GetComponent(rhiID, "Explosive");
            if ( explosive ) explosive->Activate();
        }
        rightHandItem->GetAttributeDataPtr<glm::vec3>("velocity") = glm::vec3(0,0,0);
        rightHandItem->GetAttributeDataPtr<bool>("generateCollisions") = true;
        rightHandItem->GetAttributeDataPtr<int>("ownerID") = -1;
        rightHandItem = NULL;
        m_owner->GetAttributeDataPtr<int>("rightHandItemID") = -1;
        
    } else {    // Throw rock
        const float rockRadius = BLOCK_RADIUS;
        DynaCube* cube = m_manager->world->AddDynaCube(newPos, btVector3(rockRadius,rockRadius,rockRadius), ColorForType(Type_Rock));
        cube->SetVelocity(newVel);
        cube->timer = 10.0;
    }
    rHandTimer = 0.0;
    rightArmAnimState = Arm_Idle;
}
void HumanoidComponent::UseRightHand() {
    rHandTimer = Timer::Seconds();
    if ( rightHandItem ) {
        if ( rightHandItem->GetAttributeDataPtr<int>("itemType") == Item_Potion_Health ) {
            rightArmAnimState = Arm_Holding;
        } else {
            rightArmAnimState = Arm_Swinging;
            rightHandItem->GetAttributeDataPtr<glm::vec3>("velocity") = glm::vec3(0,10.0f,0);
            bool& generate = rightHandItem->GetAttributeDataPtr<bool>("generateCollisions");
            generate  = true;
//            bool gen = rightHandItem->GetAttributeDataPtr<bool>("generateCollisions");
        }
    }
}
void HumanoidComponent::Die() {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    int torsoID = m_owner->GetAttributeDataPtr<int>("torsoID");
    int headID = m_owner->GetAttributeDataPtr<int>("headID");
    int headAccessoryID = m_owner->GetAttributeDataPtr<int>("headAccessoryID");
    int leftFootID = m_owner->GetAttributeDataPtr<int>("leftFootID");
    int rightFootID = m_owner->GetAttributeDataPtr<int>("rightFootID");
    int leftHandID = m_owner->GetAttributeDataPtr<int>("leftArmID");
    int rightHandID = m_owner->GetAttributeDataPtr<int>("rightArmID");
    std::string torsoObject = m_owner->GetAttributeDataPtr<std::string>("torsoObject");
    std::string headObject = m_owner->GetAttributeDataPtr<std::string>("headObject");
    std::string leftFootObject = m_owner->GetAttributeDataPtr<std::string>("leftFootObject");
    std::string rightFootObject = m_owner->GetAttributeDataPtr<std::string>("rightFootObject");
    std::string leftHandObject = m_owner->GetAttributeDataPtr<std::string>("leftArmObject");
    std::string rightHandObject = m_owner->GetAttributeDataPtr<std::string>("rightArmObject");
    
    Cubeject* torsoObj = m_manager->world->GetObject(torsoObject);
    Cubeject* headObj = m_manager->world->GetObject(headObject);
    Cubeject* leftFootObj = m_manager->world->GetObject(leftFootObject);
    Cubeject* rightFootObj = m_manager->world->GetObject(rightFootObject);
    Cubeject* leftHandObj = m_manager->world->GetObject(leftHandObject);
    Cubeject* rightHandObj = m_manager->world->GetObject(rightHandObject);
    
    InstanceData* torso       = torsoObj->GetInstance(torsoID);
    InstanceData* head        = headObj->GetInstance(headID);
    InstanceData* leftFoot    = leftFootObj->GetInstance(leftFootID);
    InstanceData* rightFoot   = rightFootObj->GetInstance(rightFootID);
    InstanceData* leftHand    = leftHandObj->GetInstance(leftHandID);
    InstanceData* rightHand   = rightHandObj->GetInstance(rightHandID);
    
    if ( headAccessoryID != -1 ) {
        std::string headAccessoryObject = m_owner->GetAttributeDataPtr<std::string>("headAccessoryObject");
        Cubeject* headAccObj = m_manager->world->GetObject(headAccessoryObject);
        if ( headAccObj ) {
            InstanceData* headAccessory = headAccObj->GetInstance(headAccessoryID);
            if ( headAccessory ) {
                m_manager->world->AddDecor(headAccessoryObject, headAccessory->position, headAccessory->rotation);
//                headAccObj->cubes->GetDynaBlocks(m_owner->world, headAccessory->position, headAccessory->scale.x);
                headAccObj->RemoveInstance(headAccessoryID);
            }
        }
    }

    if ( torso ) {
        m_manager->world->AddDecor(torsoObject, torso->position, torso->rotation);
        torsoObj->RemoveInstance(torsoID);
    }
    if ( head ) {
        m_manager->world->AddDecor(headObject, head->position, head->rotation);
//        headObj->cubes->GetDynaBlocks(m_owner->world, head->position, head->scale.x);
        headObj->RemoveInstance(headID);
    }
    if ( leftFoot ) {
        m_manager->world->AddDecor(leftFootObject, leftFoot->position, leftFoot->rotation);
        leftFootObj->RemoveInstance(leftFootID);
    }
    if ( rightFoot ) {
        m_manager->world->AddDecor(rightFootObject, rightFoot->position, rightFoot->rotation);
        rightFootObj->RemoveInstance(rightFootID);
    }
    if ( leftHand ) {
        m_manager->world->AddDecor(leftHandObject, leftHand->position, leftHand->rotation);
        leftHandObj->RemoveInstance(leftHandID);
    }
    
    if ( rightHandItem ) {
        glm::vec3 pos = rightHand->position;
        ThrowItem(pos+glm::vec3(0.0f,1.0f,0.0f));
    }
    if ( rightHand ) {
        m_manager->world->AddDecor(rightHandObject, rightHand->position, rightHand->rotation);
        rightHandObj->RemoveInstance(rightHandID);
    }

    m_manager->world->entityMan->KillEntity(m_ownerID);
}



