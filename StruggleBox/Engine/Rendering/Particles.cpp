#include "Particles.h"
#include "Renderer.h"
#include "Log.h"

int Particles::_nextParticleSysID = 0;

Particles::Particles(std::shared_ptr<Renderer> renderer) :
	_renderer(renderer),
	_paused(false)
{
	Log::Info("[Particles] constructor, instance at %p", this);
}

Particles::~Particles()
{
	Log::Info("[Particles] destructor, instance at %p", this);
}

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
	std::shared_ptr<ParticleSys> sys = std::make_shared<ParticleSys>(
		_renderer,
		filePath,
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
	for (it = _systems.begin(); it != _systems.end(); it++)
	{
		if (it->second == system)
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

void Particles::Draw()
{
	for (auto pair : _systems)
	{
		pair.second->Draw();
	}
}

void Particles::drawUnlit()
{
	for (auto pair : _systems)
	{
		if (pair.second->lighting == ParticleSysLightOff)
		{
			pair.second->Draw();
		}
	}
}

void Particles::drawLit()
{
	for (auto pair : _systems)
	{
		if (pair.second->lighting == ParticleSysLightOn)
		{
			pair.second->Draw();
		}
	}
}
