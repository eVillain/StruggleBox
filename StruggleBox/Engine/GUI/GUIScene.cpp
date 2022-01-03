#include "GUIScene.h"

#include "Allocator.h"
#include "Renderer.h"
#include "Input.h"
#include "Options.h"
#include "ConsoleDisplay.h"
#include "StatsView.h"
#include "FileUtil.h"
#include "Log.h"
#include "CommandProcessor.h"

GUIScene::GUIScene(const std::string sceneID, Allocator& allocator, Renderer& renderer, Input& input, Options& options, StatTracker& statTracker)
	: Scene(sceneID)
	, m_allocator(allocator)
	, m_renderer(renderer)
	, m_input(input)
	, m_options(options)
	, m_statTracker(statTracker)
	, m_gui(allocator, renderer, input)
	, m_consoleDisplay(nullptr)
	, m_statsView(nullptr)
{
}

GUIScene::~GUIScene()
{
}

void GUIScene::Initialize()
{
	Scene::Initialize();

	m_gui.initialize();
	m_input.RegisterEventObserver(this);
	m_input.RegisterMouseObserver(this);
}

void GUIScene::ReInitialize()
{
	// nothing to do, move to header?
}

void GUIScene::Pause()
{
	if (IsPaused())
	{
		return;
	}

	m_gui.terminate();
	m_input.UnRegisterEventObserver(this);
	m_input.UnRegisterMouseObserver(this);

	Scene::Pause();
}

void GUIScene::Resume()
{
	if (!IsPaused())
	{
		return;
	}

	m_gui.initialize();
	m_input.RegisterEventObserver(this);
	m_input.RegisterMouseObserver(this);

	Scene::Resume();
}

void GUIScene::Update(const double delta)
{
	if (m_statsView)
	{
		m_statsView->refresh();
	}
}

void GUIScene::Draw()
{
	m_gui.draw();
}

bool GUIScene::OnEvent(const InputEvent event, const float amount)
{
	if (m_gui.OnEvent(event, amount))
	{
		return true;
	}

	if (amount != -1) return false; // We only care for key/button release events here

	if (event == InputEvent::Console)
	{
		if (!m_consoleDisplay)
		{
			showConsole();
		}
		else
		{
			hideConsole();
		}
		return true;
	}
	else if (event == InputEvent::Stats)
	{
		if (!m_statsView)
		{
			showStats();
		}
		else
		{
			hideStats();
		}
		return true;
	}
	else if (event == InputEvent::Back)
	{
        if (m_consoleDisplay)
        {
			hideConsole();
			return true;
		}
        else
        {
			bool& grabCursor = m_options.getOption<bool>("r_grabCursor");
			if (grabCursor)
			{
				grabCursor = false; // Bring the cursor back in case it was hidden
				SDL_ShowCursor(true);
				return true;
			}
		}
	}

	return false;
}

bool GUIScene::OnMouse(const glm::ivec2& coords)
{
	if (m_gui.OnMouse(coords))
	{
		return true;
	}
	return false;
}

void GUIScene::showConsole()
{
	if (m_consoleDisplay)
	{
		return;
	}
	m_consoleDisplay = m_gui.createCustomNode<ConsoleDisplay>(m_gui, m_options);
	m_consoleDisplay->setPositionZ(10.f);
	m_gui.getRoot().addChild(m_consoleDisplay);
	CommandProcessor::AddCommand("console", Command<>([this]() { hideConsole(); }));
}

void GUIScene::hideConsole()
{
	if (!m_consoleDisplay)
	{
		return;
	}
	m_consoleDisplay->clear();
	m_gui.getRoot().removeChild(m_consoleDisplay);
	m_gui.destroyNodeAndChildren(m_consoleDisplay);
	m_consoleDisplay = nullptr;
	CommandProcessor::RemoveCommand("console");
}

void GUIScene::showStats()
{
	if (m_statsView)
	{
		return;
	}
	const int windowWidth = m_options.getOption<int>("r_resolutionX");
	const int windowHeight = m_options.getOption<int>("r_resolutionY");
	m_statsView = m_gui.createCustomNode<StatsView>(m_gui, m_statTracker);
	m_statsView->setContentSize(glm::vec2(windowWidth, windowHeight));
	m_statsView->setPositionZ(10.f);
	m_gui.getRoot().addChild(m_statsView);
}

void GUIScene::hideStats()
{
	if (!m_statsView)
	{
		return;
	}
	m_gui.getRoot().removeChild(m_statsView);
	m_gui.destroyNodeAndChildren(m_statsView);
	m_statsView = nullptr;
}