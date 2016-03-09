#include "ParticleComponent.h"
#include "Locator.h"
#include "EntityManager.h"
#include "FileUtil.h"
#include "ParticleManager.h"
#include "ParticleSys.h"
#include "World3D.h"
#include "Entity.h"

ParticleComponent::ParticleComponent(const int ownerID,
                                     const std::string& fileName,
                                     Locator& locator) :
EntityComponent(ownerID),
_locator(locator)
{
    m_family = "Particle";
    if ( fileName.length() != 0 ) { // Load particle system from filename given
        _particleSys = _locator.Get<ParticleManager>()->AddSystem(FileUtil::GetPath().append("Data/Particles/"), fileName);
//        m_particleSys->duration = -1.0f;
        offset = glm::vec3();
    } else {    // No filename given, load from attributes
        Entity* m_owner = _locator.Get<EntityManager>()->GetEntity(m_ownerID);
        std::string particleFile = m_owner->GetAttributeDataPtr<std::string>("particleFile");
        _particleSys = _locator.Get<ParticleManager>()->AddSystem(FileUtil::GetPath().append("Data/Particles/"), particleFile);
        offset = m_owner->GetAttributeDataPtr<glm::vec3>("particleOffset");
    }
}

ParticleComponent::~ParticleComponent()
{
    if (_particleSys ) {
        _locator.Get<ParticleManager>()->RemoveSystem(_particleSys);
        _particleSys = NULL;
    }
}

void ParticleComponent::Update(double delta)
{
    if ( _particleSys ) {
        Entity* m_owner = _locator.Get<EntityManager>()->GetEntity(m_ownerID);
        glm::vec3 ownerPos = m_owner->GetAttributeDataPtr<glm::vec3>("position");
        glm::quat ownerRot = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
        _particleSys->sourcePos = ownerPos+(ownerRot*offset);
    }
}
void ParticleComponent::Activate()
{
    if ( _particleSys ) {  _particleSys->active = true; }
}

void ParticleComponent::DeActivate()
{
    if ( _particleSys ) {  _particleSys->active = false; }
}

