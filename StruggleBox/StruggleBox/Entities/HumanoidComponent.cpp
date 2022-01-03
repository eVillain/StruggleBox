#include "HumanoidComponent.h"

#include "VoxelFactory.h"
#include "EntityManager.h"
#include "Renderer.h"
#include "PhysicsComponent.h"
#include "ParticleComponent.h"
#include "ItemComponent.h"
#include "InventoryComponent.h"
#include "VoxelComponent.h"
#include "ExplosiveComponent.h"
#include "Light3DComponent.h"
#include "HealthComponent.h"
#include "Physics.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "Timer.h"
#include <math.h>       /* sin */
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>

const bool useCapsuleShape = true;

HumanoidComponent::HumanoidComponent(
	const int ownerID,
	EntityManager& entityManager,
	Physics& physics,
	VoxelFactory& voxels) 
    : EntityComponent(ownerID, "Humanoid")
    , _entityManager(entityManager)
    , _physics(physics)
    , _voxels(voxels)
    , m_backpack(nullptr)
    , m_rightHandItem(nullptr)
    , m_leftHandItem(nullptr)
{    
    Entity* m_owner = _entityManager.getEntity(_ownerID);
    
    physicsWorld = _physics.getWorld();
    
    sizeScale           = 0.1f;
    float cubeRadius = DEFAULT_VOXEL_MESHING_WIDTH*sizeScale;
    characterRadius     = cubeRadius*16.0f;
    characterHeight     = cubeRadius*64.0f-(characterRadius*2.0f);
    walkSpeed           = 0.125f;
//    throwTimer          = 0.0;
    rHandTimer          = 0.0;
    lHandTimer          = 0.0;
    
    btScalar stepHeight = btScalar(0.5f);

    btTransform startTransform;
	startTransform.setIdentity ();
    const glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
	startTransform.setOrigin (btVector3(position.x, position.y, position.z));
    
    ghostObject = _physics.createGhostObject();
	ghostObject->setWorldTransform(startTransform);
    ghostObject->setUserPointer(m_owner);
    if ( useCapsuleShape )
    {
        uint32_t capsuleID = _physics.createCapsule(characterRadius, characterHeight);
        btCapsuleShape* capsule = (btCapsuleShape*)_physics.getShapeForID(capsuleID);
        ghostObject->setCollisionShape(capsule);
        ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
        character = _physics.createCharacterController(ghostObject, capsule, stepHeight);
    }
    else 
    {
        btConvexShape* boxShape = (btBoxShape*)_physics.createBox(characterRadius, characterHeight, characterRadius);
        ghostObject->setCollisionShape (boxShape);
        ghostObject->setCollisionFlags (btCollisionObject::CF_CHARACTER_OBJECT);
        character = _physics.createCharacterController(ghostObject, boxShape, stepHeight);
    }

	character->setFallSpeed(50.0f);
	character->setJumpSpeed(12.0f);
	character->setMaxJumpHeight(1.0f);
    
    physicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	physicsWorld->addCollisionObject(
        ghostObject,
        (short)CollisionType::Group_Entity,
        (short)CollisionType::Filter_Everything);
	physicsWorld->addAction(character);
    
    // Variables
    glm::quat rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
    Rotate(rotation);
    // Load player model files
    const int charType = m_owner->GetAttributeDataPtr<int>("type");
    setCharacterType(charType);
    
    m_owner->GetAttributeDataPtr<int>("backpackItemID") = 0;
    m_owner->GetAttributeDataPtr<int>("rightHandItemID") = 0;
    m_owner->GetAttributeDataPtr<int>("leftHandItemID") = 0;
    m_owner->GetAttributeDataPtr<int>("ownerID") = 0;

    glm::vec3& bb = m_owner->GetAttributeDataPtr<glm::vec3>("bb");
    bb = glm::vec3(characterRadius, characterHeight, characterRadius);
    
    leftArmAnimState = ArmState::Arm_Idle;
    rightArmAnimState = ArmState::Arm_Idle;
    legsAnimState = Legs_Idle;
    
    hipRotationAngle = 0.0f;
    torsoTiltAngle = 0.0f;
    torsoLeanAngle = 0.0f;
    torsoBobAmount = 0.0f;
    
    if (!m_owner->HasAttribute("health"))
    {
        m_owner->GetAttributeDataPtr<int>("health") = 100;
    }
}

HumanoidComponent::~HumanoidComponent()
{
}

void HumanoidComponent::setCharacterType( const int newType )
{
    removeAllVoxelMeshInstances(false);

    Entity* m_owner = _entityManager.getEntity(_ownerID);

    const glm::vec3 scale = glm::vec3(sizeScale, sizeScale, sizeScale);
    const float scaleCube = DEFAULT_VOXEL_MESHING_WIDTH*sizeScale;
    const glm::vec3 torsoPos  = glm::vec3( 0.0f,  0.0f, 0.0f );
    const glm::vec3 headPos   = glm::vec3( 0.0f, 26*scaleCube, 0.0f );
    const glm::vec3 footLPos  = glm::vec3( 8*scaleCube, -14*scaleCube, 0.0f );
    const glm::vec3 footRPos  = glm::vec3(-8*scaleCube, -14*scaleCube, 0.0f );
    const glm::vec3 handLPos  = glm::vec3( 14*scaleCube, 0.0f, 0.0f );
    const glm::vec3 handRPos  = glm::vec3(-14*scaleCube, 0.0f, 0.0f );

    int& torsoID = m_owner->GetAttributeDataPtr<int>("torsoID");
    int& headID = m_owner->GetAttributeDataPtr<int>("headID");
    int& leftFootID = m_owner->GetAttributeDataPtr<int>("leftFootID");
    int& rightFootID = m_owner->GetAttributeDataPtr<int>("rightFootID");
    int& leftArmID = m_owner->GetAttributeDataPtr<int>("leftArmID");
    int& rightArmID = m_owner->GetAttributeDataPtr<int>("rightArmID");

    const std::string& torsoObject = m_owner->GetAttributeDataPtr<std::string>("torsoObject");
    const std::string& headObject = m_owner->GetAttributeDataPtr<std::string>("headObject");
    const std::string& leftFootObject = m_owner->GetAttributeDataPtr<std::string>("leftFootObject");
    const std::string& rightFootObject = m_owner->GetAttributeDataPtr<std::string>("rightFootObject");
    const std::string& leftArmObject = m_owner->GetAttributeDataPtr<std::string>("leftArmObject");
    const std::string& rightArmObject = m_owner->GetAttributeDataPtr<std::string>("rightArmObject");

	// Load the object instances
	torsoID = _voxels.getMesh(torsoObject)->addInstance(torsoPos, glm::quat(), scale);
	headID = _voxels.getMesh(headObject)->addInstance(headPos, glm::quat(), scale);
	leftFootID = _voxels.getMesh(leftFootObject)->addInstance(footLPos, glm::quat(), scale);
	rightFootID = _voxels.getMesh(rightFootObject)->addInstance(footRPos, glm::quat(), scale);
	leftArmID = _voxels.getMesh(leftArmObject)->addInstance(handLPos, glm::quat(), scale);
	rightArmID = _voxels.getMesh(rightArmObject)->addInstance(handRPos, glm::quat(), scale);
}

/// The update funtion takes input data and picks wanted animation states
void HumanoidComponent::update(const double delta)
{
    Entity* m_owner = _entityManager.getEntity(_ownerID);
    bool& jumping = m_owner->GetAttributeDataPtr<bool>("jumping");
    bool& running = m_owner->GetAttributeDataPtr<bool>("running");
    bool& sneaking = m_owner->GetAttributeDataPtr<bool>("sneaking");
    glm::vec3& moveInput = m_owner->GetAttributeDataPtr<glm::vec3>("moveInput");        // First person movement input
    glm::vec3& direction = m_owner->GetAttributeDataPtr<glm::vec3>("direction");        // Third person movement input
 
    btScalar walkFactor = 1.0f;
    if ( running && sneaking ) walkFactor *= 0.25f;
    else if ( running ) walkFactor *= 2.0f;
    else if ( sneaking ) walkFactor *= 0.5f;

    // Update object positions and rotations based on physics transform
    if ( moveInput.x || moveInput.z )
    {
        // First person controls !!!UNTESTED!!!
        const btQuaternion playerRotation = ghostObject->getWorldTransform().getRotation();
        glm::quat oldRot = glm::quat(playerRotation.w(), playerRotation.x(), playerRotation.y(), playerRotation.z());
        glm::vec3 moveDir = moveInput*oldRot;
        character->setWalkDirection(btVector3(moveDir.x,moveDir.y,moveDir.z)*walkFactor*walkSpeed);
    }
    else 
    {
        // Third person controls
        const btTransform trans = ghostObject->getWorldTransform();
        const btQuaternion playerRotation = trans.getRotation();
        glm::quat newRot = glm::quat(playerRotation.w(), playerRotation.x(), playerRotation.y(), playerRotation.z());

        torsoLeanAngle = 0.0f;
        
        bool grounded = character->onGround();
        if (!grounded)
        {
            legsAnimState = Legs_Jumping;
            walkFactor *= 0.8f; // Slower air control
        }
        else
        {
            if (jumping)
            {
                character->jump();
            }

            if (direction.x == 0.0f && direction.z == 0.0f)
            {
                if (!jumping)
                {
                    // No movement input, not moving, set legs to idle
                    legsAnimState = Legs_Idle;
                }
            }
            else
            {
                // Calculate new wanted angle from movement direction
                const float wantedAngle = atan2(direction.x, direction.z);
                newRot = glm::angleAxis(wantedAngle, glm::vec3(0.0f, 1.0f, 0.0f));

                // If running slow down turn and lean forward
                if (running && !sneaking) 
                {
                    float torsoLeanFactor = toRads(30.0f);
                    // Amount of rotation slerp, controls speed of rotation
                    float runRotSlerp = 0.4f;
                    if ( running ) {
                        runRotSlerp = 0.2f;
                        torsoLeanFactor = toRads(90.0f);
                    }
                    const glm::quat oldRot = glm::quat(playerRotation.w(), playerRotation.x(), playerRotation.y(), playerRotation.z());
                    const glm::quat wantedRot = glm::angleAxis(wantedAngle, glm::vec3(0.0f, 1.0f, 0.0f));
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
        
        if ( delta != 0.0 ) 
		{        
            // Update movement and rotation
            btQuaternion newRotation = btQuaternion(newRot.x, newRot.y, newRot.z, newRot.w);
			ghostObject->getWorldTransform().setRotation(newRotation);
			//printf("Player rotation:%f,%f,%f,%f\n", newRot.x,newRot.y,newRot.z,newRot.w);
            const glm::vec3 walkDir = newRot*glm::vec3(0.0,0.0,glm::length(glm::vec2(direction.x, direction.z)));
            // Move physics character
            character->setWalkDirection(btVector3(walkDir.x, walkDir.y, walkDir.z)*walkFactor*walkSpeed);

            // Update object positions and rotations based on physics transform
            const btTransform trans = ghostObject->getWorldTransform();
            m_owner->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z());
            m_owner->GetAttributeDataPtr<glm::quat>("rotation") = glm::quat(trans.getRotation().w(), trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z());
        }
        else 
        {    
            // Game is paused, force move player instead of letting physics do the moving
            const btVector3 playerPosition = trans.getOrigin();
            const float epsilon = 0.00001f;
            const glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
            if ( fabsf(position.x - playerPosition.x()) > epsilon ||
                 fabsf(position.y - playerPosition.y()) > epsilon ||
                 fabsf(position.z - playerPosition.z()) > epsilon ) {
                character->warp(btVector3(position.x, position.y, position.z));
            }
            const glm::quat rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
            if ( fabsf( rotation.x - playerRotation.w() ) > epsilon ||
                 fabsf( rotation.y - playerRotation.x() ) > epsilon ||
                 fabsf( rotation.z - playerRotation.y() ) > epsilon ||
                 fabsf( rotation.w - playerRotation.z() ) > epsilon )  {
                ghostObject->getWorldTransform().setRotation(btQuaternion(rotation.x,rotation.y,rotation.z, rotation.w));
            }
        }
    }
    updateAnimations(delta);
}

void HumanoidComponent::updateAnimations(double delta)
{
    Entity* m_owner = _entityManager.getEntity(_ownerID);
    const int torsoID = m_owner->GetAttributeDataPtr<int>("torsoID");
    const int headID = m_owner->GetAttributeDataPtr<int>("headID");
    const int leftFootID = m_owner->GetAttributeDataPtr<int>("leftFootID");
    const int rightFootID = m_owner->GetAttributeDataPtr<int>("rightFootID");
    const int leftHandID = m_owner->GetAttributeDataPtr<int>("leftArmID");
    const int rightHandID = m_owner->GetAttributeDataPtr<int>("rightArmID");
    const std::string torsoObject = m_owner->GetAttributeDataPtr<std::string>("torsoObject");
    const std::string headObject = m_owner->GetAttributeDataPtr<std::string>("headObject");
    const std::string leftFootObject = m_owner->GetAttributeDataPtr<std::string>("leftFootObject");
    const std::string rightFootObject = m_owner->GetAttributeDataPtr<std::string>("rightFootObject");
    const std::string leftHandObject = m_owner->GetAttributeDataPtr<std::string>("leftArmObject");
    const std::string rightHandObject = m_owner->GetAttributeDataPtr<std::string>("rightArmObject");
    const float lifeTime = m_owner->GetAttributeDataPtr<float>("lifeTime");

    const float animationSpeed = 20.0f;
    const float scaleCube = DEFAULT_VOXEL_MESHING_WIDTH*sizeScale;

    const glm::vec3 playerPosition = m_owner->GetAttributeDataPtr<glm::vec3>("position");
    glm::quat torsoRotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");

    // Walkangles
    float leftFootAngle = std::sin(lifeTime * animationSpeed);
    float rightFootAngle = std::sin(-lifeTime * animationSpeed);
    torsoBobAmount = 0.0f;
    hipRotationAngle = 0.0f;
    torsoTiltAngle = 0.0f;

    // Look at general body motion first ( = what the legs are doing )
    if (legsAnimState == Legs_Idle) {
        torsoBobAmount = std::sin(lifeTime *10.0f)*0.01f;
        float newF_ratio = 1.0f*delta*10.0f;
        float oldF_ratio = 1.0f - newF_ratio;
        leftFootAngle = _voxels.getMesh(leftFootObject)->getRotation(leftFootID).x*oldF_ratio;
        rightFootAngle = _voxels.getMesh(rightFootObject)->getRotation(rightFootID).x*oldF_ratio;
    } else if ( legsAnimState == Legs_Sneaking ) {
        leftFootAngle *= toRads(15.0f);
        rightFootAngle *= toRads(15.0f);
        hipRotationAngle = std::sin(lifeTime *animationSpeed*0.25f)*toRads(5.0f);
        torsoBobAmount += std::sin(lifeTime *animationSpeed)*0.01f;
    } else if ( legsAnimState == Legs_Walking ) {
        torsoTiltAngle = toRads(2.5f);
        leftFootAngle *= toRads(45.0f);
        rightFootAngle *= toRads(45.0f);
        hipRotationAngle = std::sin(lifeTime *animationSpeed*0.5f)*toRads(5.0f);
        torsoBobAmount += std::sin(lifeTime *animationSpeed)*0.02f;
    } else if ( legsAnimState == Legs_Running ) {
        torsoTiltAngle = toRads(10.0f);
        leftFootAngle *= toRads(60.0f);
        rightFootAngle *= toRads(60.0f);
        hipRotationAngle = std::sin(lifeTime *animationSpeed)*toRads(5.0f);
        torsoBobAmount += std::sin(lifeTime *animationSpeed)*0.03f;
    } else if ( legsAnimState == Legs_Jumping ) {
        float newF_ratio = 0.9f *delta*10.0f;
        float oldF_ratio = 1.0f - newF_ratio;
        leftFootAngle = (_voxels.getMesh(leftFootObject)->getRotation(leftFootID).x*oldF_ratio)+(toRads(60.0f)*newF_ratio);
        rightFootAngle = (_voxels.getMesh(rightFootObject)->getRotation(rightFootID).x*oldF_ratio)+(toRads(60.0f)*newF_ratio);
    }
    torsoRotation = torsoRotation* glm::angleAxis(torsoLeanAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::quat leftFootRot = glm::angleAxis(leftFootAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat rightFootRot = glm::angleAxis(rightFootAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat leftHandRot = glm::angleAxis(-0.25f*leftFootAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat rightHandRot = glm::angleAxis(-0.25f*rightFootAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    
    float leftHandSlerp = 1.0f;
    float rightHandSlerp = 1.0f;
    // Now find parameters for the arms
    if ( leftArmAnimState == ArmState::Arm_Idle ) {
        bool& running = m_owner->GetAttributeDataPtr<bool>("running");
        if ( running ) {
            leftHandSlerp = 0.3f;
            glm::quat leftHandLift = glm::angleAxis(toRads(35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            leftHandRot = leftHandRot*leftHandLift;
        } else { // Regular slerp of rotation
            leftHandSlerp = 0.5f;
        }
    } else if ( leftArmAnimState == ArmState::Arm_Holding ) {
        
    } else if ( leftArmAnimState == ArmState::Arm_Blocking ) {
        
    } else if ( leftArmAnimState == ArmState::Arm_Swinging ) {
        
    } else if ( leftArmAnimState == ArmState::Arm_Throwing ) {
        
    }
    
    if ( rightArmAnimState == ArmState::Arm_Idle ) {
        bool& running = m_owner->GetAttributeDataPtr<bool>("running");
        if ( running ) {
            rightHandSlerp = 0.3f;
            glm::quat rightHandLift = glm::angleAxis(toRads(-35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            rightHandRot = rightHandRot*rightHandLift;
        } else { // Regular slerp of rotation
            rightHandSlerp = 0.5f;
        }
    } else if ( rightArmAnimState == ArmState::Arm_Holding ) {
        float swingTime = fminf(1.0f, (rHandTimer)*5.0f);
        if ( swingTime == 1.0f ) {
            rightArmAnimState = ArmState::Arm_Idle;
            rHandTimer = 0.0f;
            if (m_rightHandItem)
            {
                if ( m_rightHandItem->GetAttributeDataPtr<int>("itemType") == Item_Potion_Health )
                {
                    // Use potion and delete it from inventory
                    int potionID = m_rightHandItem->GetAttributeDataPtr<int>("ID");
                    int health = m_rightHandItem->GetAttributeDataPtr<int>("health");
                    HealthComponent* hc = (HealthComponent*)_entityManager.getComponent(potionID, "Health");
                    if ( hc ) hc->addHealth( health );
                    _entityManager.destroyEntity(potionID);
                    m_rightHandItem = NULL;
                }
            }
        } else {
            float swingAngle = toRads(135.0f)+std::sin(1.0-swingTime)*toRads(225.0f);
            glm::quat rightHandSwing = glm::angleAxis(swingAngle, glm::vec3(1.0f, 0.0f, 0.0f));
            rightHandRot = rightHandRot*rightHandSwing;
        }
    } else if ( rightArmAnimState == ArmState::Arm_Blocking ) {
        
    } else if ( rightArmAnimState == ArmState::Arm_Swinging ) {
        float swingTime = fminf(1.0f, (rHandTimer)*4.0f);
//        swingTime *= swingTime;
        if ( swingTime == 1.0f ) {
            rightArmAnimState = ArmState::Arm_Idle;
            rHandTimer = 0.0f;
            if ( m_rightHandItem ) {
                m_rightHandItem->GetAttributeDataPtr<glm::vec3>("velocity") = glm::vec3(0.0f,0.0f,0.0f);
                m_rightHandItem->GetAttributeDataPtr<bool>("generateCollisions") = false;
            }
        } else {
            float swingAngle = toRads(135.0f)+std::sin(swingTime)*toRads(225.0f);
            glm::quat rightHandSwing = glm::angleAxis(swingAngle, glm::vec3(1.0f, 0.0f, 0.0f));
            rightHandRot = rightHandRot*rightHandSwing;
        }
    } else if ( rightArmAnimState == ArmState::Arm_Throwing ) {
        float swingTime = fminf(1.0f, (lifeTime -rHandTimer)*8.0f);
        float swingAngle = toRads(135.0f)+std::sin(1.0-swingTime)*toRads(225.0f);
        glm::quat rightHandSwing = glm::angleAxis(swingAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        rightHandRot = rightHandRot*rightHandSwing;
    }
    
	_voxels.getMesh(leftFootObject)->setRotation(torsoRotation*leftFootRot, leftFootID);
	_voxels.getMesh(rightFootObject)->setRotation(torsoRotation*rightFootRot, rightFootID);

    glm::quat hipRot = glm::angleAxis(hipRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat tiltRot = glm::angleAxis(torsoTiltAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    
	_voxels.getMesh(torsoObject)->setRotation(torsoRotation*hipRot*tiltRot, torsoID);
    
    // Slerp hand rotations
	leftHandRot = glm::slerp(_voxels.getMesh(leftHandObject)->getRotation(leftHandID), torsoRotation*hipRot*leftHandRot, leftHandSlerp);
	rightHandRot = glm::slerp(_voxels.getMesh(rightHandObject)->getRotation(rightHandID), torsoRotation*hipRot*rightHandRot, rightHandSlerp);
	_voxels.getMesh(leftHandObject)->setRotation(leftHandRot, leftHandID);
	_voxels.getMesh(rightHandObject)->setRotation(rightHandRot, rightHandID);
        
    glm::vec3 centerPos = playerPosition;
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
    
	_voxels.getMesh(headObject)->setPosition(centerPos + headPos, headID);
	_voxels.getMesh(torsoObject)->setPosition(centerPos + torsoPos, torsoID);
	_voxels.getMesh(leftFootObject)->setPosition(centerPos + footLPos, leftFootID);
	_voxels.getMesh(rightFootObject)->setPosition(centerPos + footRPos, rightFootID);
	_voxels.getMesh(leftHandObject)->setPosition(centerPos + handLPos, leftHandID);
	_voxels.getMesh(rightHandObject)->setPosition(centerPos + handRPos, rightHandID);
    if (m_rightHandItem)
    {
        const glm::vec3 rHandGripOffset = m_rightHandItem->GetAttributeDataPtr<glm::vec3>("gripOffset");

        glm::vec3 rHandItemPos = glm::vec3( -14*scaleCube, torsoBobAmount, 0.0f ); // torso width and bob
        rHandItemPos += glm::vec3( -2*scaleCube, -12*scaleCube, 22*scaleCube ) + rHandGripOffset;
        rHandItemPos = rightHandRot*rHandItemPos;
        m_rightHandItem->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(centerPos+rHandItemPos);
        glm::quat itemrot = glm::angleAxis(toRads(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m_rightHandItem->GetAttributeDataPtr<glm::quat>("rotation") = rightHandRot*itemrot;
    }
    // Head looking (slightly) in camera direction
	_voxels.getMesh(headObject)->setRotation(torsoRotation, headID);
    
    if (m_backpack != nullptr)
    {
        m_backpack->GetAttributeDataPtr<glm::quat>("rotation") = torsoRotation*hipRot*tiltRot;
        m_backpack->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(centerPos+torsoPos);
    }
}

const void HumanoidComponent::Rotate(
	const float rotX,
	const float rotY )
{
    Entity* m_owner = _entityManager.getEntity(_ownerID);
    glm::quat& rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
    glm::quat xRot = glm::angleAxis(rotX, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yRot = glm::angleAxis(rotY, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = rotation*xRot;
    rotation = rotation*yRot;
    Rotate(rotation);
}

void HumanoidComponent::Rotate( glm::quat orientation )
{
    btQuaternion rotation = btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w);
    ghostObject->getWorldTransform().setRotation(rotation);

    Entity* m_owner = _entityManager.getEntity(_ownerID);
    m_owner->GetAttributeDataPtr<glm::quat>("rotation") = orientation;
}

void HumanoidComponent::Warp( glm::vec3 position )
{
    character->warp(btVector3(position.x,position.y,position.z));
}

void HumanoidComponent::Grab(Entity* grabbedObject)
{
    Entity* owner = _entityManager.getEntity(_ownerID);
    if (!grabbedObject  || grabbedObject == owner)
    {
        return; // Can't grab yourself
    }
    // Check for known types of items to grab
    // TODO: EXTEND TO PERHAPS ALLOW GRABBING PROJECTILES FROM MID-AIR :D
    if (grabbedObject->GetAttributeDataPtr<int>("type") != ENTITY_ITEM)
    {
        return;
    }
    const ItemType itemType = (ItemType)grabbedObject->GetAttributeDataPtr<int>("itemType");
    if (itemType == Item_Backpack && m_backpack == nullptr)
    {
        m_backpack = grabbedObject;
        const EntityID backPackID = (EntityID)m_backpack->GetAttributeDataPtr<int>("ID");
        owner->GetAttributeDataPtr<int>("backpackItemID") = backPackID;
        m_backpack->GetAttributeDataPtr<int>("ownerID") = _ownerID;
        if (PhysicsComponent* pComp = (PhysicsComponent*)_entityManager.getComponent(backPackID, "Physics"))
        { 
            pComp->setPhysicsMode(PhysicsMode::Physics_Off, false, false);
        }
        Log::Debug("Player %i grabbed backpack %i", _ownerID, backPackID);
    }
    else 
    {
        Wield(grabbedObject);
    }
}

void HumanoidComponent::Store(Entity *storedObject)
{
    if (!m_backpack)
    {
        return;
    }
    // Put item in backpack
    const EntityID backPackID = m_backpack->GetAttributeDataPtr<int>("ID");
    if (InventoryComponent* inventoryC = (InventoryComponent*)_entityManager.getComponent(backPackID, "Inventory"))
    {
        storedObject->GetAttributeDataPtr<int>("ownerID") = _ownerID;
        const EntityID objectID = storedObject->GetAttributeDataPtr<int>("ID");
        if (PhysicsComponent* pComp = (PhysicsComponent*)_entityManager.getComponent(objectID, "Physics"))
        {
            pComp->setPhysicsMode(PhysicsMode::Physics_Off, false, false);
        }
        if (VoxelComponent* cubeComp = (VoxelComponent*)_entityManager.getComponent(objectID, "Cube"))
        {
            cubeComp->unloadObject();
        }
        if (ParticleComponent* particleComp = (ParticleComponent*)_entityManager.getComponent(objectID, "Particle"))
        {
            particleComp->deActivate();
        }
        if (Light3DComponent* light3DComp = (Light3DComponent*)_entityManager.getComponent(objectID, "Light3D"))
        {
            light3DComp->deActivate();
        }
        inventoryC->addItem(storedObject);
        Log::Debug("Player %i stored item %i in backpack %i", _ownerID, objectID, backPackID);
    }
}

void HumanoidComponent::Drop(Entity* droppedObject)
{
    Entity* m_owner = _entityManager.getEntity(_ownerID);
    const glm::vec3 humanPos = m_owner->GetAttributeDataPtr<glm::vec3>("position");
    const glm::vec3 objPos = droppedObject->GetAttributeDataPtr<glm::vec3>("position");
    const glm::vec3 dir = objPos - humanPos;
    const glm::vec3 vel = glm::normalize(dir);

    const int objectID = droppedObject->GetAttributeDataPtr<int>("ID");
    PhysicsComponent* itemPhysComp = (PhysicsComponent*)_entityManager.getComponent(objectID, "Physics");
    if (itemPhysComp)
    {
        const btVector3 newVel = btVector3(vel.x, vel.y, vel.z);
        itemPhysComp->setPhysicsMode(PhysicsMode::Physics_Cube_AABBs, false, false);
        itemPhysComp->setLinearVelocity(&newVel);
    }

    droppedObject->GetAttributeDataPtr<glm::vec3>("velocity") = vel;
    droppedObject->GetAttributeDataPtr<bool>("generateCollisions") = true;
    droppedObject->GetAttributeDataPtr<int>("ownerID") = 0;

    Log::Debug("Player %i dropped object %i", _ownerID, objectID);
}

void HumanoidComponent::Wield(Entity* entityToWield)
{
    Entity* owner = _entityManager.getEntity(_ownerID);
    if (!entityToWield || entityToWield == owner)
    {
        return; // Can't grab yourself
    }

    if (m_rightHandItem)
    {
        if (m_backpack)
        {
            Store(m_rightHandItem);
        }
        else
        {
            Drop(m_rightHandItem);
        }
        m_rightHandItem = nullptr;
        owner->GetAttributeDataPtr<int>("rightHandItemID") = 0;
    }

    const int entityToWieldID = entityToWield->GetAttributeDataPtr<int>("ID");
    std::vector<EntityComponent*> components = _entityManager.getAllComponents(entityToWieldID);
    for (EntityComponent* component : components)
    {
        if (component->getFamily() == "Cube")
        {
            ((VoxelComponent*)component)->loadObject();
        }
        if (component->getFamily() == "Physics")
        {
            ((PhysicsComponent*)component)->setPhysicsMode(PhysicsMode::Physics_Off, false, true);
        }
        if (component->getFamily() == "Particle")
        {
            ((ParticleComponent*)component)->activate();
        }
        if (component->getFamily() == "Light3D")
        {
            ((Light3DComponent*)component)->activate();
        }
    }

    m_rightHandItem = entityToWield;
    const int itemID = m_rightHandItem->GetAttributeDataPtr<int>("ID");
    m_rightHandItem->GetAttributeDataPtr<int>("ownerID") = _ownerID;
    owner->GetAttributeDataPtr<int>("rightHandItemID") = itemID;

    Log::Debug("Player %i wielded object %i", _ownerID, itemID);
}

void HumanoidComponent::ThrowStart()
{
    //if (!m_rightHandItem)
    //{
    //    return;
    //}
    Entity* m_owner = _entityManager.getEntity(_ownerID);
    rHandTimer = m_owner->GetAttributeDataPtr<float>("lifeTime");
    rightArmAnimState = ArmState::Arm_Throwing;
}

void HumanoidComponent::ThrowItem( const glm::vec3 targetPos )
{
    if (rHandTimer == 0.0)
    {
        return;
    }

    const double throwTime = rHandTimer;
    rHandTimer = 0.0;
    rightArmAnimState = ArmState::Arm_Idle;

    if (!m_rightHandItem)
    {
        return;
    }
    Entity* m_owner = _entityManager.getEntity(_ownerID);
    const double timeNow = m_owner->GetAttributeDataPtr<float>("lifeTime");
    const float strength = fminf(timeNow- throwTime, 1.0f)*10.0f;
    const int rightHandID = m_owner->GetAttributeDataPtr<int>("rightArmID");
    const std::string rightHandObject = m_owner->GetAttributeDataPtr<std::string>("rightArmObject");
    const glm::vec3 pos = _voxels.getMesh(rightHandObject)->getPosition(rightHandID);
    
    glm::vec3 dir = targetPos-pos;
    glm::vec3 vel = glm::normalize(dir) * strength;
    btVector3 newVel = btVector3(vel.x,vel.y,vel.z);
    btVector3 newPos = btVector3(pos.x,pos.y,pos.z)+newVel;

    m_rightHandItem->GetAttributeDataPtr<glm::vec3>("position") = glm::vec3(newPos.x(), newPos.y(), newPos.z());
    const int rhiID = m_rightHandItem->GetAttributeDataPtr<int>("ID");
    PhysicsComponent* itemPhysComp = (PhysicsComponent*)_entityManager.getComponent(rhiID, "Physics");
    if (itemPhysComp) 
    {
        itemPhysComp->setPhysicsMode(PhysicsMode::Physics_Cube_AABBs, false, false);
        itemPhysComp->setLinearVelocity(&newVel);
    }
    // If weapon type is axe or knife, add rotation
    if (m_rightHandItem->GetAttributeDataPtr<int>("itemType") == Item_Weapon_Axe)
    {
        btVector3 angVel = btVector3(strength, 0.0f, 0.0f) * strength;
        if (itemPhysComp) itemPhysComp->setAngularVelocity(&angVel);
    }
    else if (m_rightHandItem->GetAttributeDataPtr<int>("itemType") == Item_Grenade)
    {
        ExplosiveComponent* explosive = (ExplosiveComponent*)_entityManager.getComponent(rhiID, "Explosive");
        if (explosive) explosive->activate();
    }
    m_rightHandItem->GetAttributeDataPtr<glm::vec3>("velocity") = glm::vec3(0, 0, 0);
    m_rightHandItem->GetAttributeDataPtr<bool>("generateCollisions") = true;
    m_rightHandItem->GetAttributeDataPtr<int>("ownerID") = 0;
    m_rightHandItem = nullptr;
    m_owner->GetAttributeDataPtr<int>("rightHandItemID") = 0;

    Log::Debug("Player %i threw object %i", _ownerID, rhiID);

    WieldNextItemFromBackpack();
}

void HumanoidComponent::UseRightHand()
{
    Entity* m_owner = _entityManager.getEntity(_ownerID);
    rHandTimer = m_owner->GetAttributeDataPtr<float>("lifeTime");

    if (!m_rightHandItem)
    {
        return;
    }
    if (m_rightHandItem->GetAttributeDataPtr<int>("itemType") == Item_Potion_Health)
    {
        rightArmAnimState = ArmState::Arm_Holding;
    }
    else
    {
        rightArmAnimState = ArmState::Arm_Swinging;
        m_rightHandItem->GetAttributeDataPtr<glm::vec3>("velocity") = glm::vec3(0, 10.0f, 0);
        m_rightHandItem->GetAttributeDataPtr<bool>("generateCollisions") = true;
    }
}

void HumanoidComponent::WieldNextItemFromBackpack()
{
    if (!m_backpack)
    {
        return;
    }

    const int backPackID = m_backpack->GetAttributeDataPtr<int>("ID");
    if (InventoryComponent* inventoryC = (InventoryComponent*)_entityManager.getComponent(backPackID, "Inventory"))
    {
        const std::vector<Entity*> items = inventoryC->getInventory();
        if (items.empty())
        {
            return;
        }

        if (m_rightHandItem)
        {
            Drop(m_rightHandItem);
            m_rightHandItem = nullptr;
        }

        Entity* nextItem = items.back();
        inventoryC->removeItem(nextItem);
        Wield(nextItem);
    }
}

void HumanoidComponent::Die()
{
    removeAllVoxelMeshInstances(true);
    
    const std::vector<Entity*> externalItems = {
        m_backpack, m_rightHandItem, m_leftHandItem
    };

    for (Entity* item : externalItems)
    {
        if (!item)
        {
            continue;
        }
        const EntityID itemID = (EntityID)item->GetAttributeDataPtr<int>("ID");
        if (PhysicsComponent* pComp = (PhysicsComponent*)_entityManager.getComponent(itemID, "Physics"))
        {
            pComp->setPhysicsMode(PhysicsMode::Physics_Cube_AABBs, false, false);
        }
        item->GetAttributeDataPtr<int>("ownerID") = 0;
    }

    //cleanup in the reverse order of creation/initialization
    if (character)
    {
        physicsWorld->removeCollisionObject(ghostObject);
        physicsWorld->removeAction(character);
    }
}

void HumanoidComponent::removeAllVoxelMeshInstances(bool spawnAsDebris)
{
    static std::vector<std::string> PART_NAMES = {
        "torso", "head", "leftFoot", "rightFoot", "leftArm", "rightArm"
    };

    Entity* owner = _entityManager.getEntity(_ownerID);
    for (const auto& partName : PART_NAMES)
    {
        const int partID = owner->GetAttributeDataPtr<int>(partName + "ID");
        if (partID > 0)
        {
            const std::string object = owner->GetAttributeDataPtr<std::string>(partName + "Object");
            if (spawnAsDebris)
            {
                const glm::vec3 pos = _voxels.getMesh(object)->getPosition(partID);
                const glm::quat rot = _voxels.getMesh(object)->getRotation(partID);
                const std::string newName = "Debris_" + intToString(Entity::GetNextEntityID());
                const EntityID newEntID = _entityManager.addEntity(newName);
                Entity* newEnt = _entityManager.getEntity(newEntID);
                newEnt->GetAttributeDataPtr<int>("ownerID") = 0;
                newEnt->GetAttributeDataPtr<int>("type") = ENTITY_DEBRIS;
                newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
                newEnt->GetAttributeDataPtr<glm::quat>("rotation") = rot;
                newEnt->GetAttributeDataPtr<std::string>("objectFile") = object;
                PhysicsComponent* physComponent = CUSTOM_NEW(PhysicsComponent, _entityManager.getAllocator())(newEntID, _entityManager, _physics, _voxels);
                _entityManager.setComponent(newEntID, physComponent);
                physComponent->setPhysicsMode(PhysicsMode::Physics_Cube_AABBs, false, false);
                VoxelComponent* cubeComponent = CUSTOM_NEW(VoxelComponent, _entityManager.getAllocator())(newEntID, object, _entityManager, _voxels);
                _entityManager.setComponent(newEntID, cubeComponent);
            }
            // TODO: SEE IF WE CAN RECYCLE RENDER INSTANCE OR IF ITS NOT WORTH IT
            _voxels.getMesh(object)->removeInstance(partID);
        }
    }
}
