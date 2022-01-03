#include "LayoutNode.h"

#include "Renderer.h"

LayoutNode::LayoutNode()
	: m_type(LayoutType::Row)
	, m_padding(2.f)
	, m_alignmentH(LayoutAlignmentH::Left)
	, m_alignmentV(LayoutAlignmentV::Bottom)
{
}

//LayoutNode::~LayoutNode()
//{
//}
//
//void LayoutNode::addChild(Node* child)
//{
//	Node::addChild(child);
//
//	refresh();
//}
//
//void LayoutNode::removeChild(Node* child)
//{
//	Node::removeChild(child);
//
//	refresh();
//}

void LayoutNode::refresh()
{
	if (m_type == LayoutType::Row)
	{
		float posX = getStartPos();
		for (Node* child : m_children)
		{
			child->setPositionX(posX);
			posX += child->getContentSize().x + m_padding;

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
		}
	}
	else
	{
		float posY = getStartPos();
		for (Node* child : m_children)
		{
			child->setPositionY(posY);
			posY += child->getContentSize().y + m_padding;

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
			else
			{
				child->setAnchorPoint(glm::vec2(1.f, 0.f));
				child->setPositionX(m_contentSize.x - m_padding);
			}
		}
	}
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
			float requiredWidth = 0.f;
			for (Node* child : m_children)
			{
				requiredWidth += child->getContentSize().x + m_padding;
			}
			return (m_contentSize.x - requiredWidth) * 0.5f;
		}
		else
		{
			float requiredWidth = 0.f;
			for (Node* child : m_children)
			{
				requiredWidth += child->getContentSize().x + m_padding;
			}
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
			float requiredHeight = 0.f;
			for (Node* child : m_children)
			{
				requiredHeight += child->getContentSize().y + m_padding;
			}
			return (m_contentSize.y - requiredHeight) * 0.5f;
		}
		else
		{
			float requiredHeight = 0.f;
			for (Node* child : m_children)
			{
				requiredHeight += child->getContentSize().y + m_padding;
			}
			return m_contentSize.y - requiredHeight;
		}
	}

	return 0.0f;
}
