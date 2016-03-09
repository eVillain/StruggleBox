#include "Light3DComponent.h"
#include "EntityManager.h"
#include "Entity.h"
#include "World3D.h"
#include "Locator.h"
#include "LightSystem3D.h"
#include "Light3D.h"

Light3DComponent::Light3DComponent(const int ownerID,
                                   EntityManager* manager,
                                   Locator& locator) :
EntityComponent(ownerID),
_locator(locator)
{
    m_family = "Light3D";
    m_manager = manager;
    m_light = new Light3D();
    m_light->lightType = Light3D_Point;
    _locator.Get<LightSystem3D>()->Add(m_light);
    m_light->position.w = 10.0f;
    m_light->ambient = COLOR_NONE;
    m_light->diffuse = LAColor(1.0f, 3.0f);
    m_light->specular = LAColor(1.0f, 10.0f);;
    m_light->attenuation = glm::vec3(0.5f,0.35f,0.2f);
//        m_light->rayCaster = true;
    m_light->shadowCaster = true;
    offset = glm::vec3();
}

Light3DComponent::~Light3DComponent() {
    
}

void Light3DComponent::Update( double delta ) {
    if ( m_light ) {
        Entity* m_owner = m_manager->GetEntity(m_ownerID);
        glm::vec3 ownerPos = m_owner->GetAttributeDataPtr<glm::vec3>("position");
        glm::quat ownerRot = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
        glm::vec3 lightPos = ownerPos+(ownerRot*offset);
        m_light->position.x = lightPos.x;
        m_light->position.y = lightPos.y;
        m_light->position.z = lightPos.z;
    }
}

void Light3DComponent::Activate() {
    if ( m_light ) {  m_light->active = true; }
}

void Light3DComponent::DeActivate() {
    if ( m_light ) {
        m_light->active = false;
    }
}

