#include "Camera.h"
#include "Random.h"
#include "Timer.h"
#include "Log.h"
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

Camera::Camera() :
_followTarget(false),
m_collisionCallback(nullptr)
{
	Log::Info("[Camera] constructor, instance at %p", this);

    fieldOfView = 70.0f;
    nearDepth = 0.1f;
    farDepth = 250.1f;
    focalDepth = 6.0;
    focalLength = 30.0;
    fStop = 10.1f;
    exposure = 4.0f;
    debugLens = false;
    autoFocus = false;
    
    shakeVect = glm::vec3();
    shakeAmount = 0.0f;
    shakeDecay = 0.85f;
    
    rotationSensitivity = glm::vec2(20.0f,20.0f);
    movementSpeedFactor = 20.0f;
    
    elasticMovement = false;
    autoRotate = false;
    thirdPerson = false;
    physicsClip = true;
    elasticity = 30.0f;
    distance = 6.0f;
    maxDistance = 20.0f;
    height = 0.0f;
}

void Camera::Update(const double deltaTime)
{
    if (deltaTime <= 0.0 ) return;

    if ( elasticMovement ) {
        if ( targetRotation.y - rotation.y < -180.0f ) targetRotation.y += 360.0f;
        else if ( targetRotation.y - rotation.y > 180.0f ) targetRotation.y -= 360.0f;

        // Dampen rotation
        float new_ratio = elasticity * 0.02f * deltaTime;
        float old_ratio = 1.f - new_ratio;
        glm::vec3 new_dir = (rotation * old_ratio) + (targetRotation * new_ratio);
        rotation = new_dir;
        
        if ( glm::length(movement) > 0.0f ) {
            // Apply direct camera movement
            CalculateCameraMovement( movement );
            targetPosition += speed*float(deltaTime);
        }
        if ( thirdPerson ) {
            // Find coordinate for camera
            glm::vec3 zVect = glm::vec3(0.0f,0.0f,1.0f);
            glm::vec3 zPos = zVect*distance;
            zPos = glm::rotateX(zPos, rotation.x);
            zPos = glm::rotateY(zPos, rotation.y);
            zPos = glm::rotateZ(zPos, -rotation.z);
            glm::vec3 behindTarget = targetPosition + zPos;
            behindTarget.y += height;
            
            if ( physicsClip && m_collisionCallback ) {
                position = m_collisionCallback(targetPosition, behindTarget);
            } else {
                position = behindTarget;
            }
        } else {
            glm::vec3 move = targetPosition - position;
            speed = move*elasticity*0.1f*float(deltaTime);
            position = position+speed;
        }
    } else {
        
        if ( thirdPerson ) {
            // Find coordinate for camera
            glm::vec3 zVect = glm::vec3(0.0f,0.0f,1.0f);
            glm::vec3 zPos = zVect*distance;
            zPos = glm::rotateX(zPos, rotation.x);
            zPos = glm::rotateY(zPos, rotation.y);
            zPos = glm::rotateZ(zPos, -rotation.z);
            glm::vec3 behindTarget = targetPosition + zPos;
            behindTarget.y += height;
            float dist = glm::distance(targetPosition, behindTarget);
            
            if (physicsClip &&
                m_collisionCallback &&
                dist > 0.001f) {
                position = m_collisionCallback(targetPosition, behindTarget);
            } else {
                position = behindTarget;
            }
        } else {
            if (_followTarget)
            {
                rotation = targetRotation;
                position = targetPosition;
            } else {
                CalculateCameraMovement( movement );
                position += speed*float(deltaTime);
            }
        }
    }
    
    if ( shakeAmount != 0.0f ) {
        Random::RandomSeed((int)Timer::Microseconds());
        int rX = Random::RandomInt(-1, 1);
        int rY = Random::RandomInt(-1, 1);
        shakeVect = glm::vec3(rX, rY, 0.0f)*shakeAmount;
        position += shakeVect;
        shakeAmount *= shakeDecay;
        if ( fabsf(shakeAmount) < 0.01f ) { shakeAmount = 0.0f; shakeVect = glm::vec3(); }
    }
}
// Function to deal with mouse position changes, called whenever the mouse cursor moves
void Camera::CameraRotate(const float rotX,
                          const float rotY)
{
//    int horizMovement = -rotX;
//    int vertMovement = -rotY;

//    if ( abs(horizMovement) < 1 ) horizMovement = 0;
//    if ( abs(vertMovement) < 1 ) vertMovement = 0;
    
//    targetRotation.x += vertMovement / rotationSensitivity.y;
//    targetRotation.y += horizMovement / rotationSensitivity.x;
    rotation.x += rotY;
    rotation.y += rotX;
    
    // Control looking up and down with the mouse forward/back movement
    double_clamp(rotation.x, -90.0, 90.0);
    
    // Looking left and right. Keep the angles in the range -180.0f (anticlockwise turn looking behind) to 180.0f (clockwise turn looking behind)
    if (rotation.y < -180.0f) {
        rotation.y += 360.0f;
    }
    if (rotation.y > 180.0f) {
        rotation.y -= 360.0f;
    }
    targetRotation = rotation;
//    if ( rotation.y < 0.0f ) rotation.y += 360.0f;
//    if ( rotation.y > 360.0f ) rotation.y -= 360.0f;

}
// Function to calculate which direction we need to move the camera and by what amount
void Camera::CalculateCameraMovement(glm::vec3 direction)
{

    // Break up our movement into components along the X, Y and Z axis
    float camMovementXComponent = 0.0f;
    float camMovementYComponent = 0.0f;
    float camMovementZComponent = 0.0f;
    // Forward/backward movement
    if (direction.z != 0.0f) {
        // Control X-Axis movement
        float pitchFactor = cos(toRads(rotation.x));
        camMovementXComponent += (movementSpeedFactor * float(sin(rotation.y)) * -direction.z) * pitchFactor;
        // Control Y-Axis movement
        camMovementYComponent += movementSpeedFactor * float(sin(rotation.x)* direction.z);
        // Control Z-Axis movement
        float yawFactor = (cos(toRads(rotation.z)));
        camMovementZComponent += ( movementSpeedFactor * float(cos(rotation.y)) * -direction.z) * yawFactor;
    }
    // Strafing
    if ( direction.x !=  0.0f ) {
        camMovementXComponent +=  movementSpeedFactor * float(cos(rotation.y)) * direction.x;
        camMovementZComponent +=  movementSpeedFactor * float(sin(rotation.y)) * -direction.x;
    }
    // Vertical movement
    if ( direction.y !=  0.0f ) {
        camMovementYComponent +=  movementSpeedFactor * float(cos(rotation.x)) * -direction.y;
        camMovementXComponent +=  movementSpeedFactor * float(sin(rotation.y)) * direction.y;
        camMovementZComponent +=  movementSpeedFactor * float(cos(rotation.y)) * direction.y;
    }
    // After combining our movements for any & all keys pressed, assign them to our camera speed along the given axis
    speed.x = camMovementXComponent;
    speed.y = camMovementYComponent;
    speed.z = camMovementZComponent;

    // Cap the speeds to our movementSpeedFactor (otherwise going forward and strafing at an angle is twice as fast as just going forward!)
    double_clamp(speed.x, -movementSpeedFactor, movementSpeedFactor);
    double_clamp(speed.y, -movementSpeedFactor, movementSpeedFactor);
    double_clamp(speed.z, -movementSpeedFactor, movementSpeedFactor);
}

// Function to move the camera the amount we've calculated in the calculateCameraMovement function
void Camera::MoveCamera(double deltaTime)
{
    position += speed*(float)deltaTime;
}

