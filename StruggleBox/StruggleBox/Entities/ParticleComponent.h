#ifndef PARTICLE_COMPONENT_H
#define PARTICLE_COMPONENT_H

#include "EntityComponent.h"
#include "CoreIncludes.h"
#include "Particles.h"

class EntityManager;
class ParticleSystem;

class ParticleComponent : public EntityComponent
{
public:
	// Constructor needs owning entity and filename for particle system
	ParticleComponent(
		const int ownerID,
		const std::string& fileName,
		EntityManager& entityManager,
		Particles& particles);
	~ParticleComponent();

	void update(const double delta);
	void activate();
	void deActivate();
	// Relative offset from main object
	glm::vec3 offset;

	ParticleSystem* getParticleSystem() { return _particleSys; }
private:
	EntityManager& _entityManager;
	Particles& _particles;

	ParticleSystemID m_particleSystemID;
	ParticleSystem* _particleSys;
};


#endif /* PARTICLE_COMPONENT_H */
