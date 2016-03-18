#include "Particles.h"
#include "Locator.h"
#include "Renderer.h"

int Particles::_nextParticleSysID = 0;

Particles::Particles(Locator& locator) :
_locator(locator),
_paused(false)
{ }

Particles::~Particles()
{ }

void Particles::Update(double deltaTime)
{
    if (_paused) return;
    for (auto pair : _systems)
    {
        pair.second->Update(deltaTime);
    }
}

std::shared_ptr<ParticleSys> Particles::create(const std::string filePath,
                                                     const std::string fileName)
{
    std::shared_ptr<ParticleSys> sys = std::make_shared<ParticleSys>(filePath,
                                                                     fileName);
    int sysID = _nextParticleSysID++;
    _systems[sysID] = sys;
    return sys;
}

int Particles::getSystemID(std::shared_ptr<ParticleSys> system)
{
    for (auto pair : _systems)
    {
        if (pair.second == system)
        {
            return pair.first;
        }
    }
    return -1;
}

void Particles::destroy(std::shared_ptr<ParticleSys> system)
{
    std::map<int, std::shared_ptr<ParticleSys>>::iterator it;
    for (it=_systems.begin(); it != _systems.end(); it++)
    {
        if ( it->second == system )
        {
            // Found system to delete
            _systems.erase(it);
            return;
        }
    }
}

void Particles::destroy(const int sysID)
{
    if (_systems.find(sysID) != _systems.end())
    {
        _systems.erase(sysID);
    }
}

void Particles::Draw(Renderer* renderer)
{
    for (auto pair : _systems)
    {
        pair.second->Draw(renderer);
    }
}

void Particles::drawUnlit(Renderer* renderer)
{
    if (_paused) return;
    for (auto pair : _systems)
    {
        if (pair.second->lighting == ParticleSysLightOff)
        {
            pair.second->Draw(renderer);
        }
    }
}

void Particles::drawLit(Renderer* renderer)
{
    if (_paused) return;
    for (auto pair : _systems)
    {
        if (pair.second->lighting == ParticleSysLightOn)
        {
            pair.second->Draw(renderer);
        }
    }
}
