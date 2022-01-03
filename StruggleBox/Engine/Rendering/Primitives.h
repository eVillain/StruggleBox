#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "GFXDefines.h"
#include "CubeConstants.h"
#include "Allocator.h"

static inline const MeshVertexData* createCubeMesh(Allocator& allocator, float materialX, float materialY) 
{
	MeshVertexData* cube = (MeshVertexData*)allocator.allocate(sizeof(MeshVertexData) * 36);
	for (int cubeVert = 0; cubeVert < 36; cubeVert++)
	{
		const int vIndex = cubeVert * 4;
		const int ntIndex = cubeVert * 3;
		const int uvIndex = cubeVert * 3;

		cube[cubeVert] =
		{
			CubeConstants::raw_cube_vertices[vIndex + 0], CubeConstants::raw_cube_vertices[vIndex + 1], CubeConstants::raw_cube_vertices[vIndex + 2], CubeConstants::raw_cube_vertices[vIndex + 3],
			CubeConstants::raw_cube_normals[ntIndex + 0], CubeConstants::raw_cube_normals[ntIndex + 1], CubeConstants::raw_cube_normals[ntIndex + 2],
			CubeConstants::raw_cube_tangents[ntIndex + 0], CubeConstants::raw_cube_tangents[ntIndex + 1], CubeConstants::raw_cube_tangents[ntIndex + 2],
			CubeConstants::raw_cube_texcoords[uvIndex + 0], CubeConstants::raw_cube_texcoords[uvIndex + 1],
			materialX, materialY
		};
	}

	return cube;
}

#endif // !PRIMITIVES_H