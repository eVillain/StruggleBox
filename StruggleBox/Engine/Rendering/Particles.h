#pragma once

#include <string>
#include <map>

class Allocator;
class Renderer;
class ParticleSystem;
class ParticleRenderer;

typedef uint32_t ParticleSystemID;

class Particles
{
public:
    Particles(Allocator& allocator, Renderer& renderer);
    ~Particles();
    
    ParticleSystemID create(const std::string filePath, const std::string fileName);
    ParticleSystem* getSystemByID(const ParticleSystemID systemID);
    void destroy(const ParticleSystemID systemID);

    void update(const double deltaTime);
    void draw();

private:
    Allocator& m_allocator;
	Renderer& m_renderer;

    std::map<ParticleSystemID, ParticleSystem*> m_systems;
    std::map<ParticleSystemID, ParticleRenderer*> m_renderers;
    ParticleSystemID m_nextParticleSysID;
};

