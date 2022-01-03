#ifndef CAMERA_H
#define CAMERA_H

#include "GFXDefines.h"
#include "GFXHelpers.h"
#include <functional>

class Camera
{
public:
    Camera();
    
    void Update(const double deltaTime);
    // Function to deal with mouse position changes, called whenever the mouse cursor moves
    void CameraRotate(const float rotX,
                      const float rotY);
    
    void SetPhysicsCallback(const std::function<glm::vec3(const glm::vec3&, const glm::vec3&)>& cb) { m_collisionCallback = cb; }
    
    // Direct manipulation:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec2 rotationSensitivity;
    glm::vec3 movement;
    glm::vec3 speed;

    glm::vec3 shakeVect;
    float shakeDecay;
    float shakeAmount;
    
    // Elastic motion
    glm::vec3 targetPosition;
    glm::vec3 targetRotation;
    bool elasticMovement;
    bool autoRotate;
    float elasticity;
    
    bool thirdPerson;
    float distance;
    float maxDistance;
    float height;
    bool physicsClip;
    
    float movementSpeedFactor;    // How fast we move (higher values mean we move and strafe faster)
    float fieldOfView;            // Define our field of view (i.e. how quickly foreshortening occurs)
    float nearDepth;              // The near (Z Axis) point of our viewing frustrum (default 0.01f)
    float farDepth;               // The far  (Z Axis) point of our viewing frustrum (default 1000.0f)
    // DOF shader lens variables
    float focalDepth;             // The focal point depth in metres (default 10.0f)
    float focalLength;            // The focal length in mm
    float fStop;                  // The lens f-stop value
    float exposure;               // The shutter exposure
    bool debugLens;            // Show debug focal point and range
    bool autoFocus;                 // Automatically focus to center

    void setFollowTarget(const bool followTarget) { _followTarget = followTarget; }
    const bool followTarget() const { return _followTarget; }
private:
    bool _followTarget;
    
    // Function to calculate which direction we need to move the camera and by what amount
    void CalculateCameraMovement(glm::vec3 movement);
    // Function to move the camera the amount we've calculated in the calculateCameraMovement function
    void MoveCamera(double deltaTime);

    std::function<glm::vec3(const glm::vec3&, const glm::vec3&)> m_collisionCallback;
};                          

#endif
