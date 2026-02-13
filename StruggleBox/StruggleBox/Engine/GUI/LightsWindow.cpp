#include "LightsWindow.h"
 
#include "GUI.h"
#include "PathUtil.h"
#include "FileUtil.h"
#include "Log.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "LayoutNode.h"
#include "ScrollerNode.h"
#include "SliderNode.h"
#include "SpriteNode.h"
#include "StringUtil.h"

const float LightNode::BUTTON_HEIGHT = 24.f;
const float LightNode::LABEL_HEIGHT = 14.f;
const float LightNode::SLIDER_HEIGHT = 18.f;

LightNode::LightNode(const GUI& gui)
	: m_gui(gui)
	, m_background(gui.createSpriteNode(GUI::WINDOW_CONTENT))
	, m_layoutNode(gui.createCustomNode<LayoutNode>())
	, m_buttonLayout(gui.createCustomNode<LayoutNode>())
{
	m_background->setEnable9Slice(true);
	m_layoutNode->setLayoutType(LayoutType::Column);
	m_layoutNode->setAlignmentV(LayoutAlignmentV::Top);
	m_layoutNode->setPadding(4.f);
	m_buttonLayout->setContentSize(glm::vec2(m_contentSize.x, BUTTON_HEIGHT + 4.f));
	m_buttonLayout->setPadding(2.f);
	addChild(m_background);
	m_background->addChild(m_layoutNode);
	m_layoutNode->addChild(m_buttonLayout);
}

static const std::string getLightTypeName(LightType type)
{
	if (type == Light_Type_Directional)
	{
		return "Directional";
	}
	if (type == Light_Type_Point)
	{
		return "Point";
	}
	if (type == Light_Type_Spot)
	{
		return "Spot";
	}
	return "Invalid";
}

void LightNode::setLight(LightInstance& light)
{
	addPositionSliders(light);
	addColorSliders(light);
	addAttenuationSliders(light);

	const glm::vec2 buttonSize = glm::vec2(110.f, BUTTON_HEIGHT);
	ButtonNode* buttonSC = m_gui.createRectButton(buttonSize, "Shadows");
	buttonSC->setToggleable(true);
	buttonSC->setToggled(light.shadowCaster);
	buttonSC->setCallback([&light](bool value) {
		light.shadowCaster = value;
		});
	m_buttonLayout->addChild(buttonSC);

	ButtonNode* buttonRC = m_gui.createRectButton(buttonSize, "Rays");
	buttonRC->setToggleable(true);
	//buttonRC->setToggled(light.rayCaster);
	//buttonRC->setCallback([&light](bool value) {
	//	light.rayCaster = value;
	//	});
	m_buttonLayout->addChild(buttonRC);

	ButtonNode* buttonActive = m_gui.createRectButton(buttonSize, "Active");
	buttonActive->setToggleable(true);
	buttonActive->setToggled(light.active);
	buttonActive->setCallback([&light](bool value) {
		light.active = value;
		});
	m_buttonLayout->addChild(buttonActive);

	m_buttonLayout->refresh();
	m_layoutNode->refresh();
}

void LightNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);
	m_background->setContentSize(contentSize);
	m_layoutNode->setContentSize(contentSize);
	m_buttonLayout->setContentSize(glm::vec2(m_contentSize.x, BUTTON_HEIGHT + 4.f));
}

void LightNode::addPositionSliders(LightInstance& light)
{
	const float roomSize = 16.f;

	LabelNode* posXLabel = m_gui.createLabelNode("Position X: " + StringUtil::FloatToString(light.position.x, 6), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posXLabel);
	const glm::vec2 sliderSize = glm::vec2(m_contentSize.x - 8.f, SLIDER_HEIGHT);
	SliderNode* sliderX = m_gui.createDefaultSliderFloat();
	sliderX->setContentSize(sliderSize);
	sliderX->setFloatValues(-roomSize, roomSize, light.position.x);
	sliderX->setFloatCallback([&light, posXLabel](float value) {
		light.position.x = value;
		posXLabel->setText("Position X: " + StringUtil::FloatToString(light.position.x, 6));
		});
	m_layoutNode->addChild(sliderX);

	LabelNode* posYLabel = m_gui.createLabelNode("Position Y: " + StringUtil::FloatToString(light.position.y, 6), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posYLabel);
	SliderNode* sliderY = m_gui.createDefaultSliderFloat();
	sliderY->setContentSize(sliderSize);
	sliderY->setFloatValues(-roomSize, roomSize, light.position.y);
	sliderY->setFloatCallback([&light, posYLabel](float value) {
		light.position.y = value;
		posYLabel->setText("Position Y: " + StringUtil::FloatToString(light.position.y, 6));
		});
	m_layoutNode->addChild(sliderY);

	LabelNode* posZLabel = m_gui.createLabelNode("Position Z: " + StringUtil::FloatToString(light.position.z, 6), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posZLabel);
	SliderNode* sliderZ = m_gui.createDefaultSliderFloat();
	sliderZ->setContentSize(sliderSize);
	sliderZ->setFloatValues(-roomSize, roomSize, light.position.z);
	sliderZ->setFloatCallback([&light, posZLabel](float value) {
		light.position.z = value;
		posZLabel->setText("Position Z: " + StringUtil::FloatToString(light.position.z, 6));
		});
	m_layoutNode->addChild(sliderZ);

	LabelNode* radiusLabel = m_gui.createLabelNode("Radius: " + StringUtil::FloatToString(light.position.w, 6), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(radiusLabel);
	SliderNode* sliderRadius = m_gui.createDefaultSliderFloat();
	sliderRadius->setContentSize(sliderSize);
	sliderRadius->setFloatValues(0.f, 100.f, light.position.w);
	sliderRadius->setFloatCallback([&light, radiusLabel](float value) {
		light.position.w = value;
		radiusLabel->setText("Radius: " + StringUtil::FloatToString(light.position.w, 6));
		});
	m_layoutNode->addChild(sliderRadius);
}

void LightNode::addColorSliders(LightInstance& light)
{
	LabelNode* posXLabel = m_gui.createLabelNode("Color R: " + StringUtil::FloatToString(light.color.r), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posXLabel);
	const glm::vec2 sliderSize = glm::vec2(m_contentSize.x * 0.5f, SLIDER_HEIGHT);
	SliderNode* sliderX = m_gui.createDefaultSliderFloat();
	sliderX->setContentSize(sliderSize);
	sliderX->setFloatValues(0.f, 1.f, light.color.r);
	sliderX->setFloatCallback([&light, posXLabel](float value) {
		light.color.r = value;
		posXLabel->setText("Color R: " + StringUtil::FloatToString(light.color.r));
		});
	m_layoutNode->addChild(sliderX);

	LabelNode* posYLabel = m_gui.createLabelNode("Color G: " + StringUtil::FloatToString(light.color.g), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posYLabel);
	SliderNode* sliderY = m_gui.createDefaultSliderFloat();
	sliderY->setContentSize(sliderSize);
	sliderY->setFloatValues(0.f, 1.f, light.color.g);
	sliderY->setFloatCallback([&light, posYLabel](float value) {
		light.color.g = value;
		posYLabel->setText("Color G: " + StringUtil::FloatToString(light.color.g));
		});
	m_layoutNode->addChild(sliderY);

	LabelNode* posZLabel = m_gui.createLabelNode("Color B: " + StringUtil::FloatToString(light.color.b), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posZLabel);
	SliderNode* sliderZ = m_gui.createDefaultSliderFloat();
	sliderZ->setContentSize(sliderSize);
	sliderZ->setFloatValues(0.f, 1.f, light.color.b);
	sliderZ->setFloatCallback([&light, posZLabel](float value) {
		light.color.b = value;
		posZLabel->setText("Color B: " + StringUtil::FloatToString(light.color.b));
		});
	m_layoutNode->addChild(sliderZ);

	LabelNode* radiusLabel = m_gui.createLabelNode("Ambient: " + StringUtil::FloatToString(light.color.a), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(radiusLabel);
	SliderNode* sliderRadius = m_gui.createDefaultSliderFloat();
	sliderRadius->setContentSize(sliderSize);
	sliderRadius->setFloatValues(0.f, 1.f, light.color.a);
	sliderRadius->setFloatCallback([&light, radiusLabel](float value) {
		light.color.a = value;
		radiusLabel->setText("Ambient: " + StringUtil::FloatToString(light.color.a));
		});
	m_layoutNode->addChild(sliderRadius);
}

void LightNode::addAttenuationSliders(LightInstance& light)
{
	LabelNode* posXLabel = m_gui.createLabelNode("Constant: " + StringUtil::FloatToString(light.attenuation.x), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posXLabel);
	const glm::vec2 sliderSize = glm::vec2(m_contentSize.x * 0.5f, SLIDER_HEIGHT);
	SliderNode* sliderX = m_gui.createDefaultSliderFloat();
	sliderX->setContentSize(sliderSize);
	sliderX->setFloatValues(0.f, 1.f, light.attenuation.x);
	sliderX->setFloatCallback([&light, posXLabel](float value) {
		light.attenuation.x = value;
		posXLabel->setText("Constant: " + StringUtil::FloatToString(light.attenuation.x));
		});
	m_layoutNode->addChild(sliderX);

	LabelNode* posYLabel = m_gui.createLabelNode("Linear: " + StringUtil::FloatToString(light.attenuation.y), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posYLabel);
	SliderNode* sliderY = m_gui.createDefaultSliderFloat();
	sliderY->setContentSize(sliderSize);
	sliderY->setFloatValues(0.f, 1.f, light.attenuation.y);
	sliderY->setFloatCallback([&light, posYLabel](float value) {
		light.attenuation.y = value;
		posYLabel->setText("Linear: " + StringUtil::FloatToString(light.attenuation.y));
		});
	m_layoutNode->addChild(sliderY);

	LabelNode* posZLabel = m_gui.createLabelNode("Quadratic: " + StringUtil::FloatToString(light.attenuation.z), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(posZLabel);
	SliderNode* sliderZ = m_gui.createDefaultSliderFloat();
	sliderZ->setContentSize(sliderSize);
	sliderZ->setFloatValues(0.f, 1.f, light.attenuation.z);
	sliderZ->setFloatCallback([&light, posZLabel](float value) {
		light.attenuation.z = value;
		posZLabel->setText("Quadratic: " + StringUtil::FloatToString(light.attenuation.z));
		});
	m_layoutNode->addChild(sliderZ);

	LabelNode* radiusLabel = m_gui.createLabelNode("Type: " + getLightTypeName(light.type), GUI::FONT_DEFAULT, LABEL_HEIGHT);
	m_layoutNode->addChild(radiusLabel);
	SliderNode* sliderRadius = m_gui.createDefaultSliderInt();
	sliderRadius->setContentSize(sliderSize);
	sliderRadius->setIntValues(1, 3, light.type);
	sliderRadius->setIntCallback([&light, radiusLabel](int value) {
		light.type = (LightType)value;
		radiusLabel->setText("Type: " + getLightTypeName(light.type));
		});
	m_layoutNode->addChild(sliderRadius);
}

const glm::vec2 LightsWindow::WINDOW_SIZE = glm::vec2(400.f, 600.f);

LightsWindow::LightsWindow(
	const GUI& gui, 
	std::vector<LightInstance>& lights)
	: WindowNode(gui, WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, "Lights")
	, m_lights(lights)
	, m_scrollerNode(gui.createScrollerNode(GUI::RECT_DEFAULT))
{
	m_scrollerNode->setShowScrollBar(true);
	addChild(m_scrollerNode);

	refreshList();
}

LightsWindow::~LightsWindow()
{
}

void LightsWindow::refreshList()
{
	m_scrollerNode->setContentSize(m_contentNode->getContentSize() - glm::vec2(4.f, 4.f));
	m_scrollerNode->setPosition(glm::vec3(2.f, 2.f, 1.f));
	
	std::vector<Node*> nodes;
	for (LightInstance& light : m_lights)
	{
		LightNode* node = m_gui.createCustomNode<LightNode>(m_gui);
		node->setContentSize(glm::vec2(m_contentNode->getContentSize().x - 24.f, 500.f));
		node->setLight(light);
		nodes.push_back(node);
	}

	m_scrollerNode->setContent(nodes, ScrollStrategy::KEEP_OFFSET);
}
