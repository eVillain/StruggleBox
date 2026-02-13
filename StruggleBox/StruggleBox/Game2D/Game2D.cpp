#include "Game2D.h"

#include "LabelNode.h"
#include "OSWindow.h"
#include "Renderer2D.h"
#include "Timer.h"
#include <chipmunk/chipmunk.h>

Game2D::Game2D(Allocator& allocator, Renderer2DDeferred& renderer, Input& input, OSWindow& window, Options& options, StatTracker& statTracker)
	: GUIScene("Game2D", allocator, renderer.getRenderCore(), input, window, options, statTracker)
	, m_renderer(renderer)
	, m_physics()
	, m_player(m_physics)
{
}

Game2D::~Game2D()
{
}

void Game2D::Initialize()
{
	GUIScene::Initialize();

	int hW = m_window.GetWidth() / 2;
	int hH = m_window.GetHeight() / 2;
	float buttonPosY = hH + 100.f;
	const float buttonSpacing = 42.f;

	LabelNode* label = m_gui.createLabelNode("Game test", GUI::FONT_DEFAULT, 24);
	label->setPosition(glm::vec3(hW, 24, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);

	m_physics.initialize();
	m_player.initialize();
	Shape2DCircle* playerShape = CUSTOM_NEW(Shape2DCircle, m_allocator)(25.f);
	playerShape->setColor(COLOR_PLAYER);
	playerShape->setOutlineColor(COLOR_RED);
	playerShape->setDepth(1.f);
	m_foregroundShapes.push_back(playerShape);

	m_testLightID = m_renderer.getLightSystem().addLight();
	Light2D* light = m_renderer.getLightSystem().getLightForID(m_testLightID);
	light->position = glm::vec2(300, 300);
	light->lightWidth = 600.f;
	light->lightHeight = 600.f;

	Light2DID light2ID = m_renderer.getLightSystem().addLight();
	Light2D* light2 = m_renderer.getLightSystem().getLightForID(light2ID);
	light2->position = glm::vec2(750, 500);
	light2->lightWidth = 900.f;
	light2->lightHeight = 900.f;
	light2->lightColor = COLOR_YELLOW;
}

void Game2D::ReInitialize()
{
	GUIScene::ReInitialize();
}

void Game2D::Release()
{
	GUIScene::Release();
	m_physics.terminate();
	m_gui.terminate();
	for (Shape2D* shape : m_foregroundShapes)
	{
		CUSTOM_DELETE(shape, m_allocator);
	}
}

void Game2D::Pause()
{
	GUIScene::Pause();
}

void Game2D::Resume()
{
	GUIScene::Resume();
}

void Game2D::Update(const double delta)
{
	const double time = Timer::Seconds();
	Light2D* light = m_renderer.getLightSystem().getLightForID(m_testLightID);
	//light->position = glm::vec2(250 + 250 * sinf(time), 500);

	const glm::vec2 aimPos = m_renderer.getDefaultCamera().screenToWorld(m_inputAimBuffer);
	m_player.setInputDirection(m_inputDirectionBuffer);
	m_player.setAimPosition(aimPos);
	m_player.update(delta);
	m_physics.update(delta);

	m_foregroundShapes[0]->setPosition(m_player.getPosition());
	m_foregroundShapes[0]->setRotation(m_player.getAngle());

	
	m_renderer.getDefaultCamera().setTargetPosition(m_player.getPosition());
	m_renderer.update(delta);
	GUIScene::Update(delta);
}

void Game2D::Draw()
{
	m_renderer.drawGrid(40.f, Rect2D(0, 0, 1920, 1080), 0, COLOR_GREY, 0);
	
	m_renderer.flush(m_foregroundShapes);

	GUIScene::Draw();
}

bool Game2D::OnEvent(const InputEvent event, const float amount)
{
	if (GUIScene::OnEvent(event, amount))
	{
		return true;
	}

	if (event == InputEvent::Move_Forward)
	{
		m_inputDirectionBuffer.y += amount;
	}
	else if (event == InputEvent::Move_Backward)
	{
		m_inputDirectionBuffer.y -= amount;
	}
	else if (event == InputEvent::Move_Left)
	{
		m_inputDirectionBuffer.x -= amount;
	}
	else if (event == InputEvent::Move_Right)
	{
		m_inputDirectionBuffer.x += amount;
	}
	else if (event == InputEvent::Scroll_Y)
	{
		const float zoom = fmaxf(fminf(m_renderer.getDefaultCamera().getZoom() + amount, 10.f), 0.1f);
		m_renderer.getDefaultCamera().setZoom(zoom);
	}
	return false;
}

bool Game2D::OnMouse(const glm::ivec2& coord)
{
	if (GUIScene::OnMouse(coord))
	{
		return true;
	}
	m_inputAimBuffer = coord;

	return false;
}
