#ifndef EDITOR_CURSOR3D_H
#define EDITOR_CURSOR3D_H

#include <glm\glm.hpp>

struct EditorCursor3D
{
    glm::vec2 posScrn;
    glm::vec3 posWorld;
    
    glm::vec2 lClickPosScrn;
    glm::vec3 lClickPosWorld;
    glm::vec2 rClickPosScrn;
    glm::vec3 rClickPosWorld;
    bool leftClick;
    bool rightClick;
    
    EditorCursor3D();
};
#endif /* EDITOR_CURSOR3D_H */
