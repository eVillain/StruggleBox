#include "ScrollerNode.h"

#include "SpriteNode.h"
#include "GUI.h"

ScrollerNode::ScrollerNode(
	const GUI& gui,
	const std::string& frame)
	: m_gui(gui)
	, m_sprite(nullptr)
	, m_currentIndex(0)
	, m_currentVisibleCount(0)
	, m_contentPadding(2.f)
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
		m_currentIndex = std::min(m_currentIndex, m_contentNodes.size() - 1);
	}
	else if (strategy == ScrollStrategy::SCROLL_TO_TOP)
	{
		m_currentIndex = 0;
	}
	else if (strategy == ScrollStrategy::SCROLL_TO_BOTTOM)
	{
		m_currentIndex = std::max(m_contentNodes.size() - 5, size_t(0));
	}
	refresh();
}

void ScrollerNode::scroll(const float amount)
{
	if ((m_currentIndex == 0 && amount < 0.f) ||
		(m_currentIndex == m_contentNodes.size() - 1 && amount > 0.f))
	{
		return;
	}

	m_currentIndex += amount;
	refresh();
}

void ScrollerNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);
	if (m_sprite)
	{
		m_sprite->setContentSize(contentSize);
	}
}

void ScrollerNode::refresh()
{
	const auto& children = m_sprite->getChildren();
	m_sprite->removeAllChildren();
	m_currentVisibleCount = 0;

	float height = 0.f;
	for (size_t index = m_currentIndex; index < m_contentNodes.size(); index++)
	{
		Node* nextNode = m_contentNodes.at(index);
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
