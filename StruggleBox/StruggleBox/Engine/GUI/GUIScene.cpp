#include "GUIScene.h"

#include "Allocator.h"
#include "RenderCore.h"
#include "Input.h"
#include "Options.h"
#include "ConsoleDisplay.h"
#include "StatsDisplay.h"
#include "FileUtil.h"
#include "Log.h"
#include "CommandProcessor.h"

GUIScene::GUIScene(const std::string sceneID, Allocator& allocator, RenderCore& renderCore, Input& input, OSWindow& window, Options& options, StatTracker& statTracker)
	: Scene(sceneID)
	, m_allocator(allocator)
	, m_input(input)
	, m_window(window)
	, m_options(options)
	, m_statTracker(statTracker)
	, m_gui(allocator, renderCore, input, window)
	, m_consoleDisplay(nullptr)
	, m_statsDisplay(nullptr)
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

void GUIScene::Release()
{
	m_gui.terminate();
	//m_input.UnRegisterEventObserver(this);
	//m_input.UnRegisterMouseObserver(this);
}

void GUIScene::Pause()
{
	if (IsPaused())
	{
		return;
	}

	//m_gui.terminate();
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

	//m_gui.initialize();
	m_input.RegisterEventObserver(this);
	m_input.RegisterMouseObserver(this);

	Scene::Resume();
}

void GUIScene::Update(const double delta)
{
	if (m_statsDisplay)
	{
		m_statsDisplay->update();
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
		if (!m_statsDisplay)
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
				SDL_ShowCursor();
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
	if (m_statsDisplay)
	{
		return;
	}
	m_statsDisplay = m_gui.createCustomNode<StatsDisplay>(m_gui, m_statTracker, m_options);
	m_statsDisplay->setPositionZ(10.f);
	m_gui.getRoot().addChild(m_statsDisplay);
}

void GUIScene::hideStats()
{
	if (!m_statsDisplay)
	{
		return;
	}
	m_gui.getRoot().removeChild(m_statsDisplay);
	m_gui.destroyNodeAndChildren(m_statsDisplay);
	m_statsDisplay = nullptr;
}