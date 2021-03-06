#include "Scene.h"

Scene::Scene(const std::string sceneID) :
_sceneID(sceneID),
_init(false),
_paused(false)
{ }

const std::string Scene::GetID() const
{
    return _sceneID;
}

void Scene::Initialize()
{
    if(_init == false)
    {
        _init = true;
        _paused = false;
    }
}

void Scene::Release(void)
{
    if(_init == true)
    {
        _init = false;
    }
}

void Scene::Pause(void)
{
    if (_paused == false)
    {
        _paused = true;
    }
}

void Scene::Resume(void)
{
    if (_paused == true)
    {
        _paused = false;
    }
}
