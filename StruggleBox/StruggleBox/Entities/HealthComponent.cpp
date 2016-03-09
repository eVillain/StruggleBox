//
//  HealthComponent.cpp
//  Bloxelizer
//
//  Created by The Drudgerist on 10/4/13.
//
//

#include "HealthComponent.h"
#include "EntityManager.h"
#include "World3D.h"

HealthComponent::HealthComponent( const int ownerID, EntityManager* manager ) :
EntityComponent(ownerID) {
    m_family = "Health";
    m_manager = manager;
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    bool setDefaults = !m_owner->HasAttribute("maxHealth");
    health = &m_owner->GetAttributeDataPtr<int>("health");
    maxHealth = &m_owner->GetAttributeDataPtr<int>("maxHealth");
    if ( setDefaults ) {
        *health = 100;
        *maxHealth = 100;
    }
    damageTimer = 0.0;
}

HealthComponent::~HealthComponent() {
    
}

void HealthComponent::Update(double delta) {
    if ( *health <= 0 ) {
        printf("Entity %i health reached zero, dead\n", m_ownerID);
//        m_owner->world->entityMan->KillEntity(ownerID);
    }
    if ( damageTimer != 0.0 ) {
        damageTimer -= delta;
        if ( damageTimer < 0.0 ) damageTimer = 0.0;
    }
}
void HealthComponent::AddHealth( int newHealth, Entity* healer ) {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    printf("Healed %s by %i points\n", m_owner->GetAttributeDataPtr<std::string>("name").c_str(), newHealth);
    health += newHealth;
    if ( health < maxHealth ) health = maxHealth;
}
void HealthComponent::TakeDamage( int damage, Entity* damager ) {
    if ( damageTimer == 0.0 ) {
        Entity* m_owner = m_manager->GetEntity(m_ownerID);
        printf("%s took %i damage from ", m_owner->GetAttributeDataPtr<std::string>("name").c_str(), damage);
        if ( damager ) {
            const int ownerID = damager->GetAttributeDataPtr<int>("ownerID");
            if ( ownerID != -1 ) {
                Entity* damageOwner = m_manager->world->entityMan->GetEntity(ownerID);
                if ( damageOwner ) {
                    printf("%s's ", damageOwner->GetAttributeDataPtr<std::string>("name").c_str());
                }
            }
            printf("%s\n", damager->GetAttributeDataPtr<std::string>("name").c_str());
        } else {
            printf("???\n");
        }
        *health -= damage;
        if ( *health <= 0 ) {
            // Dead
            printf("%s died!\n", m_owner->GetAttributeDataPtr<std::string>("name").c_str());
        }
        damageTimer = 0.25f;
    }
}

