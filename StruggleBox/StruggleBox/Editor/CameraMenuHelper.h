#ifndef CAMERA_MENU_HELPER_H
#define CAMERA_MENU_HELPER_H

#include "Camera.h"
#include "GUI.h"
#include "Menu.h"
#include "Button.h"
#include "Slider.h"
#include <memory>

class CameraMenuHelper
{
public:
    static std::shared_ptr<Menu> createCameraMenu(Camera& camera,
                                                  GUI* gui)
    {
        const glm::ivec2 itemSize = glm::ivec2(140, 24);
        auto cameraMenu = gui->CreateWidget<Menu>();
        cameraMenu->setName("Camera");
        cameraMenu->setSize(itemSize);
        {
            auto followTargetBtn = gui->CreateWidget<Button>();
            std::string followLabel = "Follow Target: ";
            followLabel += (camera.followTarget() ? "ON": "OFF");
            followTargetBtn->setLabel(followLabel);
            followTargetBtn->setSize(itemSize);
            followTargetBtn->SetBehavior(new ButtonBehaviorLambda([&](){
                bool follow = camera.followTarget();
                camera.setFollowTarget(!follow);
                std::string followLabel = "Follow Target: ";
                followLabel += (camera.followTarget() ? "ON": "OFF");
                followTargetBtn->setLabel(followLabel);
            }));
            cameraMenu->addWidget(followTargetBtn);
        }
        return cameraMenu;
        /*
         cameraMenu->AddVar<bool>("Auto Rotate", &camera.autoRotate, "Camera" );
         cameraMenu->AddVar<bool>("3rd Person", &camera.thirdPerson, "Camera" );
         cameraMenu->AddSlider<float>("Distance", &camera.distance, 1.0f, 20.0f, "Camera" );
         cameraMenu->AddSlider<float>("Height", &camera.height, -20.0f, 20.0f, "Camera" );
         cameraMenu->AddVar<bool>("Elastic", &camera.elasticMovement, "Camera" );
         cameraMenu->AddSlider<float>("Elasticity", &camera.elasticity, 1.0f, 100.0f, "Camera" );
         cameraMenu->AddVar<bool>("Clipping", &camera.physicsClip, "Camera" );
         cameraMenu->AddVar<bool>("AutoFocus", &camera.autoFocus, "Camera" );
         cameraMenu->AddSlider<float>("FOV", &camera.fieldOfView, 10.0f, 179.0f, "Camera" );
         cameraMenu->AddSlider<float>("Near Depth", &camera.nearDepth, 0.001f, 1.0f, "Camera" );
         cameraMenu->AddSlider<float>("Far Depth", &camera.farDepth, 10.0f, 1000.0f, "Camera" );
         cameraMenu->AddSlider<float>("Focal Depth", &camera.focalDepth, 10.0f, 200.0f, "Camera" );
         cameraMenu->AddSlider<float>("Focal Length", &camera.focalLength, 30.0f, 300.0f, "Camera" );
         */
    }
};

#endif /* CAMERA_MENU_HELPER_H */
