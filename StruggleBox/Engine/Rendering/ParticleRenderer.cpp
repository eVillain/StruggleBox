#include "ParticleRenderer.h"

#include "VertBuffer.h"
#include "Log.h"

ParticleRenderer::ParticleRenderer(size_t maxParticles, Renderer& renderer, Allocator& allocator)
    : m_renderer(renderer)
	, m_textureID(0)
	, m_vertBuffer(renderer.addVertBuffer(VertexDataType::SpriteVerts))
	, m_vertData(maxParticles, VertexDataType::SpriteVerts, allocator)
	, m_count(0)
{
}

ParticleRenderer::~ParticleRenderer()
{
}

void ParticleRenderer::Update(const glm::vec3& position, const Particle* particles, size_t count)
{
    for (size_t particleIdx = 0; particleIdx < count; particleIdx++)
	{
        const Particle& p = particles[particleIdx];

		const float particleSize = std::max(p.size * 0.5f, 0.000000000001f);
		const glm::vec3 pos = p.pos;
		ColorVertexData vert = {
			pos.x, pos.y, pos.z, particleSize,
			p.color.r, p.color.g, p.color.b, p.color.a
		};
		if (m_vertData.getCount() <= particleIdx)
		{
			m_vertData.buffer(&vert, 1);
		}
		else
		{
			m_vertData.getData()[particleIdx] = vert;
		}
    }

	m_count = count;
}

void ParticleRenderer::Draw(const BlendMode& blendMode, const DepthMode& depthMode, const ParticleSysLighting lighting)
{
	if (m_count == 0)
	{
		return;
	}

	if (lighting == ParticleSysLighting::ParticleSysLightOff)
	{
		m_renderer.queueForwardBuffer(
			m_vertBuffer,
			m_vertData.getData(),
			0,
			m_count,
			m_textureID,
			blendMode,
			depthMode);
	}
	else
	{
		m_renderer.queueDeferredBuffer(
			m_vertBuffer,
			m_vertData.getData(),
			0,
			m_count,
			m_textureID,
			blendMode,
			depthMode);
	} 
}

