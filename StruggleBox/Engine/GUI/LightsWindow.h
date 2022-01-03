#pragma once

#include "Light3D.h"
#include "WindowNode.h"
#include <vector>
#include <string>

class ButtonNode;
class LabelNode;
class ScrollerNode;

class LightNode : public Node
{
public:
	const LightNode(const GUI& gui);

	void setLight(LightInstance& light);

	void setContentSize(const glm::vec2& contentSize) override;

private:
	static const float ROW_HEIGHT;

	const GUI& m_gui;
	SpriteNode* m_background;

	void addPositionSliders(LightInstance& light, float& vertPos);
	void addColorSliders(LightInstance& light, float& vertPos);
	void addAttenuationSliders(LightInstance& light, float& vertPos);
};

class LightsWindow : public WindowNode
{
public:
	LightsWindow(const GUI& gui, std::vector<LightInstance>& lights);
	~LightsWindow();
	
private:
	static const glm::vec2 WINDOW_SIZE;

	std::vector<LightInstance>& m_lights;
	ScrollerNode* m_scrollerNode;

	void refreshList();
};
