#include "MaterialTexture.h"
#include "RenderUtils.h"
#include "Log.h"

MaterialTexture::MaterialTexture() :
	_texture(0)
{
	Log::Debug("[MaterialTexture] constructor, instance at: %p", this);
}

MaterialTexture::~MaterialTexture()
{
	Log::Debug("[MaterialTexture] constructor, instance at: %p", this);
	glDeleteTextures(1, &_texture);
}

void MaterialTexture::setData(MaterialData & materials)
{

	GLfloat data[256 * 2 * 4];
	const GLuint secondRow = 256 * 4;
	data[0] = 1.0f;
	data[1] = 1.0f;
	data[2] = 1.0f;
	data[3] = 1.0f;
	data[secondRow + 0] = 1.0f;
	data[secondRow + 1] = 1.0f;
	data[secondRow + 2] = 1.0f;
	data[secondRow + 3] = 1.0f;

	for (size_t i = 1; i < 256; i++)
	{
		const MaterialDef& material = materials[i];
		data[i * 4 + 0] = material.albedo.r;
		data[i * 4 + 1] = material.albedo.g;
		data[i * 4 + 2] = material.albedo.b;
		data[i * 4 + 3] = material.albedo.a;
		data[i * 4 + 0 + secondRow] = material.roughness;
		data[i * 4 + 1 + secondRow] = material.metalness;
		data[i * 4 + 2 + secondRow] = material.noiseAmount;
		data[i * 4 + 3 + secondRow] = material.noiseScale;
	}

	if (!_texture)
	{
		_texture = RenderUtils::GenerateTextureRGBAF(256, 2);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 2, GL_RGBA, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}
