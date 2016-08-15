#ifndef EDITOR_ROOM_H
#define EDITOR_ROOM_H

#include "Light3D.h"
#include <memory>
class Renderer;

class EditorRoom
{
public:
	EditorRoom(std::shared_ptr<Renderer> renderer);
	~EditorRoom();

	void draw();

	void buildRoom(const float roomWidth, const int tilesPerSide);
	void setDefaultLights();
	void setColoredLights();

	const float getRoomWidth() const { return _roomWidth; }
private:
	std::shared_ptr<Renderer> _renderer;
	LightInstance _lights[4];
	CubeInstance* _floorCubes;
	int _numCubes;

	float _roomWidth;
	int _tilesPerSide;

	bool _rotateLights = false;
};


#endif // !EDITOR_ROOM_H

