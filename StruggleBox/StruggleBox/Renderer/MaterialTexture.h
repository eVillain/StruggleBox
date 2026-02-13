#pragma once

#include "MaterialData.h"

class MaterialTexture
{
public:
	MaterialTexture();
	~MaterialTexture();

	void setData(MaterialData& materials);

	GLuint getTexture() { return m_texture; }
private:
	GLuint m_texture;
};
