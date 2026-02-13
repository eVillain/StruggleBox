#include "LayoutNode.h"

LayoutNode::LayoutNode()
	: m_type(LayoutType::Row)
	, m_padding(2.f)
	, m_alignmentH(LayoutAlignmentH::Left)
	, m_alignmentV(LayoutAlignmentV::Bottom)
{
}

void LayoutNode::refresh()
{
	m_usedSize = glm::vec2();

	if (m_type == LayoutType::Row)
	{
		float posX = getStartPos();
		for (Node* child : m_children)
		{
			child->setPositionX(posX);
			const glm::vec2 childSize = child->getContentSize();
			posX += childSize.x + m_padding;

			if (m_alignmentV == LayoutAlignmentV::Bottom)
			{
				child->setAnchorPoint(glm::vec2(0.f, 0.f));
				child->setPositionY(m_padding);
			}
			else if (m_alignmentV == LayoutAlignmentV::Middle)
			{
				child->setAnchorPoint(glm::vec2(0.f, 0.5f));
				child->setPositionY(m_contentSize.y * 0.5f);
			}
			else
			{
				child->setAnchorPoint(glm::vec2(0.f, 1.f));
				child->setPositionY(m_contentSize.y - m_padding);
			}
			if (childSize.y + m_padding > m_usedSize.y)
			{
				m_usedSize.y = childSize.y + m_padding;
			}
		}
		m_usedSize.x = getRequiredWidth();
	}
	else
	{
		float posY = getStartPos();
		for (Node* child : m_children)
		{
			const glm::vec2 childSize = child->getContentSize();
			posY -= childSize.y + m_padding;
			child->setPositionY(posY);

			if (m_alignmentH == LayoutAlignmentH::Left)
			{
				child->setAnchorPoint(glm::vec2(0.f, 0.f));
				child->setPositionX(m_padding);
			}
			else if (m_alignmentH == LayoutAlignmentH::Middle)
			{
				child->setAnchorPoint(glm::vec2(0.5f, 0.f));
				child->setPositionX(m_contentSize.x * 0.5f);
			}
			else // LayoutAlignment::Top
			{
				child->setAnchorPoint(glm::vec2(1.f, 0.f));
				child->setPositionX(m_contentSize.x - m_padding);
			}
			if (childSize.x + m_padding > m_usedSize.x)
			{
				m_usedSize.x = childSize.x + m_padding;
			}
		}
		m_usedSize.y = getRequiredHeight();
	}
}

const float LayoutNode::getRequiredWidth() const
{
	float requiredWidth = 0.f;
	for (Node* child : m_children)
	{
		requiredWidth += child->getContentSize().x + m_padding;
	}
	return requiredWidth;
}

const float LayoutNode::getRequiredHeight() const
{
	float requiredHeight = 0.f;
	for (Node* child : m_children)
	{
		requiredHeight += child->getContentSize().y + m_padding;
	}
	return requiredHeight;
}

float LayoutNode::getStartPos() const
{
	if (m_type == LayoutType::Row)
	{
		if (m_alignmentH == LayoutAlignmentH::Left)
		{
			return m_padding;
		}
		else if (m_alignmentH == LayoutAlignmentH::Middle)
		{
			const float requiredWidth = getRequiredWidth();
			return (m_contentSize.x - requiredWidth) * 0.5f;
		}
		else // LayoutAlignmentH::Right
		{
			const float requiredWidth = getRequiredWidth();
			return m_contentSize.x - requiredWidth;
		}
	}
	else
	{
		if (m_alignmentV == LayoutAlignmentV::Bottom)
		{
			return m_padding;
		}
		else if (m_alignmentV == LayoutAlignmentV::Middle)
		{
			const float requiredHeight = getRequiredHeight();
			return (m_contentSize.y - requiredHeight) * 0.5f;
		}
		else // LayoutAlignmentV::Top
		{
			//const float requiredHeight = getRequiredHeight();
			//return m_contentSize.y - requiredHeight;
			return m_contentSize.y;
		}
	}

	return 0.0f;
}
