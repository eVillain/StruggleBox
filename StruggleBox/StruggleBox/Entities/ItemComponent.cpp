#include "ItemComponent.h"
#include "EntityManager.h"
#include "HealthComponent.h"
#include "GFXHelpers.h"
#include "FileUtil.h"
#include "Text.h"
#include "Particles.h"
#include "PhysicsComponent.h"
#include "btRigidBody.h"
#include "Log.h"

ItemComponent::ItemComponent(
	const int ownerID,
	std::shared_ptr<EntityManager> entityManager,
	std::shared_ptr<Particles> particles,
	std::shared_ptr<Text> text) :
EntityComponent(ownerID, "Item"),
_entityManager(entityManager),
_particles(particles),
_text(text)
{
    Entity* _owner = _entityManager->getEntity(_ownerID);
    
    _owner->GetAttributeDataPtr<int>("type") = ENTITY_ITEM;

    if ( !_owner->HasAttribute("ownerID") ) {
        _owner->GetAttributeDataPtr<int>("ownerID") = -1;
    }
    if ( !_owner->HasAttribute("itemType") ) {
        _owner->GetAttributeDataPtr<int>("itemType") = Item_None;
    }
    if ( !_owner->HasAttribute("damage") ) {
        _owner->GetAttributeDataPtr<int>("damage") = 0;
    }
    if ( !_owner->HasAttribute("healing") ) {
        _owner->GetAttributeDataPtr<int>("healing") = 0;
    }
}

void ItemComponent::update(const double delta)
{
    // Nothing here for now, timers to do stuff later
}

// Perform damage calculation or healing effect etc.
void ItemComponent::hitEntity(Entity* entityB,
                              glm::vec3 velocity,
                              glm::vec3 position)
{
    Entity* m_owner = _entityManager->getEntity(_ownerID);
    int ownerIDA = m_owner->GetAttributeDataPtr<int>("ownerID");
    int ownerIDB = entityB->GetAttributeDataPtr<int>("ownerID");
    int IDB = entityB->GetAttributeDataPtr<int>("ID");

    if ( ownerIDA == ownerIDB ||
        ownerIDA == IDB ) {  // Same owner, cancel?
        return;
    }
    if ( ownerIDA == -1 ) {     // Item without owner, if moving fast it's probably thrown
        Log::Debug("[ItemComponent] (entity %i, no owner) hit %i, owned by %i", _ownerID, ownerIDA, IDB, ownerIDB);
    } else {    // entityA has owner, collided with another entity
        Log::Debug("[ItemComponent] (entity %i, owner %i) hit %i, owned by %i", _ownerID, ownerIDA, IDB, ownerIDB);
    }
    
    int aDamage = m_owner->GetAttributeDataPtr<int>("damage");  // Check if item does damage
    if ( aDamage != 0 ) {   // Calculate damage from velocity
        float velDamage = velocity.length();
        if ( velDamage > 1.0f ) {
            velDamage = velDamage*2.0f;
            HealthComponent* healthB = (HealthComponent*)_entityManager->getComponent(ownerIDB, "Health");
            if ( healthB ) {
                healthB->takeDamage(aDamage*velDamage, m_owner);
                std::string dmgText = intToString(aDamage*velDamage);
                glm::vec3 bPos = entityB->GetAttributeDataPtr<glm::vec3>("position")+glm::vec3(0.0f,1.0f,0.0f);
                //_text->AddText(dmgText, bPos, false, 40, FONT_PIXEL, 2.0f, COLOR_RED);
                // TODO: ADD TEXT ANIMATION ( FLOAT UP AND FAED OUT )
            }
            int bType = entityB->GetAttributeDataPtr<int>("type");
            if ( bType == ENTITY_ITEM ) {   // Item collided with another item
                // Item on item collision, throw sparks!
                std::shared_ptr<ParticleSys> pSys = _particles->create(FileUtil::GetPath().append("Data/Particles/"),
                                                                               "Sparks3D.plist");
                pSys->position = position;
            } else if ( bType == ENTITY_HUMANOID ) {
                std::shared_ptr<ParticleSys> pSys = _particles->create(FileUtil::GetPath().append("Data/Particles/"),
                                                                               "Blood3D.plist");
                pSys->position = position;
                pSys->duration = velDamage/10.0f;
            } else if ( bType == ENTITY_DECOR ) {
                PhysicsComponent* pComp = (PhysicsComponent*)_entityManager->getComponent(ownerIDB,
                                                                                                         "Physics");
                if ( pComp ) {
                    pComp->getRigidBody()->applyCentralImpulse(btVector3(velocity.x,velocity.y,velocity.z)*0.5f);
                }
            }
        }   // entity has velocity over threshold
    }   // entity does damage
    
    int aHealth = m_owner->GetAttributeDataPtr<int>("health");  // Check if item does health
    if ( aHealth != 0 ) {   // Add health
        HealthComponent* healthB = (HealthComponent*)_entityManager->getComponent(ownerIDB,
                                                                                                 "Health");
        if ( healthB ) {
            healthB->addHealth(aHealth, m_owner);
            std::string healthText = intToString(aHealth);
            glm::vec3 bPos = entityB->GetAttributeDataPtr<glm::vec3>("position");
            //_text->AddText(healthText,
            //                                     bPos + glm::vec3(0.0f,1.0f,0.0f),
            //                                     false,
            //                                     40,
            //                                     FONT_PIXEL,
            //                                     2.0f,
            //                                     COLOR_GREEN);
        }
    }
}
