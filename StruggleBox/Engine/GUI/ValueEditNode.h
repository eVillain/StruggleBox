#pragma once

#include "Node.h"
#include <string>
#include <map>

class Attribute;
class GUI;
class Options;
class ButtonNode;
class SpriteNode;
class LabelNode;
class WindowNode;

class BaseValueEditNode : public Node
{
public:
	BaseValueEditNode(const GUI& gui);

	void setContentSize(const glm::vec2& contentSize) override;

protected:
	const GUI& m_gui;
	SpriteNode* m_background;
	LabelNode* m_label;
};

template <typename T>
class ValueEditNode : public BaseValueEditNode
{
public:
	ValueEditNode(const GUI& gui) 
		: BaseValueEditNode(gui)
	{}

	void setValue(const std::string& name, T& value, T min, T max);
};

template<> void ValueEditNode<bool>::setValue(const std::string& name, bool& value, bool min, bool max);
template<> void ValueEditNode<int>::setValue(const std::string& name, int& value, int min, int max);
template<> void ValueEditNode<float>::setValue(const std::string& name, float& value, float min, float max);
template<> void ValueEditNode<std::string>::setValue(const std::string& name, std::string& value, std::string min, std::string max);
template<> void ValueEditNode<glm::vec3>::setValue(const std::string& name, glm::vec3& value, glm::vec3 min, glm::vec3 max);