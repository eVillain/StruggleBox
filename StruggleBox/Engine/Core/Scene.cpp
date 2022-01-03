#include "Scene.h"

Scene::Scene(const std::string sceneID) :
m_sceneID(sceneID),
m_init(false),
m_paused(false)
{ }

const std::string Scene::GetID() const
{
    return m_sceneID;
}

void Scene::Initialize()
{
    if(m_init == false)
    {
        m_init = true;
        m_paused = false;
    }
}

void Scene::Release(void)
{
    if(m_init == true)
    {
        m_init = false;
    }
}

void Scene::Pause(void)
{
    if (m_paused == false)
    {
        m_paused = true;
    }
}

void Scene::Resume(void)
{
    if (m_paused == true)
    {
        m_paused = false;
    }
}
