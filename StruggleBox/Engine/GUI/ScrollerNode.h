#pragma once

#include "Node.h"
#include <vector>
#include <string>

class SpriteNode;
class GUI;

enum class ScrollStrategy {
	KEEP_OFFSET,
	SCROLL_TO_TOP,
	SCROLL_TO_BOTTOM
};

class ScrollerNode : public Node
{
public:
	ScrollerNode(
		const GUI& gui,
		const std::string& frame);
	virtual ~ScrollerNode();

	void setContent(const std::vector<Node*>& contentNodes, ScrollStrategy strategy);
	void setContentPadding(float contentPadding) { m_contentPadding = contentPadding; }

	void scroll(const float amount);

	void setContentSize(const glm::vec2& contentSize) override;

private:
	const GUI& m_gui;
	SpriteNode* m_sprite;
	size_t m_currentIndex;
	size_t m_currentVisibleCount;
	float m_contentPadding;

	std::vector<Node*> m_contentNodes;
	void refresh();
};

