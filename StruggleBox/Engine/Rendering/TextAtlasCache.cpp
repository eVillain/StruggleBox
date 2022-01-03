#include "TextAtlasCache.h"

#include "TextAtlas.h"
#include "Log.h"

const TextAtlasID TextAtlasCache::NO_ATLAS_ID = 0;

TextAtlasCache::TextAtlasCache()
	: m_nextTextAtlasID(1)
{
}

TextAtlasID TextAtlasCache::addTextAtlas(TextAtlas* texture, const std::string& name)
{
	const TextAtlasID prevID = getTextAtlasID(name);
	if (prevID != NO_ATLAS_ID)
	{
		Log::Warn("Text atlas with name %s already cached!", name.c_str());
		return NO_ATLAS_ID;
	}
	const TextAtlasID atlasID = m_nextTextAtlasID;
	m_nextTextAtlasID++;
	m_atlases[atlasID] = texture;
	m_atlasNames[name] = atlasID;
	return atlasID;
}

TextAtlasID TextAtlasCache::getTextAtlasID(const std::string& name)
{
	const auto it = m_atlasNames.find(name);
	if (it != m_atlasNames.end())
	{
		return it->second;
	}
	return NO_ATLAS_ID;
}

TextAtlas* TextAtlasCache::getTextAtlasByID(const TextAtlasID textureID)
{
	const auto it = m_atlases.find(textureID);
	if (it != m_atlases.end())
	{
		return it->second;
	}
	return nullptr;
}
