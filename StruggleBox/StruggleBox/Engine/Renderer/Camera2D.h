#pragma once

#include "glm/glm.hpp"
#include "Rect2D.h"

class Camera2D
{
public:
	Camera2D();
	~Camera2D();

	void update(const double deltaTime);
	glm::vec2 screenToWorld(const glm::ivec2& screenPosition) const;
	Rect2D getWorldRect() const;


	const glm::vec2& getPosition() const { return m_position; }
	const float getZoom() const { return m_zoom; }
	const glm::mat4& getProjectionMatrix() const { return m_projection; }
	const glm::mat4& getViewMatrix() const { return m_view; }
	const glm::ivec2& getViewSize() const { return m_viewSize; }

	void setPosition(const glm::vec2& position) { m_position = position; }
	void setTargetPosition(const glm::vec2& position) { m_targetPosition = position; }
	void setZoom(const float zoom) { m_zoom = zoom; }
	void setViewSize(const glm::ivec2& viewSize) { m_viewSize = viewSize; }
	void setNearDepth(float nearDepth) { m_nearDepth = nearDepth; }
	void setFarDepth(float farDepth) { m_farDepth = farDepth; }

private:
	glm::vec2 m_position;
	glm::vec2 m_targetPosition;
	float m_zoom;
	float m_elasticity;
	glm::ivec2 m_viewSize;
	float m_nearDepth;
	float m_farDepth;
	glm::mat4 m_projection;
	glm::mat4 m_view;
};

