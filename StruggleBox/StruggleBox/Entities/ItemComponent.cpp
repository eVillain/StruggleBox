//
//  ItemComponent.cpp
//  Bloxelizer
//
//  Created by The Drudgerist on 9/22/13.
//
//

#include "ItemComponent.h"
#include "EntityManager.h"
#include "HealthComponent.h"
#include "GFXHelpers.h"
#include "Locator.h"
#include "FileUtil.h"
#include "TextManager.h"
#include "ParticleManager.h"
#include "PhysicsComponent.h"
#include "btRigidBody.h"

ItemComponent::ItemComponent( const int ownerID,
                             EntityManager* manager,
                             Locator& locator) :
EntityComponent(ownerID),
_locator(locator)
{
    m_family = "Item";
    m_manager = manager;
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    
    m_owner->GetAttributeDataPtr<int>("type") = ENTITY_ITEM;

    if ( !m_owner->HasAttribute("ownerID") ) {
        m_owner->GetAttributeDataPtr<int>("ownerID") = -1;
    }
    if ( !m_owner->HasAttribute("itemType") ) {
        m_owner->GetAttributeDataPtr<int>("itemType") = Item_None;
    }
    if ( !m_owner->HasAttribute("damage") ) {
        m_owner->GetAttributeDataPtr<int>("damage") = 0;
    }
    if ( !m_owner->HasAttribute("healing") ) {
        m_owner->GetAttributeDataPtr<int>("healing") = 0;
    }
}

ItemComponent::~ItemComponent()
{ }

void ItemComponent::Update(double delta) {
    // Nothing here for now, timers to do stuff later
}

// Perform damage calculation or healing effect etc.
void ItemComponent::HitEntity( Entity* entityB, glm::vec3 velocity, glm::vec3 position ) {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    int ownerIDA = m_owner->GetAttributeDataPtr<int>("ownerID");
    int ownerIDB = entityB->GetAttributeDataPtr<int>("ownerID");
    int IDB = entityB->GetAttributeDataPtr<int>("ID");

    if ( ownerIDA == ownerIDB ||
        ownerIDA == IDB ) {  // Same owner, cancel?
        return;
    }
    if ( ownerIDA == -1 ) {     // Item without owner, if moving fast it's probably thrown
    } else {    // entityA has owner, collided with another entity
    }
    int aDamage = m_owner->GetAttributeDataPtr<int>("damage");  // Check if item does damage
    if ( aDamage != 0 ) {   // Calculate damage from velocity
        float velDamage = velocity.length();
        if ( velDamage > 1.0f ) {
            velDamage = velDamage*2.0f;
            HealthComponent* healthB = (HealthComponent*)m_manager->GetComponent(ownerIDB, "Health");
            if ( healthB ) {
                healthB->TakeDamage(aDamage*velDamage, m_owner);
                std::string dmgText = intToString(aDamage*velDamage);
                glm::vec3 bPos = entityB->GetAttributeDataPtr<glm::vec3>("position")+glm::vec3(0.0f,1.0f,0.0f);
                _locator.Get<TextManager>()->AddText(dmgText, bPos, false, 40, FONT_PIXEL, 2.0f, COLOR_RED);
                // TODO: ADD TEXT ANIMATION ( FLOAT UP AND FAED OUT )
            }
            int bType = entityB->GetAttributeDataPtr<int>("type");
            if ( bType == ENTITY_ITEM ) {   // Item collided with another item
                // Item on item collision, throw sparks!
                ParticleSys* pSys = _locator.Get<ParticleManager>()->AddSystem(FileUtil::GetPath().append("Data/Particles/"),
                                                                               "Sparks3D.plist");
                pSys->position = position;
            } else if ( bType == ENTITY_HUMANOID ) {
                ParticleSys* pSys = _locator.Get<ParticleManager>()->AddSystem(FileUtil::GetPath().append("Data/Particles/"),
                                                                               "Blood3D.plist");
                pSys->position = position;
                pSys->duration = velDamage/10.0f;
            } else if ( bType == ENTITY_DECOR ) {
                PhysicsComponent* pComp = (PhysicsComponent*)m_manager->GetComponent(ownerIDB, "Physics");
                if ( pComp ) {
                    pComp->GetRigidBody()->applyCentralImpulse(btVector3(velocity.x,velocity.y,velocity.z)*0.5f);
                }
            }
        }   // entity has velocity over threshold
    }   // entity does damage
    int aHealth = m_owner->GetAttributeDataPtr<int>("health");  // Check if item does health
    if ( aHealth != 0 ) {   // Add health
        HealthComponent* healthB = (HealthComponent*)m_manager->GetComponent(ownerIDB, "Health");
        if ( healthB ) {
            healthB->AddHealth(aHealth, m_owner);
            std::string healthText = intToString(aHealth);
            glm::vec3 bPos = entityB->GetAttributeDataPtr<glm::vec3>("position");
            _locator.Get<TextManager>()->AddText(healthText,
                                                 bPos + glm::vec3(0.0f,1.0f,0.0f),
                                                 false,
                                                 40,
                                                 FONT_PIXEL,
                                                 2.0f,
                                                 COLOR_GREEN);
        }
    }
}
