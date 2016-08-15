#include "ParticleComponent.h"
#include "EntityManager.h"
#include "FileUtil.h"
#include "Particles.h"
#include "ParticleSys.h"
#include "World3D.h"
#include "Entity.h"

ParticleComponent::ParticleComponent(
	const int ownerID,
	const std::string& fileName,
	std::shared_ptr<EntityManager> entityManager,
	std::shared_ptr<Particles> particles) :
	EntityComponent(ownerID, "Particle"),
	_entityManager(entityManager),
	_particles(particles)
{
	if (fileName.length() != 0)
	{ // Load particle system from filename given
		_particleSys = _particles->create(FileUtil::GetPath().append("Data/Particles/"), fileName);
		offset = glm::vec3();
	}
	else
	{    // No filename given, load from attributes
		Entity* m_owner = _entityManager->getEntity(_ownerID);
		std::string particleFile = m_owner->GetAttributeDataPtr<std::string>("particleFile");
		_particleSys = _particles->create(FileUtil::GetPath().append("Data/Particles/"), particleFile);
		offset = m_owner->GetAttributeDataPtr<glm::vec3>("particleOffset");
	}
}

ParticleComponent::~ParticleComponent()
{
	if (_particleSys)
	{
		_particles->destroy(_particleSys);
		_particleSys = NULL;
	}
}

void ParticleComponent::update(const double delta)
{
	if (_particleSys) {
		Entity* m_owner = _entityManager->getEntity(_ownerID);
		glm::vec3 ownerPos = m_owner->GetAttributeDataPtr<glm::vec3>("position");
		glm::quat ownerRot = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
		_particleSys->sourcePos = ownerPos + (ownerRot*offset);
	}
}

void ParticleComponent::activate()
{
	if (_particleSys) { _particleSys->active = true; }
}

void ParticleComponent::deActivate()
{
	if (_particleSys) { _particleSys->active = false; }
}

