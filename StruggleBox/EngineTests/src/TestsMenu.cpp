#include "TestsMenu.h"

#include "Injector.h"
#include "Allocator.h"
#include "ProxyAllocator.h"
#include "EngineCore.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "Renderer3DDeferred.h"
#include "Input.h"
#include "OSWindow.h"
#include "Options.h"
#include "StatTracker.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "ComputeTestScene.h"
#include "RenderTestScene.h"
#include "Render3DTestScene.h"
#include "RenderPBRTestScene.h"
#include "VoxelRenderer.h"
#include "VoxelTestScene.h"
#include "SceneManager.h"
#include "RenderCore.h"

TestsMenu::TestsMenu(Injector& injector, Allocator& allocator, Renderer2D& renderer, Input& input, OSWindow& window, Options& options, StatTracker& statTracker)
	: GUIScene("Engine Tests Menu", allocator, renderer.getRenderCore(), input, window, options, statTracker)
	, m_injector(injector)
{
}

TestsMenu::~TestsMenu()
{
}

void TestsMenu::Initialize()
{
	//Log::Info("[TestsMenu] initializing...");
	GUIScene::Initialize();

	int hW = m_window.GetWidth() / 2;
	int hH = m_window.GetHeight() / 2;
	float buttonPosY = hH + 100.f;
	const float buttonSpacing = 42.f;

	LabelNode* label = m_gui.createLabelNode("Engine Tests Menu", GUI::FONT_DEFAULT, 96);
	label->setPosition(glm::vec3(hW, hH + 200, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);

	ButtonNode* button2D = createMenuButton("Renderer 2D Tests");
	button2D->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	button2D->setCallback([this](bool) {
		RenderTestScene& testScene = m_injector.instantiateUnmapped<RenderTestScene, Allocator, Renderer2D, RenderCore, SceneManager, Input, OSWindow, Options, StatTracker>();
		m_injector.getInstance<SceneManager>().AddActiveScene(&testScene);
		});
	m_gui.getRoot().addChild(button2D);
	buttonPosY -= buttonSpacing;
	ButtonNode* button3D = createMenuButton("Renderer 3D Tests");
	button3D->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	button3D->setCallback([this](bool) {
		Render3DTestScene& testScene = m_injector.instantiateUnmapped<Render3DTestScene, Allocator, Renderer2D, Renderer3D, RenderCore, SceneManager, Input, OSWindow, Options, StatTracker>();
		m_injector.getInstance<SceneManager>().AddActiveScene(&testScene);
		});
	m_gui.getRoot().addChild(button3D);
	buttonPosY -= buttonSpacing;
	ButtonNode* buttonPBR = createMenuButton("Renderer PBR Tests");
	buttonPBR->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	buttonPBR->setCallback([this](bool) {
		RenderPBRTestScene& testScene = m_injector.instantiateUnmapped<RenderPBRTestScene, Allocator, Renderer2D, Renderer3DDeferred, RenderCore, Input, SceneManager, OSWindow, Options, StatTracker>();
		m_injector.getInstance<SceneManager>().AddActiveScene(&testScene);
		});
	m_gui.getRoot().addChild(buttonPBR);
	buttonPosY -= buttonSpacing;
	ButtonNode* buttonVoxel = createMenuButton("Voxel Tests");
	buttonVoxel->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	buttonVoxel->setCallback([this](bool) {
		VoxelTestScene& testScene = m_injector.instantiateUnmapped<VoxelTestScene, Allocator, VoxelRenderer, RenderCore, SceneManager, Input, OSWindow, Options, StatTracker>();
		m_injector.getInstance<SceneManager>().AddActiveScene(&testScene);
		});
	m_gui.getRoot().addChild(buttonVoxel);
	buttonPosY -= buttonSpacing;
	ButtonNode* buttonCompute = createMenuButton("Compute Tests");
	buttonCompute->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	buttonCompute->setCallback([this](bool) {
		ComputeTestScene& testScene = m_injector.instantiateUnmapped<ComputeTestScene, Allocator, Renderer2D, RenderCore, SceneManager, Input, OSWindow, Options, StatTracker>();
		m_injector.getInstance<SceneManager>().AddActiveScene(&testScene);
		});
	m_gui.getRoot().addChild(buttonCompute);
}

void TestsMenu::Update(const double delta)
{
	GUIScene::Update(delta);
}

void TestsMenu::Draw()
{
	GUIScene::Draw();
}

ButtonNode* TestsMenu::createMenuButton(const std::string& title)
{
	static const glm::vec2 BUTTON_SIZE = glm::vec2(260.f, 40.f);
	ButtonNode* button = m_gui.createDefaultButton(BUTTON_SIZE, title);
	button->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	return button;
}