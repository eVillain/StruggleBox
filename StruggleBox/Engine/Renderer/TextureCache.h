#pragma once

#include "RendererDefines.h"
#include <map>
#include <string>

class Texture;

class TextureCache
{
public:
	static const TextureID NO_TEXTURE_ID;

	TextureCache();

	TextureID addTexture(Texture* texture, const std::string& name);
	void removeTexture(const TextureID textureID);

	TextureID getTextureID(const std::string& name);
	Texture* getTextureByID(const TextureID texID);

private:
	TextureID m_nextTextureID;

	friend class RenderCore;
	
	std::map<TextureID, Texture*> m_textures;
	std::map<std::string, TextureID> m_textureNames;
};

