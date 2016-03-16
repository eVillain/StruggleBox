#ifndef EDITOR_SCENE_H
#define EDITOR_SCENE_H

#include "Scene.h"
#include "Input.h"
#include "EditorCursor3D.h"
#include <vector>
#include <memory>

class Widget;

class EditorScene :
public Scene, public InputEventListener, public MouseEventListener
{
public:
    EditorScene(Locator& locator);
    virtual ~EditorScene();
    
    virtual void Initialize();
    virtual void ReInitialize();
    virtual void Release();
    virtual void Pause();
    virtual void Resume();
    virtual void Update(double deltaTime);
    virtual void Draw() = 0;
    
protected:
    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);
    
    std::vector<std::shared_ptr<Widget>> _widgets;
    EditorCursor3D _cursor;
    
    glm::vec2 _inputMove;
    glm::vec2 _inputRotate;
private:
    void HandleMovement();
};

#endif /* EDITOR_SCENE_H */
