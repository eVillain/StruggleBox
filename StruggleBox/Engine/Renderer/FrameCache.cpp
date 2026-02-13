#include "FrameCache.h"

#include "TextureAtlas.h"
#include "TextureAtlasCache.h"

FrameCache::FrameCache()
{
}

void FrameCache::addAtlas(TextureAtlas* atlas, TextureAtlasID atlasID)
{
	const auto& frames = atlas->getFrames();
	for (const auto& pair : frames)
	{
		m_frameCache[pair.first] = atlasID;
	}
}

TextureAtlasID FrameCache::getAtlasForFrame(const std::string& frame) const
{
	const auto it = m_frameCache.find(frame);
	if (it == m_frameCache.end())
	{
		return TextureAtlasCache::NO_ATLAS_ID;
	}
	return it->second;
}
