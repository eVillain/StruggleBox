#pragma once

#include "GUIScene.h"

class Injector;
class ButtonNode;
class RenderCore;

class MainMenu : public GUIScene
{
public:
	MainMenu(Injector& injector, Allocator& allocator, RenderCore& renderCore, Input& input, OSWindow& window, Options& options, StatTracker& statTracker);
	~MainMenu();

	void Initialize() override;
	void Update(const double delta) override;
	void Draw() override;

private:
	Injector& m_injector;
	ButtonNode* createMenuButton(const std::string& title);
};
