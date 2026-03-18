#pragma once

#include "CoreIncludes.h"
#include "RendererDefines.h"

class Texture;
class RenderCore;

class Material
{
public:
	Material();

	void initialize(RenderCore& renderer, bool multithreaded);

	bool allLoaded() const {
		return m_albedo && m_normal && m_metalness && m_roughness && m_displacement && m_emissive;
	}

	const Texture* getAlbedo() const { return m_albedo; }
	const Texture* getNormal() const { return m_normal; }
	const Texture* getMetalness() const { return m_metalness; }
	const Texture* getRoughness() const { return m_roughness; }
	const Texture* getDisplacement() const { return m_displacement; }
	const Texture* getEmissive() const { return m_emissive; }

	const TextureID getAlbedoID() const { return m_albedoID; }
	const TextureID getNormalID() const { return m_normalID; }
	const TextureID getMetalnessID() const { return m_metalnessID; }
	const TextureID getRoughnessID() const { return m_roughnessID; }
	const TextureID getDisplacementID() const { return m_displacementID; }
	const TextureID getEmissiveID() const { return m_emissiveID; }

private:
	const Texture* m_albedo;
	const Texture* m_normal;
	const Texture* m_metalness;
	const Texture* m_roughness;
	const Texture* m_displacement;
	const Texture* m_emissive;

	TextureID m_albedoID;
	TextureID m_normalID;
	TextureID m_metalnessID;
	TextureID m_roughnessID;
	TextureID m_displacementID;
	TextureID m_emissiveID;
};

