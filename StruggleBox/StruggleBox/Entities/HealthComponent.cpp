#include "HealthComponent.h"
#include "EntityManager.h"
#include "World3D.h"
#include "Log.h"

HealthComponent::HealthComponent(
	const int ownerID,
	std::shared_ptr<EntityManager> manager) :
EntityComponent(ownerID, "Health"),
_manager(manager)
{
    Entity* _owner = _manager->getEntity(_ownerID);
    bool setDefaults = !_owner->HasAttribute("maxHealth");
    health = &_owner->GetAttributeDataPtr<int>("health");
    maxHealth = &_owner->GetAttributeDataPtr<int>("maxHealth");
    if ( setDefaults ) {
        *health = 100;
        *maxHealth = 100;
    }
    damageTimer = 0.0;
}

HealthComponent::~HealthComponent()
{ }

void HealthComponent::update(const double delta)
{
    if ( *health <= 0 )
	{
		Entity* _owner = _manager->getEntity(_ownerID);
        Log::Debug("[HealthComponent] Entity %i (%s) health reached zero, died",
			_ownerID,
			_owner->GetAttributeDataPtr<std::string>("name"));
		_manager->killEntity(_ownerID);
    }
    if ( damageTimer != 0.0 )
	{
        damageTimer -= delta;
        if ( damageTimer < 0.0 ) damageTimer = 0.0;
    }
}

void HealthComponent::addHealth(
	int newHealth,
	Entity* healer )
{
    Entity* _owner = _manager->getEntity(_ownerID);
    Log::Debug("[HealthComponent] Entity %i (%s) healed by %i points\n",
		_ownerID,
		_owner->GetAttributeDataPtr<std::string>("name").c_str(),
		newHealth);
    health += newHealth;
    if ( health < maxHealth ) health = maxHealth;
}

void HealthComponent::takeDamage(
	int damage,
	Entity* damager )
{
    if ( damageTimer == 0.0 )
	{
        Entity* _owner = _manager->getEntity(_ownerID);
		Log::Debug("[HealthComponent] Entity %i (%s) took %i damage",
			_ownerID,
			_owner->GetAttributeDataPtr<std::string>("name").c_str(),
			damage);
        if (damager)
		{
			const int damagerID = damager->GetID();
			const int damageOwnerID = damager->GetAttributeDataPtr<int>("ownerID");			
			Log::Debug("[HealthComponent] Damage inflicted by %i (%s)",
				damagerID,
				damager->GetAttributeDataPtr<std::string>("name").c_str());
            if ( damageOwnerID != -1 ) {
                Entity* damageOwner = _manager->getEntity(damageOwnerID);
                if (damageOwner)
				{
					Log::Debug("[HealthComponent] Entity to blame is %i (%s)",
						damageOwnerID,
						damageOwner->GetAttributeDataPtr<std::string>("name").c_str());
                }
            }
        } else {
			Log::Debug("[HealthComponent] Damage inflicted by unknown effect!");
		}

		// Add damage
        *health -= damage;
        if ( *health <= 0 )
		{
			Log::Debug("[HealthComponent] The damage resulted in death for %i (%s)",
				_ownerID,
				_owner->GetAttributeDataPtr<std::string>("name").c_str());
		}
		else
		{
			damageTimer = 0.25f;
		}
	}
	else // damageTimer != 0.0 (invulnerable to damage because just took a hit)
	{
	
	}
}

