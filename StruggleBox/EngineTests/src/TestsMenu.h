#pragma once

#include "GUIScene.h"

class Injector;
class ButtonNode;
class Renderer2D;

class TestsMenu : public GUIScene
{
public:
	TestsMenu(Injector& injector, Allocator& allocator, Renderer2D& renderer, Input& input, OSWindow& window, Options& options, StatTracker& statTracker);
	~TestsMenu();

	void Initialize() override;
	void Update(const double delta) override;
	void Draw() override;

private:
	Injector& m_injector;
	ButtonNode* createMenuButton(const std::string& title);

};
