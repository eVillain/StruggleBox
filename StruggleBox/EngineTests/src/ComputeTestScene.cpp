#include "ComputeTestScene.h"

#include "Renderer2D.h"
#include "RenderCore.h"
#include "FileUtil.h"
#include "SceneManager.h"

ComputeTestScene::ComputeTestScene(
	Allocator& allocator,
	Renderer2D& renderer,
	RenderCore& renderCore,
	SceneManager& sceneManager,
	Input& input,
	OSWindow& window,
	Options& options,
	StatTracker& statTracker)
	: GUIScene("ComputeTestScene", allocator, renderCore, input, window, options, statTracker)
	, m_renderer(renderer)
	, m_renderCore(renderCore)
	, m_sceneManager(sceneManager)
	, m_textureID(0)
{
}

ComputeTestScene::~ComputeTestScene()
{
	m_renderCore.removeTexture(m_textureID);
}

void ComputeTestScene::Initialize()
{
	GUIScene::Initialize();
	const std::string texturePath = FileUtil::GetPath().append("Data/GFX/Cloud256.png");
	m_textureID = m_renderCore.getTextureID(texturePath, true);
}

void ComputeTestScene::ReInitialize()
{
	GUIScene::ReInitialize();
}

void ComputeTestScene::Pause()
{
	GUIScene::Pause();
}

void ComputeTestScene::Resume()
{
	GUIScene::Resume();
}

void ComputeTestScene::Update(const double delta)
{
	GUIScene::Update(delta);
	m_renderer.update(delta);
}

void ComputeTestScene::Draw()
{
	GUIScene::Draw();

	ImpostorVertexData* impostorVerts = m_renderer.bufferImpostorPoints(1, m_textureID);
	impostorVerts[0] = { glm::vec3(100, 100, 0), 200, COLOR_WHITE };

	const glm::ivec2 resolution = m_renderCore.getRenderResolution();
	m_renderer.drawGrid(20.f, Rect2D(0, 0, resolution.x, resolution.y), 0, COLOR_PURPLE, 0.f);
	//m_renderer.drawRectColor(Rect2D(10, 10, 200, 200), COLOR_BLUE, COLOR_RED, 0);
	//m_renderer.drawCircleColor(glm::vec2(500, 500), 0.f, 200.f, COLOR_RED, COLOR_BLUE, 0, 8);
	//m_renderer.drawRingColor(glm::vec2(0, 0), 100.f, 200.f, 32, COLOR_GREEN, COLOR_PURPLE, 0);
	//m_renderer.drawRectTextured(Rect2D(300, 0, 256, 256), Rect2D(0, 0, 1, 1), m_textureID, 0.f);

	m_renderer.flush();
}

bool ComputeTestScene::OnEvent(const InputEvent event, const float amount)
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
	return false;
}
