#pragma once

#include "RendererDefines.h"
#include <map>
#include <string>

class TextAtlas;

class TextAtlasCache
{
public:
	static const TextAtlasID NO_ATLAS_ID;

	TextAtlasCache();

	TextAtlasID addTextAtlas(TextAtlas* atlas, const std::string& name);

	TextAtlasID getTextAtlasID(const std::string& name);
	TextAtlas* getTextAtlasByID(const TextAtlasID texID);

private:
	TextAtlasID m_nextTextAtlasID;

	std::map<TextAtlasID, TextAtlas*> m_atlases;
	std::map<std::string, TextAtlasID> m_atlasNames;
};

