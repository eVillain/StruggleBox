#include "EditorRoom.h"
#include "Random.h"
#include "Timer.h"
#include "MaterialData.h"
#include "Renderer.h"
#include "Log.h"

EditorRoom::EditorRoom(Renderer& renderer)
	: m_renderer(renderer)
	, m_lights(4)
	, m_floorCubes(nullptr)
	, m_numCubes(0)
	, m_roomWidth(0.f)
	, m_tilesPerSide(0)
	, m_rotateLights(false)
{
	Log::Info("EditorRoom constructed at %p", this);
}

EditorRoom::~EditorRoom()
{
	if (m_floorCubes)
	{ 
		delete[] m_floorCubes;
	}
}

void EditorRoom::draw()
{
	if (m_rotateLights)
	{
		double timeNow = Timer::Seconds();
		const float lightRotationRadius = m_roomWidth*0.25;
		m_lights[0].position.x = sin(timeNow)*lightRotationRadius;
		m_lights[0].position.z = cos(timeNow)*lightRotationRadius;
	}

	if (!m_lights.empty())
	{
		m_renderer.queueLights(m_lights.data(), m_lights.size());
	}
	if (m_numCubes)
	{
		m_renderer.bufferCubes(m_floorCubes, m_numCubes);

	}
}

void EditorRoom::buildRoom(const float roomWidth, const int tilesPerSide)
{
	if (m_floorCubes)
	{
		delete[] m_floorCubes;
	}

	const bool randomBlockTypes = true;

	m_roomWidth = roomWidth;
	m_tilesPerSide = tilesPerSide;
	const int min = -tilesPerSide / 2;
	const int max = tilesPerSide / 2;
	const float floorTileWidth = (roomWidth / tilesPerSide);
	const float randomFactor = 0.0f;
	m_floorCubes = new CubeInstance[tilesPerSide * tilesPerSide * 6];

	Random::RandomSeed(Timer::Seconds());
	// Build room 
	m_numCubes = 0;
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

				GLuint blockType = 1;
				if (randomBlockTypes)
				{
					blockType = Random::RandomInt(198, 200);
				}
				CubeInstance floorCube = {
					posX, posY, posZ, floorTileWidth*0.5,
					0.0f, 0.0f, 0.0f, 1.0f,
					MaterialData::texOffsetX(blockType), MaterialData::texOffsetY(blockType)
				};
				m_floorCubes[m_numCubes] = floorCube;
				m_numCubes++;
			}
		}
	}

	m_renderer.setRoomSize(roomWidth - (floorTileWidth * 2.f));

	setDefaultLights();
	//setColoredLights();
}

void EditorRoom::setDefaultLights()
{
	const float lightRadius = m_roomWidth * 0.5f;
	const float lightAmbient = 0.0f;
	const float lightSpacing = m_roomWidth * 0.25f;
	const glm::vec3 lightAttenuation = glm::vec3(1.f, 0.05f, 0.02f);
	m_lights[0].type = Light_Type_Point;
	m_lights[0].shadowCaster = true;
	m_lights[0].raySize = 32.f;
	m_lights[0].position = glm::vec4(0.f, lightSpacing, 0.f, lightRadius); // top
	m_lights[0].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	m_lights[0].attenuation = lightAttenuation;
	m_lights[1].type = Light_Type_Point;
	m_lights[1].shadowCaster = true;
	m_lights[1].raySize = 16.f;
	m_lights[1].position = glm::vec4(lightSpacing, 0.f, lightSpacing, lightRadius); // left
	m_lights[1].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	m_lights[1].attenuation = lightAttenuation;
	m_lights[2].type = Light_Type_Point;
	m_lights[2].shadowCaster = true;
	m_lights[2].raySize = 8.f;
	m_lights[2].position = glm::vec4(0.0, -lightSpacing, 0.f, lightRadius); // bottom
	m_lights[2].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	m_lights[2].attenuation = lightAttenuation;
	m_lights[3].type = Light_Type_Point;
	m_lights[3].shadowCaster = true;
	m_lights[3].raySize = 8.f;
	m_lights[3].position = glm::vec4(-lightSpacing, 0.f, lightSpacing, lightRadius); // right
	m_lights[3].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	m_lights[3].attenuation = lightAttenuation;
}

void EditorRoom::setColoredLights()
{
	const float lightRadius = m_roomWidth * 0.5f;
	const float lightAmbient = 0.0f;
	const float lightSpacing = m_roomWidth * 0.125f;
	const glm::vec3 lightAttenuation = glm::vec3(1.f, 0.15f, 0.15f);
	m_lights[0].type = Light_Type_Point;
	m_lights[0].shadowCaster = true;
	m_lights[0].raySize = 32.f;
	m_lights[0].position = glm::vec4(-lightRadius*0.5f, lightRadius * 0.5f, -lightRadius * 0.5f, m_roomWidth); // top corner
	m_lights[0].color = RGBAColor(1.0, 1.0, 1.0, lightAmbient);
	m_lights[0].attenuation = lightAttenuation;
	m_lights[1].type = Light_Type_Point;
	m_lights[1].shadowCaster = true;
	m_lights[0].raySize = 32.f;
	m_lights[1].position = glm::vec4(lightSpacing, -lightSpacing, 0.f, lightRadius);
	m_lights[1].color = RGBAColor(1.0, 0.25, 0.25, lightAmbient);
	m_lights[1].attenuation = lightAttenuation;
	m_lights[2].type = Light_Type_Point;
	m_lights[2].shadowCaster = true;
	m_lights[0].raySize = 32.f;
	m_lights[2].position = glm::vec4(0.f, -lightSpacing, 0.f, lightRadius);
	m_lights[2].color = RGBAColor(0.25, 1.0, 0.25, lightAmbient);
	m_lights[2].attenuation = lightAttenuation;
	m_lights[3].type = Light_Type_Point;
	m_lights[3].shadowCaster = true;
	m_lights[0].raySize = 32.f;
	m_lights[3].position = glm::vec4(0.f, -lightSpacing, lightSpacing, lightRadius);
	m_lights[3].color = RGBAColor(0.25, 0.25, 1.0, lightAmbient);
	m_lights[3].attenuation = lightAttenuation;
}
