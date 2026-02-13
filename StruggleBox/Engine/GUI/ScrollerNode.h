#pragma once

#include "InteractableNode.h"
#include <vector>
#include <string>

class ScrollBar;
class SpriteNode;
class GUI;

enum class ScrollStrategy {
	KEEP_OFFSET,
	SCROLL_TO_TOP,
	SCROLL_TO_BOTTOM
};

class ScrollerNode : public InteractableNode
{
public:
	ScrollerNode(
		const GUI& gui,
		const std::string& frame);
	virtual ~ScrollerNode();

	void setContent(const std::vector<Node*>& contentNodes, ScrollStrategy strategy);
	void setContentPadding(float contentPadding) { m_contentPadding = contentPadding; }
	void setShowScrollBar(bool showScrollBar);
	void setIndex(size_t index);

	bool onPress(const glm::vec2& relativeCursorPosition) override { return false; }
	bool onRelease(const glm::vec2& relativeCursorPosition) override { return false; }
	bool onScroll(const float amount) override;
	void onHighlight(bool inside, const glm::vec2& relativeCursorPosition) override {}

	void setContentSize(const glm::vec2& contentSize) override;

	const std::vector<Node*> getContent() const { return m_contentNodes; }

private:
	const GUI& m_gui;
	SpriteNode* m_sprite;
	ScrollBar* m_scrollBar;

	size_t m_currentIndex;
	size_t m_currentVisibleCount;
	float m_contentPadding;
	bool m_showScrollBar;

	std::vector<Node*> m_contentNodes;
	void refresh();
	void updateScrollBar();
};

