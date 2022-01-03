#include "TextureCache.h"

#include "Log.h"

const TextureID TextureCache::NO_TEXTURE_ID = 0;

TextureCache::TextureCache()
	: m_nextTextureID(1)
{
}

TextureID TextureCache::addTexture(Texture2D* texture, const std::string& name)
{
	const TextureID prevID = getTextureID(name);
	if (prevID != NO_TEXTURE_ID)
	{
		Log::Warn("[TextureCache::addTexture] Texture with name %s already cached!", name.c_str());
		return NO_TEXTURE_ID;
	}
	const TextureID textureID = m_nextTextureID;
	m_nextTextureID++;
	m_textures[textureID] = texture;
	m_textureNames[name] = textureID;
	return textureID;
}

TextureID TextureCache::getTextureID(const std::string& name)
{
	const auto it = m_textureNames.find(name);
	if (it != m_textureNames.end())
	{
		return it->second;
	}
	return NO_TEXTURE_ID;
}

Texture2D* TextureCache::getTextureByID(const TextureID textureID)
{
	const auto it = m_textures.find(textureID);
	if (it != m_textures.end())
	{
		return it->second;
	}
	return nullptr;
}
