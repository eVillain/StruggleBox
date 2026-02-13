#pragma once

#include "Lighting3DDeferred.h"
#include "WindowNode.h"
#include <vector>
#include <string>

class ButtonNode;
class LabelNode;
class LayoutNode;
class ScrollerNode;

class LightNode : public Node
{
public:
	LightNode(const GUI& gui);

	void setLight(LightInstance& light);

	void setContentSize(const glm::vec2& contentSize) override;

private:
	static const float BUTTON_HEIGHT;
	static const float LABEL_HEIGHT;
	static const float SLIDER_HEIGHT;

	const GUI& m_gui;
	SpriteNode* m_background;
	LayoutNode* m_layoutNode;
	LayoutNode* m_buttonLayout;

	void addPositionSliders(LightInstance& light);
	void addColorSliders(LightInstance& light);
	void addAttenuationSliders(LightInstance& light);
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
