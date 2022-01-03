#pragma once

#include "Node.h"
#include <string>

class LabelNode;
class SpriteNode;
class GUI;

class WindowNode : public Node
{
public:
	static const uint32_t HEADER_HEIGHT;

	WindowNode(
		const GUI& gui,
		const glm::vec2 size,
		const std::string& headerFrame,
		const std::string& backgroundFrame,
		const std::string& title);

	void setContentSize(const glm::vec2& contentSize) override;

protected:
	const GUI& m_gui;
	SpriteNode* m_headerSprite;
	LabelNode* m_titleLabel;
	SpriteNode* m_backgroundSprite;
	Node* m_contentNode;
};

