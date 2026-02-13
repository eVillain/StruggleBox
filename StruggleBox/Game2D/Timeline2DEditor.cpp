#include "Timeline2DEditor.h"

#include "Allocator.h"
#include "Input.h"
#include "OSWindow.h"
#include "Options.h"
#include "StatTracker.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "TimelineNode.h"
#include "SceneManager.h"
#include "RenderCore.h"
#include "Renderer2DDeferred.h"

Timeline2DEditor::Timeline2DEditor(Allocator& allocator, RenderCore& renderCore, Input& input, OSWindow& window, Options& options, StatTracker& statTracker)
	: GUIScene("Timeline 2D Editor", allocator, renderCore, input, window, options, statTracker)
	, m_timeline(CUSTOM_NEW(TimelineNode, allocator)(m_gui, glm::vec2(m_window.GetWidth(), 200.f)))
{
}

Timeline2DEditor::~Timeline2DEditor()
{
}

void Timeline2DEditor::Initialize()
{
	//Log::Info("[Timeline2DEditor] initializing...");
	GUIScene::Initialize();

	int hW = m_window.GetWidth() / 2;
	int hH = m_window.GetHeight() / 2;
	float buttonPosY = hH + 100.f;
	const float buttonSpacing = 42.f;

	LabelNode* label = m_gui.createLabelNode("Editor", GUI::FONT_DEFAULT, 96);
	label->setPosition(glm::vec3(hW, hH + 200, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);

	m_gui.getRoot().addChild(m_timeline);
}

void Timeline2DEditor::Update(const double delta)
{
	GUIScene::Update(delta);
}

void Timeline2DEditor::Draw()
{
	GUIScene::Draw();
}

ButtonNode* Timeline2DEditor::createMenuButton(const std::string& title)
{
	static const glm::vec2 BUTTON_SIZE = glm::vec2(260.f, 40.f);
	ButtonNode* button = m_gui.createDefaultButton(BUTTON_SIZE, title);
	button->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	return button;
}