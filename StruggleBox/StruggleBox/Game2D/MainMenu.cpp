#include "MainMenu.h"

#include "Injector.h"
#include "Allocator.h"
#include "Renderer2DDeferred.h"
#include "Input.h"
#include "OSWindow.h"
#include "Options.h"
#include "StatTracker.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "SceneManager.h"
#include "RenderCore.h"
#include "Game2D.h"
#include "Timeline2DEditor.h"

MainMenu::MainMenu(Injector& injector, Allocator& allocator, RenderCore& renderCore, Input& input, OSWindow& window, Options& options, StatTracker& statTracker)
	: GUIScene("Main Menu", allocator, renderCore, input, window, options, statTracker)
	, m_injector(injector)
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::Initialize()
{
	//Log::Info("[MainMenu] initializing...");
	GUIScene::Initialize();

	int hW = m_window.GetWidth() / 2;
	int hH = m_window.GetHeight() / 2;
	float buttonPosY = hH + 100.f;
	const float buttonSpacing = 42.f;

	LabelNode* label = m_gui.createLabelNode("Main Menu", GUI::FONT_DEFAULT, 96);
	label->setPosition(glm::vec3(hW, hH + 200, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);

	ButtonNode* buttonGame = createMenuButton("Start Game");
	buttonGame->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	buttonGame->setCallback([this](bool) {
		Game2D& game2D = m_injector.instantiateUnmapped<Game2D, Allocator, Renderer2DDeferred, Input, OSWindow, Options, StatTracker>();
		m_injector.getInstance<SceneManager>().AddActiveScene(&game2D);
		});
	m_gui.getRoot().addChild(buttonGame);
	buttonPosY -= buttonSpacing;

	ButtonNode* button2D = createMenuButton("Timeline 2D Editor");
	button2D->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	button2D->setCallback([this](bool) {
		Timeline2DEditor& editor = m_injector.instantiateUnmapped<Timeline2DEditor, Allocator, RenderCore, Input, OSWindow, Options, StatTracker>();
		m_injector.getInstance<SceneManager>().AddActiveScene(&editor);
		});
	m_gui.getRoot().addChild(button2D);
	buttonPosY -= buttonSpacing;
}

void MainMenu::Update(const double delta)
{
	GUIScene::Update(delta);
}

void MainMenu::Draw()
{
	GUIScene::Draw();
}

ButtonNode* MainMenu::createMenuButton(const std::string& title)
{
	static const glm::vec2 BUTTON_SIZE = glm::vec2(260.f, 40.f);
	ButtonNode* button = m_gui.createDefaultButton(BUTTON_SIZE, title);
	button->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	return button;
}