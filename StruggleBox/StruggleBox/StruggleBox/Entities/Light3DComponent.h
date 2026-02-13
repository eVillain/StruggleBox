#pragma once

#include "EntityComponent.h"
#include "Lighting3DDeferred.h"

class EntityManager;

class Light3DComponent : public EntityComponent
{
public:
    Light3DComponent(const int ownerID,
		EntityManager& entityManager);
    ~Light3DComponent();

    void update(const double delta);

    void activate();
    void deActivate();

    // Relative offset from main object
    glm::vec3 offset;
    
	LightInstance& getLight() { return _light; }

private:
	EntityManager& _entityManager;
	
	LightInstance _light;
};
