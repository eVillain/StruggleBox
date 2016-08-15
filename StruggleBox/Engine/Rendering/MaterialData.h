#ifndef MATERIAL_DATA_H
#define MATERIAL_DATA_H

#include "Color.h"
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

	static const GLfloat texOffset(const GLuint materialID)
	{
		return GLfloat(materialID) / 255.0f;
	}
private:
	MaterialDef _data[255];
	std::string _names[255];
};

#endif
