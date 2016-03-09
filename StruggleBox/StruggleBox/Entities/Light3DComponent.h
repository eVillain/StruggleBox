#ifndef LIGHT3D_COMPONENT_H
#define LIGHT3D_COMPONENT_H

#include "EntityComponent.h"
#include "GFXIncludes.h"

class Locator;
class Light3D;
class EntityManager;

class Light3DComponent : public EntityComponent
{
public:
    Light3DComponent(const int ownerID,
                     EntityManager* owner,
                     Locator& locator);
    ~Light3DComponent();
    void Update( double delta );
    void Activate();
    void DeActivate();
    // Relative offset from main object
    glm::vec3 offset;
    
private:
    Locator& _locator;
    EntityManager* m_manager;
    Light3D* m_light;
};

#endif /* LIGHT3D_COMPONENT_H */
