#ifndef EDITOR_SCENE_H
#define EDITOR_SCENE_H

#include "GUIScene.h"
#include "Input.h"
#include "EditorCursor3D.h"
#include "EditorRoom.h"
#include "tb_select_item.h"
#include <vector>
#include <memory>

class Camera;
class Renderer;
class Options;

class EditorScene :
public GUIScene, public InputEventListener, public MouseEventListener
{
public:
    EditorScene(
		std::shared_ptr<TBGUI> gui,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<Options> options,
		std::shared_ptr<Input> input);
    virtual ~EditorScene();
    
    virtual void Initialize();
    virtual void ReInitialize();
    virtual void Release();
    virtual void Pause();
    virtual void Resume();
    virtual void Update(const double deltaTime);
    virtual void Draw();
    
protected:
	std::shared_ptr<Camera> _camera;
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<Options> _options;
	std::shared_ptr<Input> _input;

    EditorCursor3D _cursor;
	EditorRoom _room;

    glm::vec2 _inputMove;
    glm::vec2 _inputRotate;

	tb::TBGenericStringItemSource _file_menu_source;

	bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);
    
private:
    void HandleMovement();
};

#endif /* EDITOR_SCENE_H */
