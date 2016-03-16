#ifndef PARTICLE3D_EDITOR_H
#define PARTICLE3D_EDITOR_H

#include "EditorScene.h"
#include "CoreTypes.h"
#include "Input.h"
#include "ParticleSys.h"

class Menu;
class FileMenu;

class Particle3DEditor : public EditorScene
{
public:
    Particle3DEditor(Locator& locator);
    ~Particle3DEditor();
    
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(double deltaTime);
    void Draw();
    
private:
    void ShowEditor();
    void RemoveEditor();
    
    std::vector<ButtonBase*> buttonVect;
    std::shared_ptr<Menu> _editorMenu;
    std::shared_ptr<Menu> _particleMenu;
    std::shared_ptr<FileMenu> _fileSelectMenu;

    ParticleSys* _particleSys;
    float timeScaler;

    glm::vec2 joyMoveInput;
    glm::vec2 joyRotateInput;
    void UpdateMovement();
    
    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);

    // Editor button callbacks
    void LoadSystemButtonCB( void*data );
    void CloseEditorButtonCB( void*data );
    
    void LoadSystem( const std::string fileName );
    void SaveSystem( const std::string fileName );

};

#endif
