#include "Node.h"

Node::Node()
	: m_position(0.f, 0.f, 0.f)
	, m_scale(1.f, 1.f)
	, m_contentSize(0.f, 0.f)
	, m_anchorPoint(0.f, 0.f)
	, m_parent(nullptr)
{
}

Node::~Node()
{
}

void Node::draw(Renderer& renderer, const glm::vec3& parentPosition, const glm::vec2& parentScale)
{
	const Rect2D bb = getBoundingBox();
	const glm::vec3 relativePosition = parentPosition + glm::vec3(bb.x, bb.y, m_position.z);
	const glm::vec2 relativeScale = parentScale * m_scale;

	for (Node* child : m_children)
	{
		child->draw(renderer, relativePosition, relativeScale);
	}
}

void Node::addChild(Node* child)
{
	assert(child->getParent() == nullptr);

	child->setParent(this);
	m_children.push_back(child);
}

void Node::removeChild(Node* child)
{
	const auto it = std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end())
	{
		(*it)->setParent(nullptr);
		m_children.erase(it);
		return;
	}
	assert(false); // child not found
}

void Node::removeAllChildren()
{
	for (Node* child : m_children)
	{
		child->setParent(nullptr);
	}
	m_children.clear();
}

Rect2D Node::getBoundingBox() const
{
	const glm::vec2 scaledContentSize = m_contentSize * m_scale;
	const glm::vec2 anchorOffset = scaledContentSize * m_anchorPoint;
	const glm::vec2 bottomLeft = glm::vec2(m_position.x, m_position.y) - anchorOffset;
	return Rect2D(bottomLeft.x, bottomLeft.y, scaledContentSize.x, scaledContentSize.y);
}
