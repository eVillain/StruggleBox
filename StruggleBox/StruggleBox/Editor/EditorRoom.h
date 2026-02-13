#pragma once

#include "RendererDefines.h"
#include "Lighting3DDeferred.h"
#include <vector>

class VoxelRenderer;

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
	std::vector<LightInstance> m_lights;
	DrawDataID m_voxelInstancesDrawDataID;
	ShaderID m_voxelInstancesShaderID;

	int m_numCubes;

	float m_roomWidth;
	int m_tilesPerSide;
	bool m_rotateLights;
};
