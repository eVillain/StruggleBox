#pragma once

#include "RendererDefines.h"
#include <map>
#include <string>

class Texture2D;

class TextureCache
{
public:
	static const TextureID NO_TEXTURE_ID;

	TextureCache();

	TextureID addTexture(Texture2D* texture, const std::string& name);

	TextureID getTextureID(const std::string& name);
	Texture2D* getTextureByID(const TextureID texID);

private:
	TextureID m_nextTextureID;

	std::map<TextureID, Texture2D*> m_textures;
	std::map<std::string, TextureID> m_textureNames;
};

