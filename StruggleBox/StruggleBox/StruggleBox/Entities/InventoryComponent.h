#ifndef INVENTORY_COMPONENT_H
#define INVENTORY_COMPONENT_H

#include "EntityComponent.h"
class EntityManager;

class InventoryComponent : public EntityComponent
{
public:
    InventoryComponent(
		const int owner,
		EntityManager& manager);
    virtual ~InventoryComponent();

    virtual void update(const double delta);

    bool addItem(Entity* newItem);

    bool removeItem(Entity* oldItem);

    void setMaxItems(const int newMaxItems);

    std::vector<Entity*> getInventory() { return inventory; };

private:
	EntityManager& _manager;
	std::vector<Entity*> inventory;
	int maxItems;
	bool updated;
};


#endif /* INVENTORY_COMPONENT_H */
