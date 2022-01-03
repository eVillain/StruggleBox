#ifndef PARTICLE3D_EDITOR_H
#define PARTICLE3D_EDITOR_H

#include "EditorScene.h"
#include "Input.h"
#include "Particles.h"
#include "ParticleSys.h"
#include <memory>

class Allocator;
class Camera;
class Renderer;
class Options;
class Input;
class Menu;
class FileMenu;

class Particle3DEditor : public EditorScene
{
public:
    Particle3DEditor(
        Camera& camera,
        Allocator& allocator,
        Renderer& renderer,
        Options& options,
        Input& input,
        StatTracker& statTracker);
    ~Particle3DEditor();
    
    void Initialize() override;
    void ReInitialize() override;
    void Pause() override;
    void Resume() override;
    void Update(const double deltaTime) override;
    void Draw() override;

    const std::string getFileType() const override { return "*.plist"; }
    const std::string getFilePath() const override;
    void onFileLoad(const std::string& file) override { LoadSystem(file); }
    void onFileSave(const std::string& file) override { SaveSystem(file); }
    
private:
    Particles _particles;

    void ShowEditor();
    void RemoveEditor();
    
    std::shared_ptr<Menu> _editorMenu;
    std::shared_ptr<Menu> _particleMenu;
    std::shared_ptr<FileMenu> _fileSelectMenu;

    ParticleSystemID m_particleSystemID;
    ParticleSystem* _particleSys;
    float timeScaler;

    glm::vec2 joyMoveInput;
    glm::vec2 joyRotateInput;
    
    bool OnEvent(const InputEvent event, const float amount);
    bool OnMouse(const glm::ivec2& coord);
    
    void RefreshParticleMenu();
    
    void LoadSystem(const std::string& fileName);
    void SaveSystem(const std::string& fileName);

};

#endif
