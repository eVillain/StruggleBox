#pragma once

#include "Color.h"
#include "glm/glm.hpp"

#include <string>
#include <vector>

struct MaterialDef
{
	Color albedo;
	float roughness;
	float metalness;
	float noiseAmount;
	float noiseScale;
};

class MaterialData
{
public:
	MaterialData();
	void load(const std::string& fileName);
	void save(const std::string& fileName);

	MaterialDef& operator[] (uint8_t id) { return _data[id-1]; }
	const std::string& getName(const uint8_t id) { return _names[id-1]; };
	void setName(const uint8_t id, std::string& name) { _names[id-1] = name; };

	static const glm::vec2 texOffset(const uint8_t materialID)
	{
		glm::ivec2 coords = indexToCoords(materialID);
		return glm::vec2(coords.x / 16.f, coords.y / 16.f);
	}
	static const float texOffsetX(const uint8_t materialID)
	{
		return (materialID % 16) / 16.f;
	}
	static const float texOffsetY(const uint8_t materialID)
	{
		return (materialID / 16) / 16.f;
	}

	static glm::ivec2 indexToCoords(uint8_t materialID)
	{
		return glm::ivec2(materialID % 16, materialID / 16);
	}

	static uint8_t coordsToIndex(glm::ivec2 coords)
	{
		return (coords.y * 16) + coords.x;
	}

private:
	MaterialDef _data[255];
	std::string _names[255];
};

