#ifndef PARTICLE_COMPONENT_H
#define PARTICLE_COMPONENT_H

#include "EntityComponent.h"
#include "GFXIncludes.h"

class Particles;
class EntityManager;

class ParticleSys;

class ParticleComponent : public EntityComponent
{
public:
	// Constructor needs owning entity and filename for particle system
	ParticleComponent(
		const int ownerID,
		const std::string& fileName,
		std::shared_ptr<EntityManager> entityManager,
		std::shared_ptr<Particles> particles);
	~ParticleComponent();

	void update(const double delta);
	void activate();
	void deActivate();
	// Relative offset from main object
	glm::vec3 offset;
private:
	std::shared_ptr<EntityManager> _entityManager;
	std::shared_ptr<Particles> _particles;

	std::shared_ptr<ParticleSys> _particleSys;
};


#endif /* PARTICLE_COMPONENT_H */
