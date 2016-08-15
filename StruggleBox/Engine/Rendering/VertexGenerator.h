#ifndef VERTEX_GENERATOR_H
#define VERTEX_GENERATOR_H

#include "GFXDefines.h"
class VertexGenerator
{
public:

	void sphere(float radius, unsigned int rings, unsigned int sectors)
	{
		float const R = 1. / (float)(rings - 1);
		float const S = 1. / (float)(sectors - 1);
		int r, s;

		ColorVertexData* verts = new ColorVertexData[rings * sectors];

		for (r = 0; r < rings; r++) for (s = 0; s < sectors; s++) {
			float const y = sin(-M_PI_2 + M_PI * r * R);
			float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);
			int i = r*sectors + s;
			verts[i] = { x * radius,y * radius, z * radius, 1,
				x, y, z
			};
		}
		GLushort* indices = new GLushort[rings * sectors * 4];
		GLushort* i = indices;
		for (r = 0; r < rings - 1; r++) for (s = 0; s < sectors - 1; s++) {
			*i++ = r * sectors + s;
			*i++ = r * sectors + (s + 1);
			*i++ = (r + 1) * sectors + (s + 1);
			*i++ = (r + 1) * sectors + s;
		}

		//    BufferVerts(nverts, rings*sectors);
		delete[] verts;
		delete[] indices;
	}
};



#endif
