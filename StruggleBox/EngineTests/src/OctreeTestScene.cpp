#include "OctreeTestScene.h"

#include "CubeConstants.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "SceneManager.h"
#include "Options.h"
#include "Input.h"
#include "OSWindow.h"
#include "LabelNode.h"
#include "FileUtil.h"
#include "MathUtils.h"

const uint8_t OCTREE_LEVELS = 8;
const float sizesPerLevel[8] = { 8.f, 4.f, 2.f, 1.f , 0.5f, 0.25f, 0.125f, 0.0625f};
uint8_t traverseDepth = 0;

OctreeTestScene::OctreeTestScene(
	Allocator& allocator,
	Renderer2D& renderer2D,
	Renderer3D& renderer3D,
	RenderCore& renderCore,
	SceneManager& sceneManager,
	Input& input,
	OSWindow& window,
	Options& options,
	StatTracker& statTracker)
	:GUIScene("Render 3D Tests", allocator, renderer2D.getRenderCore(), input, window, options, statTracker)
	, m_renderer2D(renderer2D)
	, m_renderer3D(renderer3D)
	, m_renderCore(renderCore)
	, m_sceneManager(sceneManager)
	, m_cubeDrawDataID(0)
	, m_octree(OCTREE_LEVELS, allocator)
{
}

OctreeTestScene::~OctreeTestScene()
{
}

void OctreeTestScene::Initialize()
{
	Log::Info("[TestsMenu] initializing...");
	GUIScene::Initialize();

	int hW = m_window.GetWidth() / 2;
	int hH = m_window.GetHeight() / 2;
	float buttonPosY = hH + 100.f;

	LabelNode* label = m_gui.createLabelNode("3D Renderer Tests", GUI::FONT_DEFAULT, 48);
	label->setPosition(glm::vec3(hW, hH + 200, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);
	m_renderer3D.getDefaultCamera().setPosition(glm::vec3(8, 8, 40));

	m_cubeDrawDataID = m_renderer3D.getInstanceDrawData("InstancedColorCube");
	ColoredVertex3DData* cubeVerts = m_renderer3D.bufferInstanceColoredTriangles(36, m_cubeDrawDataID);
	for (uint32_t i = 0; i < 36; i++)
	{
		const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
		cubeVerts[i] = { v, COLOR_WHITE };
	}

	m_octree.setLeafOccupied(glm::ivec3(0, 0, 0), true);
	m_octree.setLeafOccupied(glm::ivec3(255, 255, 255), true);
}

void OctreeTestScene::Update(const double delta)
{
	float deadZone = 0.35f;
	if (fabsf(m_inputMove.x) + fabsf(m_inputMove.y) < deadZone) m_inputMove = glm::vec2();
	if (fabsf(m_inputRotate.x) + fabsf(m_inputRotate.y) < deadZone) m_inputRotate = glm::vec2();
	if (!m_renderer3D.getDefaultCamera().getThirdPerson())
	{
		glm::vec3 movement = m_renderer3D.getDefaultCamera().getMovement();
		movement.x = m_inputMove.x;
		movement.z = m_inputMove.y;
		m_renderer3D.getDefaultCamera().setMovement(movement * 5.f);
	}
	float joySensitivity = 2.0f;
	float rotationX = -m_inputRotate.x * joySensitivity;
	float rotationY = -m_inputRotate.y * joySensitivity;
	m_renderer3D.getDefaultCamera().rotate(rotationX, rotationY);

	m_renderer2D.update(delta);
	m_renderer3D.update(delta);
}

void OctreeTestScene::Draw()
{
	GUIScene::Draw();

	const float gridSize = sizesPerLevel[traverseDepth];
	m_renderer3D.drawGrid(gridSize, glm::vec3(0, 0, 0), glm::vec3(16, 16, 16), COLOR_GREEN);

	// Traverse octree in descending levels to find occupied leaves
	const uint8_t octreeRoot = m_octree.getOccupancyBitfield()[0];
	recursiveOctreeTraversal(0, octreeRoot, glm::ivec3(), glm::vec3());

	ColoredInstanceData* instance = m_renderer3D.bufferInstanceColoredData(1, m_cubeDrawDataID);
	instance[0] = { glm::vec3(), 1.f, glm::quat(), COLOR_RED};

	m_renderer3D.flush();
	m_renderer2D.flush();
}

bool OctreeTestScene::OnMouse(const glm::ivec2& coord)
{
	if (GUIScene::OnMouse(coord))
	{
		return true;
	}

	const int windowWidth = m_options.getOption<int>("r_resolutionX");
	const int windowHeight = m_options.getOption<int>("r_resolutionY");
	const int midWindowX = windowWidth / 2;     // Middle of the window horizontally
	const int midWindowY = windowHeight / 2;    // Middle of the window vertically
	if (m_options.getOption<bool>("r_grabCursor"))
	{
		float mouseSensitivity = 0.003f;
		float rotationX = (midWindowX - coord.x) * mouseSensitivity;
		float rotationY = (midWindowY - coord.y) * mouseSensitivity;

		if (m_renderer3D.getDefaultCamera().getThirdPerson())
		{
			rotationX *= -1.0f;
			rotationY *= -1.0f;
		}

		m_renderer3D.getDefaultCamera().rotate(rotationX, rotationY);

		// Reset the mouse position to the centre of the window each frame
		m_input.MoveCursor(glm::ivec2(midWindowX, midWindowY));
		//m_cursor.posScrn = glm::vec2(midWindowX, midWindowY);
		return true;
	}
	else
	{
		//m_cursor.posScrn = glm::vec2(coord.x, windowHeight - coord.y);
	}
	return false;
}

bool OctreeTestScene::OnEvent(const InputEvent event, const float amount)
{
	if (GUIScene::OnEvent(event, amount))
	{
		return true;
	}
	if (amount == -1.0f)
	{
		if (event == InputEvent::Edit_Grab_Cursor)
		{
			bool& grabCursor = m_options.getOption<bool>("r_grabCursor");
			grabCursor = !grabCursor;
			//SDL_ShowCursor(!grabCursor);
			return true;
		}
	}
	if (event == InputEvent::Move_Forward)
	{
		m_inputMove.y += amount;
	}
	else if (event == InputEvent::Move_Backward)
	{
		m_inputMove.y += -amount;
	}
	else if (event == InputEvent::Move_Left)
	{
		m_inputMove.x += -amount;
	}
	else if (event == InputEvent::Move_Right)
	{
		m_inputMove.x += amount;
	}
	else if (event == InputEvent::Back && amount < 0.f)
	{
		m_sceneManager.DropActiveScene();
	}
	else if (event == InputEvent::Look_Up && amount < 0.f)
	{
		traverseDepth = MathUtils::Clamp(traverseDepth + 1, 0, OCTREE_LEVELS - 1);
	}
	else if (event == InputEvent::Look_Down && amount < 0.f)
	{
		traverseDepth = MathUtils::Clamp(traverseDepth - 1, 0, OCTREE_LEVELS - 1);
	}
	return false;
}


void OctreeTestScene::recursiveOctreeTraversal(const uint8_t level, const uint8_t bits, const glm::ivec3& localCoord, const glm::vec3& topLevelPos)
{
	if (bits == 0)
		return;
	for (uint32_t z = 0; z < 2; z++)
	{
		for (uint32_t y = 0; y < 2; y++)
		{
			for (uint32_t x = 0; x < 2; x++)
			{
				const uint8_t bit = x + (y * 2) + (z * 4);
				const bool occupied = bits & (1 << bit);
				if (!occupied)
				{
					continue;
				}
				const float cubeSize = sizesPerLevel[level];
				const glm::vec3 pos = topLevelPos + glm::vec3(x, y, z) * cubeSize;
				if (level < traverseDepth)
				{
					// If we are not at the lowest level, check next level 
					const glm::ivec3 octantCoord = glm::ivec3(x, y, z);
					const glm::ivec3 nextLocalCoord = (localCoord * 2) + octantCoord;
					const uint32_t octantOffset = m_octree.getOctantOffset(nextLocalCoord, level + 1);
					const uint8_t octantBits = m_octree.getOccupancyBitfield()[octantOffset];
					if (octantBits == 0)
					{
						Log::Debug("level: %i, bits: %i, local coord: %i, %i, %i", level, bits, localCoord.x, localCoord.y, localCoord.y);
						Log::Debug("octant: %i, %i, %i, next octant offset: %i, octant bits: %i", x, y, z, octantOffset, octantBits);
						Log::Debug("WTF octree data if broken here!");
					}
					recursiveOctreeTraversal(level + 1, octantBits, nextLocalCoord, pos);
				}
				else
				{
					// Otherwise this is a leaf, draw a cube
					ColoredInstanceData* instance = m_renderer3D.bufferInstanceColoredData(1, m_cubeDrawDataID);
					instance[0] = { pos + glm::vec3(cubeSize / 2.f, cubeSize / 2.f, cubeSize / 2.f), sizesPerLevel[level], glm::quat(), COLOR_WHITE };
				}
			}
		}
	}
}
