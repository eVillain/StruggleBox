#ifndef ITEM_COMPONENT_H
#define ITEM_COMPONENT_H

#include "EntityComponent.h"
#include "GFXIncludes.h"

typedef enum {
    Item_None = 0,
    Item_Block,         // One voxel
    Item_Weapon_Sword,
    Item_Weapon_Knife,
    Item_Weapon_Axe,
    Item_Weapon_Lance,
    Item_Weapon_Bow,
    Item_Arrow_Wood,
    Item_Pickaxe,
    Item_Potion_Health,
    Item_Backpack,
    Item_Coin_1,
    Item_Coin_10,
    Item_Coin_100,
    Item_Torch,
    Item_Grenade,
    Item_Suckernade,
} ItemType;

inline static std::string NameForItem(const ItemType type)
{
    std::string name;
    switch (type) {
        case Item_None:
            name = "UnknownItem";
            break;
        case Item_Block:
            name = "Block";
            break;
        case Item_Weapon_Sword:
            name = "Sword";
            break;
        case Item_Weapon_Knife:
            name = "Knife";
            break;
        case Item_Weapon_Axe:
            name = "Axe";
            break;
        case Item_Weapon_Lance:
            name = "Lance";
            break;
        case Item_Weapon_Bow:
            name = "Bow";
            break;
        case Item_Arrow_Wood:
            name = "Arrow";
            break;
        case Item_Pickaxe:
            name = "Pickaxe";
            break;
        case Item_Potion_Health:
            name = "Health Potion";
            break;
        case Item_Backpack:
            name = "Backpack";
            break;
        case Item_Coin_1:
            name = "1G";
            break;
        case Item_Coin_10:
            name = "10G";
            break;
        case Item_Coin_100:
            name = "100G";
            break;
        case Item_Torch:
            name = "Torch";
            break;
        case Item_Grenade:
            name = "Grenade";
            break;
        case Item_Suckernade:
            name = "Suckernade";
            break;
        default:
            break;
    }
    return name;
}

class EntityManager;
class Particles;
class Text;

class ItemComponent : public EntityComponent
{
public:
    ItemComponent(
		const int ownerID,
		std::shared_ptr<EntityManager> entityManager,
		std::shared_ptr<Particles> particles,
		std::shared_ptr<Text> text);

    void update(const double delta);

    void hitEntity(Entity* entityB,
                   glm::vec3 velocity,
                   glm::vec3 position);

private:
	std::shared_ptr<EntityManager> _entityManager;
	std::shared_ptr<Particles> _particles;
	std::shared_ptr<Text> _text;
};

#endif
