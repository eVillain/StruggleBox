//
//  ExplosiveComponent.h
//  Ingenium
//
//  Created by The Drudgerist on 14/04/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#ifndef NGN_EXPLOSIVE_COMPONENT_H
#define NGN_EXPLOSIVE_COMPONENT_H

#include "EntityComponent.h"
class EntityManager;
class Locator;

class ExplosiveComponent : public EntityComponent
{
public:
    ExplosiveComponent(const int ownerID,
                       Locator& locator);
    ~ExplosiveComponent();
    
    void Update( const double delta );

    void Activate();
private:
    Locator& _locator;
    EntityManager* m_manager;
    double m_timer;
    double m_duration;
};

#endif /* defined(NGN_EXPLOSIVE_COMPONENT_H) */
