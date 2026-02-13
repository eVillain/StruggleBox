#include "Camera2D.h"
#include <glm/gtc/matrix_transform.hpp>

Camera2D::Camera2D()
	: m_viewSize(1920, 1080)
	, m_nearDepth(-100.f)
	, m_farDepth(100.f)
	, m_zoom(1.f)
	, m_elasticity(0.25f)
{
}

Camera2D::~Camera2D()
{
}

void Camera2D::update(const double deltaTime)
{
	if (m_elasticity == 0.f)
	{
		m_position = m_targetPosition;
	}
	else
	{
		m_position = glm::mix(m_position, m_targetPosition, (1.f - m_elasticity) * deltaTime);
	}
	const glm::vec2 trueViewSize = glm::vec2(m_viewSize.x / m_zoom, m_viewSize.y / m_zoom);
	const glm::vec2 halfViewSize = trueViewSize / 2.f;
	m_projection = glm::ortho<float>(0.f, trueViewSize.x, 0.f, trueViewSize.y, m_nearDepth, m_farDepth);
	m_view = glm::mat4();

	// Center camera at middle coordinate
	const glm::vec2 viewOffset = halfViewSize - m_position;
	m_view = glm::translate(m_view, glm::vec3(viewOffset.x, viewOffset.y, 0.f));
}

glm::vec2 Camera2D::screenToWorld(const glm::ivec2& screenPosition) const
{
	const glm::vec2 worldPos = glm::vec2(screenPosition.x, m_viewSize.y - screenPosition.y) / m_zoom;
	const glm::vec2 trueViewSize = glm::vec2(m_viewSize.x / m_zoom, m_viewSize.y / m_zoom);
	const glm::vec2 halfViewSize = trueViewSize / 2.f;
	return worldPos + m_position - halfViewSize;
}

Rect2D Camera2D::getWorldRect() const
{
	const glm::vec2 trueViewSize = glm::vec2(m_viewSize.x / m_zoom, m_viewSize.y / m_zoom);
	const glm::vec2 halfViewSize = trueViewSize / 2.f;
	return Rect2D(m_position.x - halfViewSize.x, m_position.y - halfViewSize.y, trueViewSize.x, trueViewSize.y);
}
