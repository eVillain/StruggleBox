#pragma once

#include "Rect2D.h"
#include "glm/glm.hpp"
#include <vector>

class Renderer;

class Node
{
public:
	Node();
	virtual ~Node();

	virtual void draw(Renderer& renderer, const glm::vec3& parentPosition, const glm::vec2& parentScale);

	virtual void addChild(Node* child);
	virtual void removeChild(Node* child);
	virtual void removeAllChildren();

	Rect2D getBoundingBox() const;

	void setPosition(const glm::vec3& position) { m_position = position; }
	void setScale(const glm::vec2& scale) { m_scale = scale; }
	virtual void setContentSize(const glm::vec2& contentSize) { m_contentSize = contentSize; }
	void setAnchorPoint(const glm::vec2& anchorPoint) { m_anchorPoint = anchorPoint; }

	glm::vec3 getPosition() const { return m_position; }
	glm::vec2 getScale() const { return m_scale; }
	glm::vec2 getContentSize() const { return m_contentSize; }
	glm::vec2 getAnchorPoint() const { return m_anchorPoint; }
	Node* getParent() const { return m_parent; }
	const std::vector<Node*>& getChildren() const { return m_children; }

	void setPositionX(const float positionX) { setPosition(glm::vec3(positionX, m_position.y, m_position.z)); }
	void setPositionY(const float positionY) { setPosition(glm::vec3(m_position.x, positionY, m_position.z)); }
	void setPositionZ(const float positionZ) { setPosition(glm::vec3(m_position.x, m_position.y, positionZ)); }

protected:
	glm::vec3 m_position;
	glm::vec2 m_scale;
	glm::vec2 m_contentSize;
	glm::vec2 m_anchorPoint;

	Node* m_parent;
	std::vector<Node*> m_children;

	void setParent(Node* parent) { m_parent = parent; }
};

