//
//  InventoryComponent.cpp
//  Ingenium
//
//  Created by The Drudgerist on 29/12/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#include "InventoryComponent.h"
//#include "World3D.h"

InventoryComponent::InventoryComponent( const int ownerID, EntityManager* manager ) :
EntityComponent(ownerID) {
    m_manager = manager;
    m_family = "Inventory";
    maxItems = 32;
    updated = false;
}

InventoryComponent::~InventoryComponent() {
    // Throw all items out into world?
    
}

void InventoryComponent::Update( double delta ) {
    
}
bool InventoryComponent::AddItem( Entity* newItem ) {
    if ( inventory.size() < maxItems ) {
        if ( std::find(inventory.begin(), inventory.end(), newItem) == inventory.end() ) {
            inventory.push_back(newItem);
            updated = true;
            return true;
        }
    }
    return false;
}
bool InventoryComponent::RemoveItem( Entity* oldItem ) {
    if ( inventory.size() > 0 ) {
        std::vector<Entity*>::iterator it = std::find(inventory.begin(), inventory.end(), oldItem);
        if ( it != inventory.end() ) {
            inventory.erase(it);
            updated = true;
            return true;
        }
    }
    return false;
}
void InventoryComponent::SetMaxItems( const int newMaxItems ) {
    if ( inventory.size() > newMaxItems ) {
        printf("[Inventory] couldn't resize, more items than new max size\n");
    } else {
        maxItems = newMaxItems;
    }
}

