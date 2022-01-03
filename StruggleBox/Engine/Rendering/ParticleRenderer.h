#pragma once

#include "GFXDefines.h"
#include "Texture2D.h"
#include "Renderer.h"
#include "VertexData.h"
#include "ParticleSys.h"

class Allocator;

class ParticleRenderer
{
public:
    ParticleRenderer(size_t maxParticles, Renderer& renderer, Allocator& allocator);
    ~ParticleRenderer();

    void Update(const glm::vec3& position, const Particle* particles, size_t count);
    void Draw(const BlendMode& blendMode, const DepthMode& depthMode, const ParticleSysLighting lighting);

    void setTextureID(TextureID textureID) { m_textureID = textureID; }

private:
    Renderer& m_renderer;
    TextureID m_textureID;
    VertBuffer* m_vertBuffer;
    size_t m_count;
	VertexData<ColorVertexData> m_vertData;
};
