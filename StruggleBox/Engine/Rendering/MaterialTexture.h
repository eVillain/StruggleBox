#ifndef MATERIAL_TEXTURE_H
#define MATERIAL_TEXTURE_H

#include "MaterialData.h"

class MaterialTexture
{
public:
	MaterialTexture();
	~MaterialTexture();

	void setData(MaterialData& materials);

	GLuint getTexture() { return _texture; }
private:
	GLuint _texture;
};


#endif // !MATERIAL_TEXTURE_H
