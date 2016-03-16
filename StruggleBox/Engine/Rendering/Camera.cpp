#include "Camera.h"
#include "Random.h"
#include "Timer.h"
#include <glm/gtx/rotate_vector.hpp>
//#include <time.h>       /* time */
#include <iostream>

Camera::Camera() :
_followTarget(false)
{
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
    height = 1.0f;
}

Camera::~Camera()
{ }

void Camera::Update(double delta)
{
    if ( delta <= 0.0 ) return;

    if ( elasticMovement ) {
        if ( targetRotation.y - rotation.y < -180.0f ) targetRotation.y += 360.0f;
        else if ( targetRotation.y - rotation.y > 180.0f ) targetRotation.y -= 360.0f;

        // Dampen rotation
        float new_ratio = elasticity*0.02f * delta;
        float old_ratio = 1.0 - new_ratio;
        glm::vec3 new_dir = (rotation * old_ratio) + (targetRotation * new_ratio);
        rotation = new_dir;
        
//        rotation = targetRotation;
        if ( glm::length(movement) > 0.0f ) {
            // Apply direct camera movement
            CalculateCameraMovement( movement );
            targetPosition += speed*float(delta);
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
            
//            glm::vec3 move = behindTarget - position;
//            float dist = glm::length(move);
//            if ( dist > 20.0f ) {
//                move = glm::normalize(move)*20.0f;
//            }
//            speed = move*(elasticity*0.1f)*float(delta);
            
            if ( physicsClip && physicsFunc ) {
                position = physicsFunc(targetPosition, behindTarget);
            } else {
                position = behindTarget;
            }
        } else {
            glm::vec3 move = targetPosition - position;
            speed = move*elasticity*0.1f*float(delta);
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
            float dist = glm::distance(position, behindTarget);
            
            if (physicsClip &&
                physicsFunc &&
                dist > 0.001f) {
                position = physicsFunc(targetPosition, behindTarget);
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
                position = position+speed*float(delta);
            }
        }
    }
    
    if ( shakeAmount != 0.0f ) {
        Random::RandomSeed((int)Timer::Microseconds());
        int rX = Random::RandomInt(-1, 1);
        int rY = Random::RandomInt(-1, 1);
        shakeVect = glm::vec3(rX, rY, 0.0f)*shakeAmount;
//        printf("Shake:%f / %f\n", shakeVect.x, shakeVect.y);
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
void Camera::CalculateCameraMovement(glm::vec3 direction) {
    // Break up our movement into components along the X, Y and Z axis
    float camMovementXComponent = 0.0f;
    float camMovementYComponent = 0.0f;
    float camMovementZComponent = 0.0f;
    // Forward/backward movement
    if (direction.z != 0.0f) {
        // Control X-Axis movement
        float pitchFactor = cos(toRads(rotation.x));
        camMovementXComponent += ( movementSpeedFactor * float(sin(toRads(rotation.y))) * direction.z ) * pitchFactor;
        // Control Y-Axis movement
        camMovementYComponent += movementSpeedFactor * float(sin(toRads(rotation.x))* -direction.z );
        // Control Z-Axis movement
        float yawFactor = (cos(toRads(rotation.x)));
        camMovementZComponent += ( movementSpeedFactor * float(cos(toRads(rotation.y))) * direction.z ) * yawFactor;
    }
    // Strafing
    if ( direction.x !=  0.0f ) {
        // Calculate our Y-Axis rotation in radians once here because we use it twice
        float yRotRad = toRads(rotation.y);
        camMovementXComponent +=  movementSpeedFactor * float(cos(yRotRad)) * direction.x;
        camMovementZComponent +=  movementSpeedFactor * float(sin(yRotRad)) * -direction.x;
    }
    // Vertical movement
    if ( direction.y !=  0.0f ) {
        float xRotRad = toRads(rotation.x);
        float yRotRad = toRads(rotation.y);
        camMovementYComponent +=  movementSpeedFactor * float(cos(xRotRad)) * -direction.y;
        camMovementXComponent +=  movementSpeedFactor * float(sin(yRotRad)) * direction.y;
        camMovementZComponent +=  movementSpeedFactor * float(cos(yRotRad)) * direction.y;
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

