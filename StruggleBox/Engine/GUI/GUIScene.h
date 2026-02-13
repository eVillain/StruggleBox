#pragma once

#include "Scene.h"
#include "InputListener.h"
#include "GUI.h"
#include <memory>

class Allocator;
class ConsoleDisplay;
class Input;
class OSWindow;
class Options;
class RenderCore;
class StatsDisplay;
class StatTracker;

class GUIScene : public Scene, public InputEventListener, public MouseEventListener
{
public:
	GUIScene(
		const std::string sceneID,
		Allocator& allocator,
		RenderCore& renderCore,
		Input& input,
		OSWindow& window,
		Options& options,
		StatTracker& statTracker);
	~GUIScene();

	void Initialize() override;
	void ReInitialize() override;
	void Release() override;

	void Pause() override;
	void Resume() override;

	void Update(const double delta) override;
	void Draw() override;

protected:
	Allocator& m_allocator;
	Input& m_input;
	OSWindow& m_window;
	Options& m_options;
	StatTracker& m_statTracker;
	GUI m_gui;

	bool OnEvent(const InputEvent event, const float amount) override;
	bool OnMouse(const glm::ivec2& coords) override;

private:
	ConsoleDisplay* m_consoleDisplay;
	StatsDisplay* m_statsDisplay;

	void showConsole();
	void hideConsole();
	void showStats();
	void hideStats();
};
