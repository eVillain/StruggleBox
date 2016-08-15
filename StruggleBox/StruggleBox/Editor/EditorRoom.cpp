#include "EditorRoom.h"
#include "Random.h"
#include "Timer.h"
#include "MaterialData.h"
#include "Renderer.h"

EditorRoom::EditorRoom(std::shared_ptr<Renderer> renderer) :
	_renderer(renderer),
	_floorCubes(nullptr)
{
}

EditorRoom::~EditorRoom()
{
	if (_floorCubes)
		delete[] _floorCubes;
}

void EditorRoom::draw()
{
	if (_rotateLights)
	{
		double timeNow = Timer::Seconds();
		const float lightRotationRadius = _roomWidth*0.25;
		_lights[0].position.x = sin(timeNow)*lightRotationRadius;
		_lights[0].position.z = cos(timeNow)*lightRotationRadius;
	}

	// Queue up our lights
	_renderer->queueLights(_lights, 4);

	// Render the room
	_renderer->bufferCubes(_floorCubes, _numCubes);
}

void EditorRoom::buildRoom(const float roomWidth, const int tilesPerSide)
{
	if (_floorCubes)
		delete[] _floorCubes;

	_roomWidth = roomWidth;
	_tilesPerSide = tilesPerSide;
	const int min = -tilesPerSide / 2;
	const int max = tilesPerSide / 2;
	const float floorTileWidth = (roomWidth / tilesPerSide);
	const float randomFactor = 3.0f;
	_floorCubes = new CubeInstance[tilesPerSide * tilesPerSide * 6];

	Random::RandomSeed(Timer::Seconds());
	// Build room 
	_numCubes = 0;
	for (int y = min; y < max; y++) {
		for (int x = min; x < max; x++) {
			for (int z = min; z < max; z++) {
				if (x > min && x < max - 1 &&
					y > min && y < max - 1 &&
					z > min && z < max - 1)
					continue;
				double tileRandomness = Random::RandomDouble();
				double posX = (x * floorTileWidth) + (floorTileWidth*0.5);
				posX += (x == min || x == max - 1) ? (tileRandomness * randomFactor) : 0;
				double posZ = (z * floorTileWidth) + (floorTileWidth*0.5);
				posZ += (z == min || z == max - 1) ? (tileRandomness * randomFactor) : 0;
				double posY = (y * floorTileWidth) + (floorTileWidth*0.5);
				posY += (y == min || y == max - 1) ? (tileRandomness * randomFactor) : 0;
				double floorGrey = 0.05 + (tileRandomness * 0.05);
				CubeInstance floorCube = {
					posX, posY, posZ, floorTileWidth*0.5,
					0.0f, 0.0f, 0.0f, 1.0f,
					MaterialData::texOffset(50)
				};
				_floorCubes[_numCubes] = floorCube;
				_numCubes++;
			}
		}
	}
	setDefaultLights();
}

void EditorRoom::setDefaultLights()
{
	const float lightRadius = _roomWidth;
	const float lightAmbient = 0.0f;
	const float lightSpacing = _roomWidth*0.25;
	const glm::vec3 lightAttenuation = glm::vec3(0, 0, 1.0f);
	_lights[0].type = Light_Type_Point;
	_lights[0].shadowCaster = true;
	_lights[0].rayCaster = true;
	_lights[0].position = glm::vec4(0.0, 0.0, lightSpacing, lightRadius);
	_lights[0].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	_lights[0].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[1].type = Light_Type_Point;
	_lights[1].shadowCaster = true;
	_lights[1].rayCaster = true;
	_lights[1].position = glm::vec4(-lightSpacing, 0.0, 0.0f, lightRadius);
	_lights[1].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	_lights[1].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[2].type = Light_Type_Point;
	_lights[2].shadowCaster = true;
	_lights[2].rayCaster = true;
	_lights[2].position = glm::vec4(0.0, 0.0, -lightSpacing, lightRadius);
	_lights[2].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	_lights[2].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[3].type = Light_Type_Point;
	_lights[3].shadowCaster = true;
	_lights[3].rayCaster = true;
	_lights[3].position = glm::vec4(lightSpacing, 0.0, 0.0f, lightRadius);
	_lights[3].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	_lights[3].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);

}

void EditorRoom::setColoredLights()
{
	const float lightRadius = _roomWidth;
	const float lightAmbient = 0.0f;
	const float lightSpacing = _roomWidth*0.25;
	const glm::vec3 lightAttenuation = glm::vec3(0, 0, 1.0f);
	_lights[0].type = Light_Type_Point;
	_lights[0].shadowCaster = true;
	_lights[0].rayCaster = true;
	_lights[0].position = glm::vec4(0.0, 0.0, lightSpacing, lightRadius);
	_lights[0].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	_lights[0].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[1].type = Light_Type_Point;
	_lights[1].shadowCaster = true;
	_lights[1].rayCaster = true;
	_lights[1].position = glm::vec4(-lightSpacing, 0.0, 0.0f, lightRadius);
	_lights[1].color = RGBAColor(1.0, 0.0, 0.0, lightAmbient);
	_lights[1].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[2].type = Light_Type_Point;
	_lights[2].shadowCaster = true;
	_lights[2].rayCaster = true;
	_lights[2].position = glm::vec4(0.0, 0.0, -lightSpacing, lightRadius);
	_lights[2].color = RGBAColor(0.0, 1.0, 0.0, lightAmbient);
	_lights[2].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[3].type = Light_Type_Point;
	_lights[3].shadowCaster = true;
	_lights[3].rayCaster = true;
	_lights[3].position = glm::vec4(lightSpacing, 0.0, 0.0f, lightRadius);
	_lights[3].color = RGBAColor(0.0, 0.0, 1.0, lightAmbient);
	_lights[3].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
}
