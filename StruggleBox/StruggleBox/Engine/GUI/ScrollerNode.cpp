#include "ScrollerNode.h"

#include "ScrollBar.h"
#include "SpriteNode.h"
#include "GUI.h"
#include "MathUtils.h"
#include <algorithm>

ScrollerNode::ScrollerNode(
	const GUI& gui,
	const std::string& frame)
	: m_gui(gui)
	, m_sprite(nullptr)
	, m_scrollBar(nullptr)
	, m_currentIndex(0)
	, m_currentVisibleCount(0)
	, m_contentPadding(2.f)
	, m_showScrollBar(false)
{
	Log::Debug("ScrollerNode constructor at %p", this);

	m_sprite = gui.createSpriteNode(frame);
	if (m_sprite)
	{
		m_sprite->setEnable9Slice(true);
		m_contentSize = m_sprite->getContentSize();
		addChild(m_sprite);
	}
}

ScrollerNode::~ScrollerNode()
{
	Log::Debug("ScrollerNode destructor at %p", this);
	const size_t endVisibleIndex = m_currentIndex + m_currentVisibleCount;
	for (size_t index = m_currentIndex; index < endVisibleIndex; index++)
	{
		m_contentNodes[index] = nullptr;
	}
	m_contentNodes.erase(std::remove_if(
		m_contentNodes.begin(),
		m_contentNodes.end(),
		[](Node* node) {
			return node == nullptr;
		}),
		m_contentNodes.end());

	if (!m_contentNodes.empty())
	{
		m_gui.destroyNodes(m_contentNodes);
	}
}

void ScrollerNode::setContent(const std::vector<Node*>& contentNodes, ScrollStrategy strategy)
{
	m_sprite->removeAllChildren();
	if (!m_contentNodes.empty())
	{
		m_gui.destroyNodes(m_contentNodes);
	}
	m_contentNodes = contentNodes;

	if (strategy == ScrollStrategy::KEEP_OFFSET)
	{
		m_currentIndex = (std::min)(m_currentIndex, m_contentNodes.size() - 1);
	}
	else if (strategy == ScrollStrategy::SCROLL_TO_TOP)
	{
		m_currentIndex = 0;
	}
	else if (strategy == ScrollStrategy::SCROLL_TO_BOTTOM)
	{
		m_currentIndex = (std::max)(m_contentNodes.size() - 5, size_t(0));
	}
	refresh();
}

void ScrollerNode::setShowScrollBar(bool showScrollBar)
{
	m_showScrollBar = showScrollBar;
	if (m_showScrollBar && !m_scrollBar)
	{
		m_scrollBar = m_gui.createDefaultScrollBar();
		m_scrollBar->setCallback(std::bind(&ScrollerNode::setIndex, this, std::placeholders::_1));
		addChild(m_scrollBar);
		updateScrollBar();
	}
	else if (!m_showScrollBar && m_scrollBar)
	{
		removeChild(m_scrollBar);
	}
}

void ScrollerNode::setIndex(size_t index)
{
	if (index > m_contentNodes.size() - 1 || index < 0)
	{
		return;
	}
	m_currentIndex = index;
	refresh();
}

bool ScrollerNode::onScroll(const float amount)
{
	if ((m_currentIndex == 0 && amount < 0.f) ||
		(m_currentIndex == m_contentNodes.size() - 1 && amount > 0.f))
	{
		return true;
	}

	m_currentIndex = MathUtils::Clamp((size_t)(m_currentIndex + amount), (size_t)0, m_contentNodes.size() - 1);
	refresh();
	if (m_showScrollBar)
	{
		m_scrollBar->setValue(m_currentIndex);
	}
	return true;
}

void ScrollerNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);
	if (m_sprite)
	{
		m_sprite->setContentSize(contentSize);
	}
	if (m_showScrollBar)
	{
		updateScrollBar();
	}
}

void ScrollerNode::refresh()
{
	const auto& children = m_sprite->getChildren();
	m_sprite->removeAllChildren();
	m_currentVisibleCount = 0;

	float maxWidth = m_contentSize.x - (m_contentPadding * 2.f);
	if (m_showScrollBar)
	{
		maxWidth -= m_scrollBar->getContentSize().x;
		m_scrollBar->setValueExtents(0, m_contentNodes.size());
	}
	float height = 0.f;
	for (size_t index = m_currentIndex; index < m_contentNodes.size(); index++)
	{
		Node* nextNode = m_contentNodes.at(index);
		const glm::vec2 oldContentSize = nextNode->getContentSize();
		nextNode->setContentSize(glm::vec2(MathUtils::Min(oldContentSize.x, maxWidth), oldContentSize.y));
		height += nextNode->getContentSize().y + m_contentPadding;
		if (height > m_contentSize.y)
		{
			return;
		}

		nextNode->setPosition(glm::vec3(0.f, m_contentSize.y - height, 2.f));
		m_sprite->addChild(nextNode);
		m_currentVisibleCount++;
	}
}

void ScrollerNode::updateScrollBar()
{
	m_scrollBar->setContentSize(glm::vec2(24.f, m_contentSize.y));
	m_scrollBar->setPositionX(m_contentSize.x - 24.f);
}
