#include "EditorCursor3D.h"

EditorCursor3D::EditorCursor3D()
{
    posScrn = glm::vec2();
    posWorld = glm::vec3();
    
    lClickPosScrn = glm::vec2();
    lClickPosWorld = glm::vec3();
    rClickPosScrn = glm::vec2();
    rClickPosWorld = glm::vec3();
    leftClick = false;
    rightClick = false;
}

