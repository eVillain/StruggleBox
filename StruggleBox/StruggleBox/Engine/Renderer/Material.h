#pragma once

#include "CoreIncludes.h"
#include "RendererDefines.h"

class Texture2D;
class RenderCore;

class Material
{
public:
	Material();

	void initialize(RenderCore& renderer, bool multithreaded);

	bool allLoaded() const {
		return m_albedo && m_normal && m_metalness && m_roughness && m_displacement && m_emissive;
	}

	const Texture2D* getAlbedo() const { return m_albedo; }
	const Texture2D* getNormal() const { return m_normal; }
	const Texture2D* getMetalness() const { return m_metalness; }
	const Texture2D* getRoughness() const { return m_roughness; }
	const Texture2D* getDisplacement() const { return m_displacement; }
	const Texture2D* getEmissive() const { return m_emissive; }

	const TextureID getAlbedoID() const { return m_albedoID; }
	const TextureID getNormalID() const { return m_normalID; }
	const TextureID getMetalnessID() const { return m_metalnessID; }
	const TextureID getRoughnessID() const { return m_roughnessID; }
	const TextureID getDisplacementID() const { return m_displacementID; }
	const TextureID getEmissiveID() const { return m_emissiveID; }

private:
	const Texture2D* m_albedo;
	const Texture2D* m_normal;
	const Texture2D* m_metalness;
	const Texture2D* m_roughness;
	const Texture2D* m_displacement;
	const Texture2D* m_emissive;

	TextureID m_albedoID;
	TextureID m_normalID;
	TextureID m_metalnessID;
	TextureID m_roughnessID;
	TextureID m_displacementID;
	TextureID m_emissiveID;
};

