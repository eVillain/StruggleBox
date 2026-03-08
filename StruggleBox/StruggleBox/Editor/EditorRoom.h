#pragma once

#include "RendererDefines.h"
#include "Lighting3DDeferred.h"
#include "MaterialData.h"
#include "VoxelRenderer.h"
#include <vector>
#include <stdint.h>

class EditorRoom
{
public:
	EditorRoom(VoxelRenderer& renderer);
	~EditorRoom();

	void draw();

	void buildRoom(const float roomWidth, const int tilesPerSide);
	void setDefaultLights();
	void setColoredLights();

	std::vector<LightInstance>& getLights() { return m_lights; }
	const float getRoomWidth() const { return m_roomWidth; }

private:
	VoxelRenderer& m_renderer;
	DrawDataID m_voxelInstancesDrawDataID;
	ShaderID m_voxelInstancesShaderID;

	size_t m_numCubes;

	float m_roomWidth;
	int m_tilesPerSide;
	bool m_rotateLights;
	CubeInstanceTransform3DData* m_roomCubes;
	MaterialData m_materialData;
	std::vector<LightInstance> m_lights;
};
