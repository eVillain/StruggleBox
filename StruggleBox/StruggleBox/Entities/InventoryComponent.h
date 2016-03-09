//
//  InventoryComponent.h
//  Ingenium
//
//  Created by The Drudgerist on 29/12/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//
//  This component holds entities within it

#ifndef NGN_INVENTORY_COMPONENT_H
#define NGN_INVENTORY_COMPONENT_H

#include "EntityComponent.h"
class EntityManager;

class InventoryComponent : public EntityComponent {
    EntityManager* m_manager;
    std::vector<Entity*> inventory;
    int maxItems;
public:
    InventoryComponent( const int owner, EntityManager* manager );
    virtual ~InventoryComponent();
    virtual void Update( double delta );
    bool AddItem( Entity* newItem );
    bool RemoveItem( Entity* oldItem );
    void SetMaxItems( const int newMaxItems );

    std::vector<Entity*> GetInventory( void ) { return inventory; };
    bool updated;
};


#endif /* defined(NGN_INVENTORY_COMPONENT_H) */
