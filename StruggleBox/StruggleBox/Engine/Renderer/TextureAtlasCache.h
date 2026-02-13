#pragma once

#include "RendererDefines.h"
#include <map>
#include <string>

class TextureAtlas;

class TextureAtlasCache
{
public:
	static const TextureAtlasID NO_ATLAS_ID;

	TextureAtlasCache();

	TextureAtlasID addTextureAtlas(TextureAtlas* atlas, const std::string& name);
	void removeTextureAtlas(const TextureAtlasID atlasID);

	TextureAtlasID getTextureAtlasID(const std::string& name);
	TextureAtlas* getTextureAtlasByID(const TextureAtlasID atlasID);

private:
	TextureAtlasID m_nextTextureAtlasID;

	friend class RenderCore;
	std::map<TextureAtlasID, TextureAtlas*> m_atlases;
	std::map<std::string, TextureAtlasID> m_atlasNames;
};

