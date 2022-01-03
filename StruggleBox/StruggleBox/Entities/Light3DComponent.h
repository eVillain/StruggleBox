#ifndef LIGHT3D_COMPONENT_H
#define LIGHT3D_COMPONENT_H

#include "EntityComponent.h"
#include "Light3D.h"

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

#endif /* LIGHT3D_COMPONENT_H */
