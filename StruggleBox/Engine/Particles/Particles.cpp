#include "Particles.h"

#include "ParticleSystem.h"
#include "ParticleSystemLoader.h"

#include "ArenaOperators.h"
#include "Allocator.h"
#include "Injector.h"

#include "Renderer2D.h"
#include "Renderer3D.h"
#include "Renderer3DDeferred.h"
#include "RenderCore.h"
#include "DefaultShaders.h"
#include "GLUtils.h"

#include "Log.h"
#include "Random.h"
#include "Timer.h"

const size_t PARTICLE_POOL_SIZE = 16 * 1024 * 1024;

Particles::Particles(Allocator& allocator, Injector& injector)
	: m_allocator(allocator)
	, m_renderer2D(nullptr)
	, m_renderer3D(nullptr)
	, m_rendererPBR(nullptr)
	, m_renderCore(nullptr)
	, m_impostorShaderID(0)
	, m_nextParticleSysID(0)
{
	Log::Info("[Particles] constructor, instance at %p", this);
	Random::RandomSeed((int)Timer::Microseconds());
	if (injector.hasMapping<Renderer2D>())
	{
		m_renderer2D = &injector.getInstance<Renderer2D>();
	}
	if (injector.hasMapping<Renderer3D>())
	{
		m_renderer3D = &injector.getInstance<Renderer3D>();
	}
	if (injector.hasMapping<Renderer3DDeferred>())
	{
		m_rendererPBR = &injector.getInstance<Renderer3DDeferred>();
	}
	if (injector.hasMapping<RenderCore>())
	{
		m_renderCore = &injector.getInstance<RenderCore>();
	}
	m_impostorShaderID = m_renderCore->getShaderIDFromSource(impostorGeometryShaderSource, impostorVertexShaderSource, impostorFragmentShaderSource, "ImpostorBillboardShader");
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
}

void Particles::draw()
{
	for (const auto& pair : m_systems)
	{
		const ParticleSystemID systemID = pair.first;
		ParticleSystem* system = pair.second;
		const size_t count = system->getParticleCount();
		if (count == 0)
		{
			return;
		}
		const ParticleSystemConfig config = system->getConfig();
		const TextureID textureID = m_renderCore->getTextureID(config.texFileName, true);
		ImpostorVertexData* impostorVerts = nullptr;
		if (config.dimensions == ParticleSysDimensions::ParticleSys2D)
		{
			const BlendMode blendMode = system->getBlendMode();
			const DepthMode depthMode = system->getDepthMode();
			impostorVerts = m_renderer2D->bufferImpostorPoints(count, textureID, blendMode, depthMode);
		}
		else
		{
			if (config.lighting == ParticleSysLighting::ParticleSysLightOff && m_renderer3D)
			{
				impostorVerts = m_renderer3D->bufferImpostorPoints(count, textureID);
			}
			else
			{
				impostorVerts = m_rendererPBR->bufferImpostorPoints(count, m_impostorShaderID, textureID);
			}
		}

		const Particle* particles = system->getParticles();
		for (size_t particleIdx = 0; particleIdx < count; particleIdx++)
		{
			const Particle& p = particles[particleIdx];

			const float particleSize = std::max(p.size, 0.000000000001f);
			const glm::vec3 pos = p.pos;
			impostorVerts[particleIdx] = { pos, particleSize, p.color };
		}
	}
}
