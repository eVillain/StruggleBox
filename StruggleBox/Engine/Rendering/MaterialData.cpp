#include "MaterialData.h"
#include "Dictionary.h"
#include "Log.h"

MaterialData::MaterialData()
{
	for (size_t i = 0; i < 255; i++)
	{
		_data[i] = { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 };
	}
}

void MaterialData::load(const std::string& fileName)
{
	Dictionary dict;
	dict.loadRootSubDictFromFile(fileName.c_str());
	uint32_t numKeys = dict.getNumKeys();
	for (uint32_t key = 0; key < numKeys; key++)
	{
		std::string materialName = dict.getKey(key);
		if (materialName == "MaterialsVersion")
		{
			std::string version = dict.getStringForKey("MaterialsVersion");
			if (version != "0.0")
			{
				Log::Error("[Materials] loaded wrong version materials!");
			}
		}
		else
		{
			dict.stepIntoSubDictWithKey(materialName.c_str());
			int materialID = dict.getIntegerForKey("ID");
			if (materialID == 0) materialID = 1;
			setName(materialID, materialName);
			MaterialDef& material = _data[materialID - 1];
			material.albedo.r = dict.getFloatForKey("AlbedoR");
			material.albedo.g = dict.getFloatForKey("AlbedoG");
			material.albedo.b = dict.getFloatForKey("AlbedoB");
			material.albedo.a = dict.getFloatForKey("AlbedoA");
			material.roughness = dict.getFloatForKey("Roughness");
			material.metalness = dict.getFloatForKey("Metalness");
			material.noiseAmount = dict.getFloatForKey("NoiseAmount");
			material.noiseScale = dict.getFloatForKey("NoiseScale");
			dict.stepOutOfSubDict();
		}
	}
}

void MaterialData::save(const std::string& fileName)
{
	Dictionary dict;
	dict.setStringForKey("MaterialsVersion", "0.0");
	for (size_t i = 1; i < 256; i++)
	{
		dict.setSubDictForKey(getName(i).c_str());
		dict.stepIntoSubDictWithKey(getName(i).c_str());
		MaterialDef& material = _data[i-1];
		dict.setIntegerForKey("ID", i);
		dict.setFloatForKey("AlbedoR", material.albedo.r);
		dict.setFloatForKey("AlbedoG", material.albedo.g);
		dict.setFloatForKey("AlbedoB", material.albedo.b);
		dict.setFloatForKey("AlbedoA", material.albedo.a);
		dict.setFloatForKey("Roughness", material.roughness);
		dict.setFloatForKey("Metalness", material.metalness);
		dict.setFloatForKey("NoiseAmount", material.noiseAmount);
		dict.setFloatForKey("NoiseScale", material.noiseScale);
		dict.stepOutOfSubDict();
	}
	dict.saveRootSubDictToFile(fileName.c_str());
}
