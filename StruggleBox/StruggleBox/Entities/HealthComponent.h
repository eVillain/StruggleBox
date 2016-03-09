//
//  HealthComponent.h
//  Bloxelizer
//
//  Created by The Drudgerist on 10/4/13.
//
//

#ifndef BWO_HEALTHCOMPONENT_H
#define BWO_HEALTHCOMPONENT_H

#include "EntityComponent.h"
class EntityManager;

class HealthComponent : public EntityComponent {
    EntityManager* m_manager;
    int* health;
    int* maxHealth;
    double damageTimer;
public:
    HealthComponent( const int ownerID, EntityManager* manager );
    ~HealthComponent();
    
    void Update( double delta );
    
    void AddHealth( int newHealth, Entity* healer = NULL);
    void TakeDamage( int damage, Entity* damager = NULL);
};


#endif /* defined(BWO_HEALTHCOMPONENT_H) */
