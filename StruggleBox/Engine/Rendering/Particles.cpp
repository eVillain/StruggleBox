#include "Particles.h"

#include "ParticleSys.h"
#include "ParticleSystemLoader.h"
#include "ParticleRenderer.h"

#include "ArenaOperators.h"
#include "Allocator.h"
#include "Renderer.h"
#include "VertBuffer.h"
#include "Log.h"

#include "Random.h"
#include "Timer.h"

const size_t PARTICLE_POOL_SIZE = 16 * 1024 * 1024;

Particles::Particles(Allocator& allocator, Renderer& renderer)
	: m_allocator(allocator)
	, m_renderer(renderer)
	, m_nextParticleSysID(0)
{
	Log::Info("[Particles] constructor, instance at %p", this);
	Random::RandomSeed((int)Timer::Microseconds());
}

Particles::~Particles()
{
	Log::Info("[Particles] destructor, instance at %p", this);
}

void Particles::update(double deltaTime)
{
	for (const auto& pair : m_systems)
	{
		const ParticleSystemID systemID = pair.first;
		ParticleSystem* system = pair.second;
		system->Update(deltaTime);
		if (!system->getIsDirty())
		{
			continue;
		}
		system->setIsDirty(false);
		auto it = m_renderers.find(systemID);
		if (it != m_renderers.end())
		{
			ParticleRenderer* renderer = it->second;
			renderer->Update(system->getPosition(), system->getParticles(), system->getParticleCount());
		}
	}
}

ParticleSystemID Particles::create(const std::string filePath, const std::string fileName)
{
	ParticleSystemConfig config = ParticleSystemLoader::load(filePath + fileName);
	if (config.maxParticles == 0)
	{
		return 0;
	}
	Particle* particleData = CUSTOM_NEW_ARRAY(Particle, config.maxParticles, m_allocator);
	ParticleSystem* system = CUSTOM_NEW(ParticleSystem, m_allocator)(config, particleData);
	ParticleSystemID systemID = m_nextParticleSysID++;
	m_systems[systemID] = system;

	ParticleRenderer* renderer = CUSTOM_NEW(ParticleRenderer, m_allocator)(config.maxParticles, m_renderer, m_allocator);
	renderer->setTextureID(m_renderer.getTextureID(config.texFileName, true));
	m_renderers[systemID] = renderer;

	return systemID;
}

ParticleSystem* Particles::getSystemByID(const ParticleSystemID systemID)
{
	const auto it = m_systems.find(systemID);
	if (it == m_systems.end())
	{
		return nullptr;
	}
	return it->second;
}

void Particles::destroy(const ParticleSystemID systemID)
{
	const auto it = m_systems.find(systemID);
	if (it != m_systems.end())
	{
		ParticleSystem* system = it->second;
		m_systems.erase(it);
		CUSTOM_DELETE_ARRAY(system->getParticles(), m_allocator);
		CUSTOM_DELETE(system, m_allocator);
	}

	const auto it2 = m_renderers.find(systemID);
	if (it2 != m_renderers.end())
	{
		ParticleRenderer* renderer = it2->second;
		m_renderers.erase(it2);
		CUSTOM_DELETE(renderer, m_allocator);
	}
}

void Particles::draw()
{
	for (const auto& pair : m_systems)
	{
		const ParticleSystemID systemID = pair.first;
		auto it = m_renderers.find(systemID);
		if (it != m_renderers.end())
		{
			ParticleSystem* system = pair.second;
			ParticleRenderer* renderer = it->second;
			renderer->Draw(system->getBlendMode(), system->getDepthMode(), system->getLighting());
		}
	}
}
