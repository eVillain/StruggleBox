//
//  ActorComponent.h
//  Bloxelizer
//
//  Created by The Drudgerist on 8/29/13.
//
//

#ifndef BWO_ACTOR_COMPONENT_H
#define BWO_ACTOR_COMPONENT_H
#include "EntityComponent.h"
#include "GFXIncludes.h"

class EntityManager;

typedef enum {
    Target_Move = 0,
    Target_Flee = 1,
    Target_Attack = 2,
    Target_Pickup = 3,
} TargetState;

class ActorComponent : public EntityComponent {
    EntityManager* m_manager;
    
    double lastUpdateTime;
    double lastMoveTime;

    int targetState;
    glm::vec3 targetPosition;
    Entity* targetEntity;
public:
    ActorComponent( const int ownerID, EntityManager* manager );
    ~ActorComponent();
    virtual void Update( double delta );    
};
#endif
