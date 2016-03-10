#include "Transform.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

Transform::Transform() :
_position(),
_orientation(),
_matrix(1.0f),
_dirty(false)
{ }

void Transform::SetPosition(const glm::vec3& position)
{
    _position = position;
    _dirty = true;
}

void Transform::SetOrientation(const glm::quat& orientation)
{
    _orientation = orientation;
    _dirty = true;
}

glm::vec3 Transform::GetPosition() const
{
    return _position;
}

glm::quat Transform::GetOrientation() const
{
    return _orientation;
}

glm::mat4 Transform::GetMatrix()
{
    if (_dirty)
    {
        glm::mat4 rot = glm::toMat4(_orientation);
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), _position);
        _matrix = rot * trans;
        _dirty = false;
    }
    return _matrix;
}

// Syntactic sugar
void Transform::SetPositionX(const float position)
{
    _position.x = position;
    _dirty = true;
}
void Transform::SetPositionY(const float position)
{
    _position.y = position;
    _dirty = true;
}
void Transform::SetPositionZ(const float position)
{
    _position.z = position;
    _dirty = true;
}

