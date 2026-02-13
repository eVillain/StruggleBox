#pragma once

#include <string>
#include <map>

#include "RendererDefines.h"

class Allocator;
class Injector;
class Renderer2D;
class Renderer3D;
class Renderer3DDeferred;
class RenderCore;
class ParticleSystem;
class ParticleRenderer;

typedef uint32_t ParticleSystemID;

class Particles
{
public:
    Particles(Allocator& allocator, Injector& injector);
    ~Particles();
    
    ParticleSystemID create(const std::string filePath, const std::string fileName);
    ParticleSystem* getSystemByID(const ParticleSystemID systemID);
    void destroy(const ParticleSystemID systemID);

    void update(const double deltaTime);
    void draw();

private:
    Allocator& m_allocator;
	Renderer2D* m_renderer2D;
    Renderer3D* m_renderer3D;
    Renderer3DDeferred* m_rendererPBR;
    RenderCore* m_renderCore;
    ShaderID m_impostorShaderID;

    std::map<ParticleSystemID, ParticleSystem*> m_systems;
    ParticleSystemID m_nextParticleSysID;
};

