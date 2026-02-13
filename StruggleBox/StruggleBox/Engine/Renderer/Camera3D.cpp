#include "Camera3D.h"

#include "GFXHelpers.h"
#include <glm/gtc/matrix_transform.hpp>


Camera3D::Camera3D()
    : m_position()
    , m_rotation()
    , m_targetPosition()
    , m_targetRotation()
    , m_speed()
    , m_movement()
    , m_distance(0.f)
    , m_thirdPerson(false)
    , m_elasticMovement(false)
    , m_autoRotate(false)
    , m_nearDepth(0.01f)
    , m_farDepth(250.f)
    , m_fieldOfView(70.f)
    , m_viewSize(1280, 720)
    , m_projection()
    , m_view()

{
}

Camera3D::~Camera3D()
{
}

void Camera3D::update(const double deltaTime)
{
    if (deltaTime <= 0.0) return;

    //if (elasticMovement) {
    //    if (targetRotation.y - rotation.y < -180.0f) targetRotation.y += 360.0f;
    //    else if (targetRotation.y - rotation.y > 180.0f) targetRotation.y -= 360.0f;

    //        Dampen rotation
    //    float new_ratio = elasticity * 0.02f * deltaTime;
    //    float old_ratio = 1.f - new_ratio;
    //    glm::vec3 new_dir = (rotation * old_ratio) + (targetRotation * new_ratio);
    //    rotation = new_dir;

    //    if (glm::length(movement) > 0.0f) {
    //            Apply direct camera movement
    //        CalculateCameraMovement(movement);
    //        targetPosition += speed * float(deltaTime);
    //    }
    //    if (thirdPerson)
    //    {
    //            Find coordinate for camera
    //        glm::vec3 zVect = glm::vec3(0.0f, 0.0f, 1.0f);
    //        glm::vec3 zPos = zVect * distance;
    //        zPos = glm::rotateX(zPos, rotation.x);
    //        zPos = glm::rotateY(zPos, rotation.y);
    //        zPos = glm::rotateZ(zPos, -rotation.z);
    //        glm::vec3 behindTarget = targetPosition + zPos;
    //        behindTarget.y += height;

    //        const float movementDistance = glm::distance(targetPosition, behindTarget);  // prevent physics callback crash when camera does not move
    //        if (movementDistance > 0.f && physicsClip && m_collisionCallback)
    //        {
    //            position = m_collisionCallback(targetPosition, behindTarget);
    //        }
    //        else {
    //            position = behindTarget;
    //        }
    //    }
    //    else {
    //        glm::vec3 move = targetPosition - position;
    //        speed = move * elasticity * 0.1f * float(deltaTime);
    //        position = position + speed;
    //    }
    //}
    //else {

        //if (m_thirdPerson) {
        //        // Find coordinate for camera
        //    glm::vec3 zVect = glm::vec3(0.0f, 0.0f, 1.0f);
        //    glm::vec3 zPos = zVect * distance;
        //    zPos = glm::rotateX(zPos, rotation.x);
        //    zPos = glm::rotateY(zPos, rotation.y);
        //    zPos = glm::rotateZ(zPos, -rotation.z);
        //    glm::vec3 behindTarget = targetPosition + zPos;
        //    behindTarget.y += height;
        //    float dist = glm::distance(targetPosition, behindTarget);

            //if (physicsClip &&
            //    m_collisionCallback &&
            //    dist > 0.001f) {
            //    position = m_collisionCallback(targetPosition, behindTarget);
            //}
            //else {
                //position = behindTarget;
            //}
        //}
        //else {
            //if (_followTarget)
            //{
            //    rotation = targetRotation;
            //    position = targetPosition;
            //}
            //else {
                calculateCameraMovement(m_movement);
                m_position += m_speed * float(deltaTime);
            //}
        //}
    //}

    //if (shakeAmount != 0.0f) {
    //    Random::RandomSeed((int)Timer::Microseconds());
    //    int rX = Random::RandomInt(-1, 1);
    //    int rY = Random::RandomInt(-1, 1);
    //    shakeVect = glm::vec3(rX, rY, 0.0f) * shakeAmount;
    //    position += shakeVect;
    //    shakeAmount *= shakeDecay;
    //    if (fabsf(shakeAmount) < 0.01f) { shakeAmount = 0.0f; shakeVect = glm::vec3(); }
    //}
    


    const float aspectRatio = (m_viewSize.x > m_viewSize.y) ? float(m_viewSize.x) / float(m_viewSize.y) : float(m_viewSize.y) / float(m_viewSize.x);

    m_projection = glm::perspective(m_fieldOfView, aspectRatio, m_nearDepth, m_farDepth);

    //m_projection = glm::rotate(m_projection, -m_rotation.x, glm::vec3(1.0, 0.0, 0.0));
    //m_projection = glm::rotate(m_projection, -m_rotation.y, glm::vec3(0.0, 1.0, 0.0));
    //m_projection = glm::rotate(m_projection, -m_rotation.z, glm::vec3(0.0, 0.0, 1.0));
    //m_projection = glm::translate(m_projection, glm::vec3(-m_position.x, -m_position.y, -m_position.z));

    m_view = glm::mat4();
    m_view = glm::rotate(m_view, -m_rotation.x, glm::vec3(1.0, 0.0, 0.0));
    m_view = glm::rotate(m_view, -m_rotation.y, glm::vec3(0.0, 1.0, 0.0));
    m_view = glm::rotate(m_view, -m_rotation.z, glm::vec3(0.0, 0.0, 1.0));
    m_view = glm::translate(m_view, glm::vec3(-m_position.x, -m_position.y, -m_position.z));

}

void Camera3D::rotate(const float rotX, const float rotY)
{
    //    int horizMovement = -rotX;
    //    int vertMovement = -rotY;

    //    if ( abs(horizMovement) < 1 ) horizMovement = 0;
    //    if ( abs(vertMovement) < 1 ) vertMovement = 0;

    //    targetRotation.x += vertMovement / rotationSensitivity.y;
    //    targetRotation.y += horizMovement / rotationSensitivity.x;
    m_rotation.x += rotY;
    m_rotation.y += rotX;

    // Control looking up and down with the mouse forward/back movement
    double_clamp(m_rotation.x, -90.0, 90.0);

    // Looking left and right. Keep the angles in the range -180.0f (anticlockwise turn looking behind) to 180.0f (clockwise turn looking behind)
    if (m_rotation.y < -180.0f) {
        m_rotation.y += 360.0f;
    }
    if (m_rotation.y > 180.0f) {
        m_rotation.y -= 360.0f;
    }
    //targetRotation = rotation;
    //    if ( rotation.y < 0.0f ) rotation.y += 360.0f;
    //    if ( rotation.y > 360.0f ) rotation.y -= 360.0f;

}

glm::mat4 Camera3D::getRotationMatrix() const
{
    glm::mat4 rotationMatrix;
    rotationMatrix = glm::rotate(rotationMatrix, -m_rotation.x, glm::vec3(1.0, 0.0, 0.0));
    rotationMatrix = glm::rotate(rotationMatrix, -m_rotation.y, glm::vec3(0.0, 1.0, 0.0));
    rotationMatrix = glm::rotate(rotationMatrix, -m_rotation.z, glm::vec3(0.0, 0.0, 1.0));
    return rotationMatrix;
}

// Function to calculate which direction we need to move the camera and by what amount
void Camera3D::calculateCameraMovement(const glm::vec3& direction)
{
    // Break up our movement into components along the X, Y and Z axis
    float camMovementXComponent = 0.0f;
    float camMovementYComponent = 0.0f;
    float camMovementZComponent = 0.0f;
    // Forward/backward movement
    if (direction.z != 0.0f) {
        // Control X-Axis movement
        float pitchFactor = cos(toRads(m_rotation.x));
        camMovementXComponent += (float(sin(m_rotation.y)) * -direction.z) * pitchFactor;
        // Control Y-Axis movement
        camMovementYComponent += float(sin(m_rotation.x) * direction.z);
        // Control Z-Axis movement
        float yawFactor = (cos(toRads(m_rotation.z)));
        camMovementZComponent += (float(cos(m_rotation.y)) * -direction.z) * yawFactor;
    }
    // Strafing
    if (direction.x != 0.0f) {
        camMovementXComponent += float(cos(m_rotation.y)) * direction.x;
        camMovementZComponent += float(sin(m_rotation.y)) * -direction.x;
    }
    // Vertical movement
    if (direction.y != 0.0f) {
        camMovementYComponent += float(cos(m_rotation.x)) * -direction.y;
        camMovementXComponent += float(sin(m_rotation.y)) * direction.y;
        camMovementZComponent += float(cos(m_rotation.y)) * direction.y;
    }
    // After combining our movements for any & all keys pressed, assign them to our camera speed along the given axis
    m_speed.x = camMovementXComponent;
    m_speed.y = camMovementYComponent;
    m_speed.z = camMovementZComponent;

    // Cap the speeds to our movementSpeedFactor (otherwise going forward and strafing at an angle is twice as fast as just going forward!)
    //double_clamp(m_speed.x, -movementSpeedFactor, movementSpeedFactor);
    //double_clamp(m_speed.y, -movementSpeedFactor, movementSpeedFactor);
    //double_clamp(m_speed.z, -movementSpeedFactor, movementSpeedFactor);
}
