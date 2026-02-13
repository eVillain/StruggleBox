#include "TextureAtlasCache.h"

#include "TextureAtlas.h"
#include "Log.h"

const TextureAtlasID TextureAtlasCache::NO_ATLAS_ID = 0;

TextureAtlasCache::TextureAtlasCache()
	: m_nextTextureAtlasID(1)
{
}

TextureAtlasID TextureAtlasCache::addTextureAtlas(TextureAtlas* texture, const std::string& name)
{
	const TextureAtlasID prevID = getTextureAtlasID(name);
	if (prevID != NO_ATLAS_ID)
	{
		Log::Warn("Texture with name %s already cached!", name.c_str());
		return NO_ATLAS_ID;
	}
	const TextureAtlasID atlasID = m_nextTextureAtlasID;
	m_nextTextureAtlasID++;
	m_atlases[atlasID] = texture;
	m_atlasNames[name] = atlasID;
	return atlasID;
}

void TextureAtlasCache::removeTextureAtlas(const TextureAtlasID atlasID)
{
	auto it = m_atlases.begin();
	if (it != m_atlases.end())
	{
		m_atlases.erase(it);
	}
	auto it2 = m_atlasNames.begin();
	while (it2 != m_atlasNames.end())
	{
		if (it2->second == atlasID)
		{
			m_atlasNames.erase(it2);
			break;
		}
		it2++;
	}
}

TextureAtlasID TextureAtlasCache::getTextureAtlasID(const std::string& name)
{
	const auto it = m_atlasNames.find(name);
	if (it != m_atlasNames.end())
	{
		return it->second;
	}
	return NO_ATLAS_ID;
}

TextureAtlas* TextureAtlasCache::getTextureAtlasByID(const TextureAtlasID textureID)
{
	const auto it = m_atlases.find(textureID);
	if (it != m_atlases.end())
	{
		return it->second;
	}
	return nullptr;
}
