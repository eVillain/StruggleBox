#pragma once

#include "Scene.h"
#include "InputListener.h"
#include "GUI.h"
#include <memory>

class Allocator;
class Renderer;
class Input;
class Options;
class StatTracker;
class ConsoleDisplay;
class StatsView;

class GUIScene : public Scene, public InputEventListener, public MouseEventListener
{
public:
	GUIScene(const std::string sceneID, Allocator& allocator, Renderer& renderer, Input& input, Options& options, StatTracker& statTracker);
	~GUIScene();

	void Initialize() override;
	void ReInitialize() override;
	void Pause() override;
	void Resume() override;
	void Update(const double delta) override;
	void Draw() override;

protected:
	Allocator& m_allocator;
	Renderer& m_renderer;
	Input& m_input;
	Options& m_options;
	StatTracker& m_statTracker;
	GUI m_gui;

	bool OnEvent(const InputEvent event, const float amount) override;
	bool OnMouse(const glm::ivec2& coords) override;

private:
	ConsoleDisplay* m_consoleDisplay;
	StatsView* m_statsView;

	void showConsole();
	void hideConsole();
	void showStats();
	void hideStats();
};
