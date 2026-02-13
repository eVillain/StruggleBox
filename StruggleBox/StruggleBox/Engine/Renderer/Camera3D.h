#pragma once

#include "glm/glm.hpp"
#include <functional>

class Camera3D
{
public:
	Camera3D();
	~Camera3D();

	void update(const double deltaTime);

	const glm::vec3& getPosition() const { return m_position; }
	const glm::vec3& getRotation() const { return m_rotation; }
	const glm::vec3& getTargetPosition() const { return m_targetPosition; }
	const float getDistance() const { return m_distance; }
	const float getNearDepth() const { return m_nearDepth; }
	const float getFarDepth() const { return m_farDepth; }
	const float getFieldOfView() const { return m_fieldOfView; }
	const glm::mat4& getProjectionMatrix() const { return m_projection; }
	const glm::mat4& getViewMatrix() const { return m_view; }
	const bool getThirdPerson() const { return m_thirdPerson; }
	const bool getAutoRotate() const { return m_autoRotate; }
	const glm::vec3& getMovement() const { return m_movement; }

	void setPosition(const glm::vec3& position) { m_position = position; }
	void setRotation(const glm::vec3& rotation) { m_rotation = rotation; }
	void setTargetPosition(const glm::vec3& position) { m_targetPosition = position; }
	void setTargetRotation(const glm::vec3& rotation) { m_targetRotation = rotation; }
	void setMovement(const glm::vec3& movement) { m_movement = movement; }
	void setDistance(const float distance) { m_distance = distance; }
	void setNearDepth(float nearDepth) { m_nearDepth = nearDepth; }
	void setFarDepth(float farDepth) { m_farDepth = farDepth; }
	void setThirdPerson(bool thirdPerson) { m_thirdPerson = thirdPerson; }
	void setElasticMovement(bool elastic) { m_elasticMovement= elastic; }
	void setViewSize(const glm::ivec2& viewSize) { m_viewSize = viewSize; }

	void setPhysicsCallback(const std::function<glm::vec3(const glm::vec3&, const glm::vec3&)>& cb) { m_collisionCallback = cb; }

	void rotate(const float rotX, const float rotY);
	glm::mat4 getRotationMatrix() const;

private:
	friend class CameraWindow;

	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_targetPosition;
	glm::vec3 m_targetRotation;

	glm::vec3 m_speed;
	glm::vec3 m_movement;

	float m_distance;

	bool m_thirdPerson;
	bool m_elasticMovement;
	bool m_autoRotate;

	float m_nearDepth;
	float m_farDepth;
	float m_fieldOfView;

	glm::ivec2 m_viewSize;
	glm::mat4 m_projection;
	glm::mat4 m_view;

	std::function<glm::vec3(const glm::vec3&, const glm::vec3&)> m_collisionCallback;

	void calculateCameraMovement(const glm::vec3& direction);

};
