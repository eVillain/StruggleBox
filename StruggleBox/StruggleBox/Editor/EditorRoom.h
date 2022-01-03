#ifndef EDITOR_ROOM_H
#define EDITOR_ROOM_H

#include "Light3D.h"
#include <vector>

class Renderer;

class EditorRoom
{
public:
	EditorRoom(Renderer& renderer);
	~EditorRoom();

	void draw();

	void buildRoom(const float roomWidth, const int tilesPerSide);
	void setDefaultLights();
	void setColoredLights();

	std::vector<LightInstance>& getLights() { return m_lights; }
	const float getRoomWidth() const { return m_roomWidth; }

private:
	Renderer& m_renderer;
	std::vector<LightInstance> m_lights;
	CubeInstance* m_floorCubes;
	int m_numCubes;

	float m_roomWidth;
	int m_tilesPerSide;
	bool m_rotateLights;
};


#endif // !EDITOR_ROOM_H

