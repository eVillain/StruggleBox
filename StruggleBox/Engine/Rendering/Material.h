#pragma once

#include "CoreIncludes.h"

class Texture2D;
class Renderer;

class Material
{
public:
	Material();

	void initialize(Renderer& renderer, bool multithreaded);

	bool allLoaded() const {
		return m_albedo && m_normal && m_metalness && m_roughness && m_displacement && m_emissive;
	}

	const Texture2D* getAlbedo() const { return m_albedo; }
	const Texture2D* getNormal() const { return m_normal; }
	const Texture2D* getMetalness() const { return m_metalness; }
	const Texture2D* getRoughness() const { return m_roughness; }
	const Texture2D* getDisplacement() const { return m_displacement; }
	const Texture2D* getEmissive() const { return m_emissive; }

private:
	const Texture2D* m_albedo;
	const Texture2D* m_normal;
	const Texture2D* m_metalness;
	const Texture2D* m_roughness;
	const Texture2D* m_displacement;
	const Texture2D* m_emissive;
};

