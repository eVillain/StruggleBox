#include "RenderComponent.h"

#include "EntityManager.h"
#include "Entity.h"
#include "Renderer.h"

RenderComponent::RenderComponent(
	const int ownerID,
	EntityManager& entityManager,
	Renderer& renderer)
	: EntityComponent(ownerID, "Render")
	, m_entityManager(entityManager)
	, m_renderer(renderer)
	, m_type(RenderComponentType::Type_Rendering_Off)
	, m_materialID(0)
	, m_radius(1.f)
{
}

RenderComponent::~RenderComponent()
{
}

void RenderComponent::update(const double delta)
{
	if (m_type == RenderComponentType::Type_Rendering_Off)
	{
		return;
	}
	Entity* owner = m_entityManager.getEntity(_ownerID);
	const glm::vec3 pos = owner->GetAttributeDataPtr<glm::vec3>("position");

	if (m_type == RenderComponentType::Type_Sprite_Sphere)
	{
		SphereVertexData sphere = { pos.x, pos.y, pos.z, m_radius,
			MaterialData::texOffsetX(m_materialID), MaterialData::texOffsetY(m_materialID)
		};
		m_renderer.BufferSpheres(&sphere, 1);
	}
	else if (m_type == RenderComponentType::Type_Sprite_Fireball)
	{
		const float radius = owner->GetAttributeDataPtr<float>("sphereRadius");
		const float lifeTime = owner->GetAttributeDataPtr<float>("lifeTime");
		SphereVertexData sphere = { pos.x, pos.y, pos.z, radius * 2.f,
			lifeTime * 2.f, 0.f
		};
		m_renderer.BufferFireballs(&sphere, 1);
	}
}
