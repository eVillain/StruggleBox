#include "ActorComponent.h"
#include "EntityManager.h"
#include "Timer.h"
#include "Random.h"
//#include "HumanoidComponent.h"
//#include "World3D.h"    // paused?

const float brainUpdateTime = 3.0f;
const float movementUpdateTime = 0.4f;
const float brainViewDist = 10.0f;

ActorComponent::ActorComponent(
	const int ownerID,
	std::shared_ptr<EntityManager> manager) :
EntityComponent(ownerID, "Actor"),
_manager(manager)
{
    lastUpdateTime=0.0;
    lastMoveTime=0.0;
    targetPosition = _manager->getEntity(ownerID)->GetAttributeDataPtr<glm::vec3>("position");
    targetState = Target_Move;
    targetEntity = NULL;
}
ActorComponent::~ActorComponent() {
    targetEntity = NULL;
}

void ActorComponent::update(const double delta)
{
    //if ( m_manager->world->paused ) return;
    lastUpdateTime += delta;
    lastMoveTime += delta;
    Entity* m_owner = _manager->getEntity(_ownerID);
    const glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
    if ( lastUpdateTime > brainUpdateTime ) {       // Time to update the brain
        // Try to gather data about us and the environment in 0.0 to 1.0 ranges
        const int alignment = m_owner->GetAttributeDataPtr<int>("alignment");
        const int maxHealth = m_owner->GetAttributeDataPtr<int>("maxHealth");
        const int health = m_owner->GetAttributeDataPtr<int>("health");
        float healthFactor = float(health)/maxHealth;   // If high we are good to attack
        bool armed = (m_owner->GetAttributeDataPtr<int>("rightHandItemID") > -1);
        float attackInterest = (armed?0.5f:-0.5f);      // How much we want to attack/run
        if ( alignment == ALIGNMENT_CHAOTIC ) attackInterest += 0.5f;
        // TODO:: Get value for weapon damage etc to prioritize grabbing new weapons
        
        // Gather nearby entities
        std::map<int, Entity*> nearbyEnts = _manager->getNearbyEntities(position,
                                                                         _ownerID,
                                                                         ENTITY_NONE,
                                                                         brainViewDist);
        std::map<int, Entity*>::iterator it;
        Entity* interestingEntity = NULL;   // Try to find biggest absolute interest,
        int interestType = ENTITY_NONE;
        float topInterest = 0.0f;           // Negative values mean we want to move away
        for (it=nearbyEnts.begin(); it != nearbyEnts.end(); it++) {
            Entity* ent = it->second;
            const int type = ent->GetAttributeDataPtr<int>("type");
            float interest = 0.0f;
            if ( type == ENTITY_ITEM ) {
                if ( ent->GetAttributeDataPtr<int>("ownerID") == -1 ) {
                    if ( ent->GetAttributeDataPtr<int>("damage") > 0 ) {
                        interest = (armed ? 0.0f : 1.0f);
                    } else if ( ent->GetAttributeDataPtr<int>("healing") > 0 ) {
                        interest = (1.0-healthFactor);
                    }
                }
            } else if ( type == ENTITY_HUMANOID || type == ENTITY_SKELETON ) {
                const int entAlignment = ent->GetAttributeDataPtr<int>("alignment");
                int threatLevel = alignment*entAlignment;
                if ( threatLevel == -1 ) {    // Enemy alignment, attack or flee
                    interest = (healthFactor)*(attackInterest);
                }
            }
            if ( fabsf(interest) > fabsf(topInterest) ) { // Found something more interesting than we had before
                interestingEntity = ent;
                topInterest = interest;
                interestType = type;
            }
        }
        if ( interestingEntity != NULL ) {  // We want to go towards or away from something
            targetEntity = interestingEntity;
            targetPosition = interestingEntity->GetAttributeDataPtr<glm::vec3>("position");
            if ( interestType == ENTITY_ITEM ) {
                targetState = Target_Pickup;
            } else if ( interestType == ENTITY_HUMANOID || interestType == ENTITY_SKELETON ) {
                if ( topInterest < 0.0f ) targetState = Target_Flee;
                else targetState = Target_Attack;
            }
//            printf("Got most interesting entity type %i, with %f points, state: %i!\n", interestType, topInterest, targetState);
        } else {    // Can't see anything interesting, walk somewhere randomly
            Random::RandomSeed(int(Timer::Microseconds()));
            int randX = Random::RandomInt(-2, +2);
            int randZ = Random::RandomInt(-2, +2);
            targetPosition = position + glm::vec3(randX,1.0f,randZ);
            targetState = Target_Move;
            targetEntity = NULL;
        }
        lastUpdateTime -= brainUpdateTime;
    } else if ( lastMoveTime > movementUpdateTime ) {
        // Update movement
        glm::vec3 targetDir = targetPosition-position;
        float targetHeight = targetDir.y;
        if ( targetState == Target_Flee ) {
            targetDir *= -1.0f;
        }
        float distance =  glm::length(targetDir);
        if ( distance > 0.0f ) targetDir = glm::normalize(targetDir);
        bool& jumping = m_owner->GetAttributeDataPtr<bool>("jumping");
        bool& running = m_owner->GetAttributeDataPtr<bool>("running");
        jumping = false;
        running = false;
        if ( distance > 2.0f ) { // Move towards or away from target
            if ( targetHeight > 1.0f ) { jumping = true; }
            if ( distance > 8.0f ) { running = true; }
            else if ( targetState == Target_Flee ) { running = true; }
        } else { // Close enough to perform an action on target
            if ( targetState == Target_Pickup && targetEntity ) {
                if ( targetEntity->GetAttributeDataPtr<int>("ownerID") != -1 ) {
                    targetEntity = NULL;
                    targetState = Target_Move;
                } else {
                    //HumanoidComponent* human = (HumanoidComponent*)_manager->getComponent(_ownerID, "Humanoid");
                    //human->Grab(targetEntity);
                    //targetEntity = NULL;
                    //targetState = Target_Move;
                }
            } else if ( targetState == Target_Attack && targetEntity ) {
                //HumanoidComponent* human = (HumanoidComponent*)_manager->getComponent(_ownerID, "Humanoid");
                //if ( human->rHandTimer == 0.0f ) {
                //    human->UseRightHand();
                //}
                //else {
//                    glm::vec3 rot = human->GetRotation();
                    
//                    double timeNow = SysCore::GetSeconds();
//                    if ( (timeNow - human->throwTimer) > 0.7f ) {
//                        human->ThrowItem(targetPosition);
//                    }
                //}
            }
            targetDir *= 0.1f;
        }
        m_owner->GetAttributeDataPtr<glm::vec3>("direction") = targetDir;
        lastMoveTime -= movementUpdateTime;
    }
}

