#pragma once

#include "RendererDefines.h"
#include <string>

class Allocator;
class TextureAtlas;
class RenderCore;

class TextureAtlasLoader
{
public:
	static TextureAtlas* load(const std::string& atlasFile, Allocator& allocator, RenderCore& renderCore);
};

