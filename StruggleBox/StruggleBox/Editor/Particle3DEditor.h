#pragma once

#include "EditorScene.h"
#include "Node.h"
#include "Particles.h"
#include "ParticleSystem.h"
#include <memory>

class Allocator;
class Renderer2D;
class Options;
class FileMenu;

class Particle3DEditor : public EditorScene
{
public:
    Particle3DEditor(
        Allocator& allocator,
        Injector& injector,
        VoxelRenderer& renderer,
        RenderCore& renderCore,
        OSWindow& osWindow,
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
    Renderer2D* m_renderer2D;
    Particles m_particles;
    
    Node* m_editorMenu;
    Node* m_particleMenu;
    std::shared_ptr<FileMenu> m_fileSelectMenu;

    ParticleSystemID m_particleSystemID;
    ParticleSystem* m_particleSys;
    float m_timeScaler;

    glm::vec2 m_joyMoveInput;
    glm::vec2 m_joyRotateInput;

    void ShowEditor();
    void RemoveEditor();
    
    bool OnEvent(const InputEvent event, const float amount);
    bool OnMouse(const glm::ivec2& coord);
    
    void RefreshParticleMenu();
    
    void LoadSystem(const std::string& fileName);
    void SaveSystem(const std::string& fileName);
};
