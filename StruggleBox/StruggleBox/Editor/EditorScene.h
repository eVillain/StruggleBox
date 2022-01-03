#ifndef EDITOR_SCENE_H
#define EDITOR_SCENE_H

#include "GUIScene.h"
#include "Input.h"
#include "EditorCursor3D.h"
#include "EditorRoom.h"
#include <vector>
#include <memory>

class Allocator;
class Camera;
class Renderer;
class Options;
class StatTracker;

class ButtonNode;
class SpriteNode;
class LayoutNode;
class OptionsWindow;
class FileWindow;
class LightsWindow;
class CameraWindow;

class BaseWidget3D;

class EditorScene : public GUIScene
{
public:
    EditorScene(
		Camera& camera,
        Allocator& allocator,
		Renderer& renderer,
		Options& options,
		Input& input,
        StatTracker& statTracker);
    virtual ~EditorScene();
    
    void Initialize() override;
    void ReInitialize() override;
    void Pause() override;
    void Resume() override;
    void Update(const double deltaTime) override;
    void Draw() override;
    
    virtual const std::string getFileType() const { return "*.*"; }
    virtual const std::string getFilePath() const;
    virtual void onFileLoad(const std::string& file) {}
    virtual void onFileSave(const std::string& file) {}

protected:
	Camera& m_camera;
	Renderer& m_renderer;
	Options& m_options;
	Input& m_input;

    EditorCursor3D m_cursor;
	EditorRoom m_room;

    glm::vec2 m_inputMove;
    glm::vec2 m_inputRotate;

    SpriteNode* m_topMenuBG;
    LayoutNode* m_layoutNode;
    ButtonNode* m_exitButton;
    ButtonNode* m_loadButton;
    ButtonNode* m_saveButton;
    ButtonNode* m_optionsButton;
    ButtonNode* m_lightsButton;
    ButtonNode* m_cameraButton;

	bool OnEvent(const InputEvent event, const float amount) override;
    bool OnMouse(const glm::ivec2& coord) override;
    
    void removeFileWindow();
    void removeOptionsWindow();
    void removeLightsWindow();
    void removeCameraWindow();

    void editPositionValue(glm::vec3& value);

    void drawBoxOutline(glm::vec3 center, glm::vec3 boxSize, Color color);

private:
    OptionsWindow* m_optionsWindow;
    FileWindow* m_fileWindow;
    LightsWindow* m_lightsWindow;
    CameraWindow* m_cameraWindow;

    BaseWidget3D* m_widget;

    void HandleMovement();

    void onExitButton(bool);
    void onLoadButton(bool);
    void onSaveButton(bool);
    void onOptionsButton(bool);
    void onLightsButton(bool);
    void onCameraButton(bool);

    void buildTopMenu();

};

#endif /* EDITOR_SCENE_H */
