#ifndef CAMERA_H
#define CAMERA_H

#include "GFXDefines.h"
#include "GFXHelpers.h"

typedef const glm::vec3 (*PhysicsCallback)(const glm::vec3& fromPos,
                                           const glm::vec3& toPos);

class Camera
{
public:
    Camera();
    ~Camera();
    
    void Update( double delta );
    // Function to deal with mouse position changes, called whenever the mouse cursor moves
    void CameraRotate( const float rotX, const float rotY );
    // Function to calculate which direction we need to move the camera and by what amount
    void CalculateCameraMovement(glm::vec3 movement);    
    // Function to move the camera the amount we've calculated in the calculateCameraMovement function
    void MoveCamera( double dela );
    
    void SetPhysicsCallback( PhysicsCallback cb ) { physicsFunc = cb; };
    
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
    
    GLfloat movementSpeedFactor;    // How fast we move (higher values mean we move and strafe faster)
    GLfloat fieldOfView;            // Define our field of view (i.e. how quickly foreshortening occurs)
    GLfloat nearDepth;              // The near (Z Axis) point of our viewing frustrum (default 0.01f)
    GLfloat farDepth;               // The far  (Z Axis) point of our viewing frustrum (default 1000.0f)
    // DOF shader lens variables
    GLfloat focalDepth;             // The focal point depth in metres (default 10.0f)
    GLfloat focalLength;            // The focal length in mm
    GLfloat fStop;                  // The lens f-stop value
    GLfloat exposure;               // The shutter exposure
    GLboolean debugLens;            // Show debug focal point and range
    bool autoFocus;                 // Automatically focus to center

private:
    PhysicsCallback physicsFunc;               // Pointer to a static callback function
};

#endif
