#include "ParticleComponent.h"
#include "EntityManager.h"
#include "FileUtil.h"
#include "ParticleSys.h"
#include "World3D.h"
#include "Entity.h"

ParticleComponent::ParticleComponent(
	const int ownerID,
	const std::string& fileName,
	EntityManager& entityManager,
	Particles& particles)
	: EntityComponent(ownerID, "Particle")
	, _entityManager(entityManager)
	, _particles(particles)
	, m_particleSystemID(0)
	, _particleSys(nullptr)
{
	if (fileName.length() != 0)
	{ // Load particle system from filename given
		m_particleSystemID = _particles.create(FileUtil::GetPath().append("Data/Particles/"), fileName);
		_particleSys = _particles.getSystemByID(m_particleSystemID);
		offset = glm::vec3();
		Entity* m_owner = _entityManager.getEntity(_ownerID);
		m_owner->GetAttributeDataPtr<std::string>("particleFile") = fileName;
	}
	else
	{    // No filename given, load from attributes
		Entity* m_owner = _entityManager.getEntity(_ownerID);
		std::string particleFile = m_owner->GetAttributeDataPtr<std::string>("particleFile");
		m_particleSystemID = _particles.create(FileUtil::GetPath().append("Data/Particles/"), particleFile);
		_particleSys = _particles.getSystemByID(m_particleSystemID);
		offset = m_owner->GetAttributeDataPtr<glm::vec3>("particleOffset");
	}
}

ParticleComponent::~ParticleComponent()
{
	if (_particleSys)
	{
		_particles.destroy(m_particleSystemID);
		_particleSys = NULL;
	}
}

void ParticleComponent::update(const double delta)
{
	if (!_particleSys)
	{
		return;
	}
	Entity* m_owner = _entityManager.getEntity(_ownerID);
	glm::vec3 ownerPos = m_owner->GetAttributeDataPtr<glm::vec3>("position");
	glm::quat ownerRot = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
	_particleSys->setPosition(ownerPos + (ownerRot * offset));
}

void ParticleComponent::activate()
{
	if (_particleSys) { _particleSys->setActive(true); }
}

void ParticleComponent::deActivate()
{
	if (_particleSys) { _particleSys->setActive(false); }
}

