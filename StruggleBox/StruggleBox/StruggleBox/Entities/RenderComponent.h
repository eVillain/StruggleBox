#pragma once

#include "EntityComponent.h"

class EntityManager;
class VoxelRenderer;

enum class RenderComponentType {
    Type_Rendering_Off,
    Type_Sprite_Sphere,
    Type_Sprite_Fireball
};

class RenderComponent : public EntityComponent
{
public:
    RenderComponent(const int ownerID,
		EntityManager& entityManager,
        VoxelRenderer& renderer);
    ~RenderComponent();

    void update(const double delta);
    
    void setType(RenderComponentType type) { m_type = type; }
	RenderComponentType getType() const { return m_type; }

    void setMaterialID(uint8_t materialID) { m_materialID = materialID; }
    uint8_t getMaterialID() const { return m_materialID; }

    void setRadius(float radius) { m_radius = radius; }
    float getRadius() const { return m_radius; }
private:
	EntityManager& m_entityManager;
    VoxelRenderer& m_renderer;

    RenderComponentType m_type;
    uint8_t m_materialID;
    float m_radius;
};
