#include "ValueEditNode.h"

#include "Options.h"
#include "PathUtil.h"
#include "StringUtil.h"
#include "Log.h"

#include "GUI.h"
#include "WindowNode.h"
#include "LabelNode.h"
#include "SpriteNode.h"
#include "ButtonNode.h"
#include "SliderNode.h"
#include "TextInputNode.h"
#include "ScrollerNode.h"

BaseValueEditNode::BaseValueEditNode(const GUI& gui)
	: m_gui(gui)
	, m_background(gui.createSpriteNode(GUI::RECT_DEFAULT))
	, m_label(gui.createLabelNode("", GUI::FONT_DEFAULT, 12))
{
	m_background->setEnable9Slice(true);
	m_label->setAnchorPoint(glm::vec2(0.f, 0.5f));

	addChild(m_background);
	addChild(m_label);
}

void BaseValueEditNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);
	m_background->setContentSize(contentSize);
	m_label->setPosition(glm::vec3(4.f, contentSize.y * 0.5f, 1.f));
}


template <typename T>
void ValueEditNode<T>::setValue(const std::string& name, T& value, T min, T max)
{
	//m_label->setText(name + ": " + attribute->GetValueString());
	const auto& children = m_background->getChildren();
	m_gui.destroyNodes(children);
	m_background->removeAllChildren();
}

template<>
void ValueEditNode<bool>::setValue(const std::string& name, bool& value, bool, bool)
{
	m_label->setText(name + ": " + StringUtil::BoolToString(value));

	const glm::vec2 buttonSize = glm::vec2(m_contentSize.y - 4.f, m_contentSize.y - 4.f);
	ButtonNode* button = m_gui.createRectButton(buttonSize);
	button->setPosition(glm::vec3(m_contentSize.x - (buttonSize.x + 4.f), 2.f, 2.f));
	button->setToggleable(true);
	button->setToggled(value);
	button->setCallback([this, name, &value](bool buttonValue) {
		value = buttonValue;
		m_label->setText(name + ": " + StringUtil::BoolToString(value));
		});
	m_background->addChild(button);
}

template<>
void ValueEditNode<int>::setValue(const std::string& name, int& value, int min, int max)
{
	m_label->setText(name + ": " + StringUtil::IntToString(value));

	const glm::vec2 sliderSize = glm::vec2(m_contentSize.x * 0.5f, m_contentSize.y - 4.f);
	SliderNode* slider = m_gui.createDefaultSliderInt();
	slider->setContentSize(sliderSize);
	slider->setPosition(glm::vec3(m_contentSize.x - (sliderSize.x + 4.f), 2.f, 2.f));
	slider->setIntValues(min, max, value);
	slider->setIntCallback([this, name, &value](int sliderValue) {
		value = sliderValue;
		m_label->setText(name + ": " + StringUtil::IntToString(value));
		});
	m_background->addChild(slider);
}

template<>
void ValueEditNode<float>::setValue(const std::string& name, float& value, float min, float max)
{
	m_label->setText(name + ": " + StringUtil::FloatToString(value));

	const glm::vec2 sliderSize = glm::vec2(m_contentSize.x * 0.5f, m_contentSize.y - 4.f);
	SliderNode* slider = m_gui.createDefaultSliderFloat();
	slider->setContentSize(sliderSize);
	slider->setPosition(glm::vec3(m_contentSize.x - (sliderSize.x + 4.f), 2.f, 2.f));
	slider->setFloatValues(min, max, value);
	slider->setFloatCallback([this, name, &value](float sliderValue) {
		value = sliderValue;
		m_label->setText(name + ": " + StringUtil::FloatToString(value));
		});
	m_background->addChild(slider);
}

template<>
void ValueEditNode<std::string>::setValue(const std::string& name, std::string& value, std::string, std::string)
{
	m_label->setText(name + ": " + value);

	const glm::vec2 inputSize = glm::vec2(m_contentSize.x * 0.5f, m_contentSize.y - 4.f);
	TextInputNode* input = m_gui.createDefaultTextInput(value);
	input->setContentSize(inputSize);
	input->setPosition(glm::vec3(m_contentSize.x - (inputSize.x + 4.f), 2.f, 2.f));
	input->setFinishCallback([this, name, &value](const std::string& inputValue, bool) {
		value = inputValue;
		m_label->setText(name + ": " + value);
		});
	m_background->addChild(input);
}

template<>
void ValueEditNode<glm::vec3>::setValue(const std::string& name, glm::vec3& value, glm::vec3 min, glm::vec3 max)
{
	m_label->setText(name + " - X: " + StringUtil::FloatToString(value.x ) + " Y: " + StringUtil::FloatToString(value.y) + " Z: " + StringUtil::FloatToString(value.z));
	const glm::vec2 sliderSize = glm::vec2(m_contentSize.x - 4.f, (m_contentSize.y - 4.f) / 3.f);
	SliderNode* sliderX = m_gui.createDefaultSliderFloat();
	sliderX->setContentSize(sliderSize);
	sliderX->setPosition(glm::vec3(m_contentSize.x - (sliderSize.x + 4.f), (m_contentSize.y - 2.f) - sliderSize.y, 1.f));
	sliderX->setFloatValues(min.x, max.x, value.x);
	sliderX->setFloatCallback([this, name, &value](float sliderValue) {
		value.x = sliderValue;
		m_label->setText(name + " - X: " + StringUtil::FloatToString(value.x) + " Y: " + StringUtil::FloatToString(value.y) + " Z: " + StringUtil::FloatToString(value.z));
		});
	m_background->addChild(sliderX);
	SliderNode* sliderY = m_gui.createDefaultSliderFloat();
	sliderY->setContentSize(sliderSize);
	sliderY->setPosition(glm::vec3(m_contentSize.x - (sliderSize.x + 4.f), (m_contentSize.y - 2.f) - (sliderSize.y * 2.f), 1.f));
	sliderY->setFloatValues(min.y, max.y, value.y);
	sliderY->setFloatCallback([this, name, &value](float sliderValue) {
		value.y = sliderValue;
		m_label->setText(name + " - X: " + StringUtil::FloatToString(value.x) + " Y: " + StringUtil::FloatToString(value.y) + " Z: " + StringUtil::FloatToString(value.z));
		});
	m_background->addChild(sliderY);
	SliderNode* sliderZ = m_gui.createDefaultSliderFloat();
	sliderZ->setContentSize(sliderSize);
	sliderZ->setPosition(glm::vec3(m_contentSize.x - (sliderSize.x + 4.f), (m_contentSize.y - 2.f) - (sliderSize.y * 3.f), 1.f));
	sliderZ->setFloatValues(min.z, max.z, value.z);
	sliderZ->setFloatCallback([this, name, &value](float sliderValue) {
		value.z = sliderValue;
		m_label->setText(name + " - X: " + StringUtil::FloatToString(value.x) + " Y: " + StringUtil::FloatToString(value.y) + " Z: " + StringUtil::FloatToString(value.z));
		});
	m_background->addChild(sliderZ);
}