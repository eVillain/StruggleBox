#pragma once

#include "RendererDefines.h"
#include <map>
#include <string>

class TextureAtlas;

class FrameCache
{
public:
	FrameCache();

	void addAtlas(TextureAtlas* atlas, TextureAtlasID atlasID);

	TextureAtlasID getAtlasForFrame(const std::string& frame) const;

private:
	std::map<std::string, TextureAtlasID> m_frameCache;
};

