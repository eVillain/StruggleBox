#include "EditorRoom.h"

#include "CubeConstants.h"
#include "Log.h"
#include "Random.h"
#include "Timer.h"
#include "VoxelRenderer.h"

EditorRoom::EditorRoom(VoxelRenderer& renderer)
	: m_renderer(renderer)
	, m_voxelInstancesDrawDataID(0)
	, m_voxelInstancesShaderID(0)
	, m_numCubes(0)
	, m_roomWidth(0.f)
	, m_tilesPerSide(0)
	, m_rotateLights(false)
	, m_roomCubes(nullptr)
	, m_materialData()
	, m_lights(4)
{
	m_materialData.load("C:\\Users\\rpska\\Desktop\\Gamedev\\StruggleBox\\StruggleBox\\Runtime\\Data\\Materials\\default.plist");
	Log::Info("EditorRoom constructed at %p", this);
}

EditorRoom::~EditorRoom()
{
	if (m_roomCubes)
	{
		delete[] m_roomCubes;
	}
}

void EditorRoom::draw()
{
	if (m_rotateLights)
	{
		double timeNow = Timer::Seconds();
		const float lightRotationRadius = m_roomWidth * 0.25f;
		m_lights[0].position.x = float(sin(timeNow)) * lightRotationRadius;
		m_lights[0].position.z = float(cos(timeNow)) * lightRotationRadius;
	}
	
	for (const LightInstance& light : m_lights)
	{
		m_renderer.queueLight(light);
	}
	if (m_numCubes)
	{
		CubeInstanceTransform3DData* cubes = m_renderer.bufferCubes(m_numCubes);
		memcpy(cubes, m_roomCubes, m_numCubes * sizeof(CubeInstanceTransform3DData));
	}
}

void EditorRoom::buildRoom(const float roomWidth, const int tilesPerSide)
{
	if (m_roomCubes)
	{
		delete[] m_roomCubes;
	}

	const bool randomBlockTypes = true;

	m_roomWidth = roomWidth;
	m_tilesPerSide = tilesPerSide;
	const int min = -tilesPerSide / 2;
	const int max = tilesPerSide / 2;
	const float floorTileWidth = (roomWidth / tilesPerSide);
	const float randomFactor = 0.0f;
	m_roomCubes = new CubeInstanceTransform3DData[tilesPerSide * tilesPerSide * 6];

	Random::RandomSeed((int)Timer::Seconds());
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

				uint8_t blockType = 1;
				if (randomBlockTypes)
				{
					blockType = Random::RandomInt(1, 82);
				}
				const MaterialDef& material = m_materialData[blockType];
				m_roomCubes[m_numCubes++] = {
					glm::vec3(posX, posY, posZ),
					glm::vec3(floorTileWidth, floorTileWidth, floorTileWidth),
					glm::quat(),
					material.albedo,
					glm::vec3(material.roughness, material.metalness, 0.f)
				};
			}
		}
	}

	setDefaultLights();
	//setColoredLights();
}

void EditorRoom::setDefaultLights()
{
	const float lightRadius = m_roomWidth * 0.5f;
	const float lightAmbient = 0.0f;
	const float lightSpacing = m_roomWidth * 0.25f;
	const glm::vec3 lightAttenuation = glm::vec3(0.0f, 0.75f, 0.25f);
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
	m_lights[1].raySize = 32.f;
	m_lights[1].position = glm::vec4(lightSpacing, -lightSpacing, 0.f, lightRadius);
	m_lights[1].color = RGBAColor(1.0, 0.25, 0.25, lightAmbient);
	m_lights[1].attenuation = lightAttenuation;
	m_lights[2].type = Light_Type_Point;
	m_lights[2].shadowCaster = true;
	m_lights[2].raySize = 32.f;
	m_lights[2].position = glm::vec4(0.f, -lightSpacing, 0.f, lightRadius);
	m_lights[2].color = RGBAColor(0.25, 1.0, 0.25, lightAmbient);
	m_lights[2].attenuation = lightAttenuation;
	m_lights[3].type = Light_Type_Point;
	m_lights[3].shadowCaster = true;
	m_lights[3].raySize = 32.f;
	m_lights[3].position = glm::vec4(0.f, -lightSpacing, lightSpacing, lightRadius);
	m_lights[3].color = RGBAColor(0.25, 0.25, 1.0, lightAmbient);
	m_lights[3].attenuation = lightAttenuation;
}
