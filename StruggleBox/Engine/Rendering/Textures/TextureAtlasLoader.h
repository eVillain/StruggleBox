#pragma once

#include "RendererDefines.h"
#include <string>

class Allocator;
class TextureAtlas;
class Renderer;

class AtlasLoader
{
public:
	static TextureAtlas* load(const std::string& atlasFile, Allocator& allocator, Renderer& renderer);
};

