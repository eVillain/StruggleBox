#ifndef PARTICLE_COMPONENT_H
#define PARTICLE_COMPONENT_H

#include "EntityComponent.h"
#include "GFXIncludes.h"

class Locator;
class ParticleSys;

class ParticleComponent : public EntityComponent
{
public:
    // Constructor needs owning entity and filename for particle system
    ParticleComponent(const int ownerID,
                      const std::string& fileName,
                      Locator& locator);
    ~ParticleComponent();
    
    void Update( double delta );
    void Activate();
    void DeActivate();
    // Relative offset from main object
    glm::vec3 offset;
private:
    Locator& _locator;
    ParticleSys* _particleSys;
};


#endif /* defined(NGN_PARTICLE_COMPONENT_H) */
