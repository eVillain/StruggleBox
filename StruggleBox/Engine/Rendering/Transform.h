#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform
{
public:
    Transform();
    
    void SetPosition(const glm::vec3& position);
    void SetOrientation(const glm::quat& orientation);
    glm::vec3 GetPosition() const;
    glm::quat GetOrientation() const;
    glm::mat4 GetMatrix();
    
    // Syntactic sugar
    void SetPositionX(const float position);
    void SetPositionY(const float position);
    void SetPositionZ(const float position);

    bool isDirty() const { return _dirty; }
    void unflagDirty() { _dirty = false; }
private:
    glm::vec3 _position;
    glm::quat _orientation;
    glm::mat4 _matrix;  // Cached, uses dirty flag below
    bool _dirty;
};

#endif /* TRANSFORM_H */
