#include "InventoryComponent.h"

InventoryComponent::InventoryComponent(
	const int ownerID,
	EntityManager& manager) :
EntityComponent(ownerID, "Inventory"),
_manager(manager)
{
    maxItems = 32;
    updated = false;
}

InventoryComponent::~InventoryComponent()
{
    // Throw all items out into world?
    
}

void InventoryComponent::update(const double delta)
{ }

bool InventoryComponent::addItem(Entity* newItem)
{
    if ( inventory.size() < maxItems )
    {
        if ( std::find(inventory.begin(), inventory.end(), newItem) == inventory.end() ) {
            inventory.push_back(newItem);
            updated = true;
            return true;
        }
    }
    return false;
}

bool InventoryComponent::removeItem(Entity* oldItem)
{
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

void InventoryComponent::setMaxItems(const int newMaxItems)
{
    if ( inventory.size() > newMaxItems ) {
        printf("[Inventory] couldn't resize, contained more items than new max size\n");
    } else {
        maxItems = newMaxItems;
    }
}

