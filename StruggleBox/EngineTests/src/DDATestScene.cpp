#include "DDATestScene.h"

#include "Renderer2D.h"
#include "RenderCore.h"
#include "FileUtil.h"
#include "SceneManager.h"

const float SCALE = 40.f;

DDATestScene::DDATestScene(
	Allocator& allocator,
	Renderer2D& renderer,
	RenderCore& renderCore,
	SceneManager& sceneManager,
	Input& input,
	OSWindow& window,
	Options& options,
	StatTracker& statTracker)
	: GUIScene("DDATestScene", allocator, renderCore, input, window, options, statTracker)
	, m_renderer(renderer)
	, m_renderCore(renderCore)
	, m_sceneManager(sceneManager)
	, m_textureID(0)
	, m_time(0.0)
	, m_dragStart(false)
	, m_dragEnd(false)
{
}

DDATestScene::~DDATestScene()
{
	m_renderCore.removeTexture(m_textureID);
}

void DDATestScene::Initialize()
{
	GUIScene::Initialize();
	const std::string texturePath = FileUtil::GetPath().append("Data/GFX/Cloud256.png");
	m_textureID = m_renderCore.getTextureID(texturePath, true);

	m_startPoint = glm::vec2(3.5f, 1.5f);
	m_endPoint = m_startPoint + glm::vec2(5.f, 9.f);

	m_renderer.getDefaultCamera().setPosition(m_renderCore.getRenderResolution() / 2);
	m_renderer.getDefaultCamera().setTargetPosition(m_renderCore.getRenderResolution() / 2);
}

void DDATestScene::ReInitialize()
{
	GUIScene::ReInitialize();
}

void DDATestScene::Pause()
{
	GUIScene::Pause();
}

void DDATestScene::Resume()
{
	GUIScene::Resume();
}

void DDATestScene::Update(const double delta)
{
	GUIScene::Update(delta);
	m_renderer.update(delta);

	m_time += delta * 0.2;
	//m_endPoint = m_startPoint + glm::vec2(cosf(m_time), sinf(m_time)) * 10.f;

	raycast();
}

void DDATestScene::Draw()
{
	GUIScene::Draw();


	const glm::ivec2 resolution = m_renderCore.getRenderResolution();
	const glm::vec2 OFFSET = glm::vec2(SCALE / 2.f, SCALE / 2.f);
	m_renderer.drawGrid(SCALE, Rect2D(0, 0, resolution.x, resolution.y), 0, COLOR_GREY, 0.f);

	m_renderer.drawRingColor(m_startPoint * SCALE, 12.f, 16.f, 8, COLOR_WHITE, COLOR_GREEN, 0.f);
	m_renderer.drawRingColor(m_endPoint * SCALE, 12.f, 16.f, 8, COLOR_WHITE, COLOR_RED, 0.f);
	m_renderer.drawLine(m_startPoint * SCALE, m_endPoint * SCALE, COLOR_BLACK, 1.f);

	for (const glm::ivec2& cell : m_tracedCells)
	{
		Rect2D rect = Rect2D(OFFSET + glm::vec2(cell) * SCALE, glm::vec2(SCALE, SCALE));
		m_renderer.drawRectColor(rect, COLOR_NONE, RGBAColor(1.f, 1.f, 1.f, 0.75f), 0.f);
	}

	for (const glm::vec2& point : m_edgePoints)
	{
		m_renderer.drawCircleColor(point * SCALE, 0.f, 6.f, COLOR_GREEN, RGBAColor(0,0,1,0.5f), 1.f, 8);
	}
	m_renderer.flush();
}

bool DDATestScene::OnEvent(const InputEvent event, const float amount)
{
	if (GUIScene::OnEvent(event, amount))
	{
		return true;
	}
	if (event == InputEvent::Back && amount < 0.f)
	{
		m_sceneManager.DropActiveScene();
		return true;
	}
	if (event == InputEvent::Shoot)
	{
		float distToStart = glm::length(m_mouseCoord - m_startPoint);
		float distToEnd = glm::length(m_mouseCoord - m_endPoint);
		if (amount > 0.f)
		{
			if (distToStart < 20.f && distToStart < distToEnd)
			{
				m_dragStart = true;
			}
			else if (distToEnd < 20.f)
			{
				m_dragEnd = true;
			}
		}
		else
		{
			m_dragStart = false;
			m_dragEnd = false;
		}
	}
	return false;
}

bool DDATestScene::OnMouse(const glm::ivec2& coord)
{
	m_mouseCoord = m_renderer.getDefaultCamera().screenToWorld(coord) / SCALE;
	
	if (m_dragStart)
	{
		m_startPoint = m_mouseCoord;
	}
	else if (m_dragEnd)
	{
		m_endPoint = m_mouseCoord;
	}

	return false;
}

void DDATestScene::raycast()
{
	m_edgePoints.clear();
	m_tracedCells.clear();

	const float posX = m_startPoint.x;
	const float posY = m_startPoint.y;
	const glm::vec2 dir = glm::normalize(m_endPoint - m_startPoint);

	const int endX = std::floor(m_endPoint.x);
	const int endY = std::floor(m_endPoint.y);

	//calculate ray position and direction
	const double rayDirX = dir.x;
	const double rayDirY = dir.y;

	//which box of the map we're in
	int mapX = int(posX);
	int mapY = int(posY);
	//length of ray from current position to next x or y-side
	double sideDistX;
	double sideDistY;

	//length of ray from one x or y-side to next x or y-side
	const double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
	const double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);

	//what direction to step in x or y-direction (either +1 or -1)
	int stepX;
	int stepY;

	int hit = 0; //was there a wall hit?
	int side; //was a NS or a EW wall hit?

	//calculate step and initial sideDist
	if (rayDirX < 0)
	{
		stepX = -1;
		sideDistX = (posX - mapX) * deltaDistX;
	}
	else
	{
		stepX = 1;
		sideDistX = (mapX + 1.0 - posX) * deltaDistX;
	}
	if (rayDirY < 0)
	{
		stepY = -1;
		sideDistY = (posY - mapY) * deltaDistY;
	}
	else
	{
		stepY = 1;
		sideDistY = (mapY + 1.0 - posY) * deltaDistY;
	}

	m_tracedCells.push_back(glm::ivec2(mapX, mapY));
	//perform DDA
	//while (hit == 0)
	for (int i = 0; i < 64; i++)
	{
		//jump to next map square, either in x-direction, or in y-direction
		if (sideDistX < sideDistY)
		{
			m_edgePoints.push_back(glm::vec2(posX, posY) + dir * (float)sideDistX);
			sideDistX += deltaDistX;
			mapX += stepX;
			side = 0;
		}
		else
		{
			m_edgePoints.push_back(glm::vec2(posX, posY) + dir * (float)sideDistY);
			sideDistY += deltaDistY;
			mapY += stepY;
			side = 1;
		}
		m_tracedCells.push_back(glm::ivec2(mapX, mapY));
		//Check if ray has hit a wall
		if (mapX == endX && mapY == endY)
		{
			hit = 1;
			break;
		}
	}
}
