#include "Light3DComponent.h"
#include "EntityManager.h"
#include "Entity.h"
#include "Renderer.h"

Light3DComponent::Light3DComponent(
	const int ownerID,
	std::shared_ptr<EntityManager> entityManager) :
	EntityComponent(ownerID, "Light3D"),
	_entityManager(entityManager)
{
	_light.type = Light_Type_Point;
	_light.position.w = 10.0f;
	_light.color = LAColor(1.0f, 0.0f);;
	_light.attenuation = glm::vec3(0.5f, 0.35f, 0.2f);
	_light.shadowCaster = true;
	offset = glm::vec3();
}

Light3DComponent::~Light3DComponent()
{ }

void Light3DComponent::update(const double delta)
{
	Entity* m_owner = _entityManager->getEntity(_ownerID);
	glm::vec3 ownerPos = m_owner->GetAttributeDataPtr<glm::vec3>("position");
	glm::quat ownerRot = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
	glm::vec3 lightPos = ownerPos + (ownerRot*offset);
	_light.position.x = lightPos.x;
	_light.position.y = lightPos.y;
	_light.position.z = lightPos.z;
}

void Light3DComponent::activate()
{
	_light.active = true;
}

void Light3DComponent::deActivate()
{

	_light.active = false;
}
