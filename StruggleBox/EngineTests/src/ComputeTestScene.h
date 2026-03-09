#pragma once

#include "GUIScene.h"

class Renderer2D;
class RenderCore;
class SceneManager;

class ComputeTestScene : public GUIScene
{
public:
    ComputeTestScene(
        Allocator& allocator,
        Renderer2D& renderer,
        RenderCore& renderCore,
        SceneManager& sceneManager,
        Input& input,
        OSWindow& window,
        Options& options,
        StatTracker& statTracker);
    ~ComputeTestScene();

    void Initialize() override;
    void ReInitialize() override;

    void Pause() override;
    void Resume() override;

    void Update(const double delta) override;
    void Draw() override;

    bool OnEvent(const InputEvent event, const float amount) override;
private:
    Renderer2D& m_renderer;
    RenderCore& m_renderCore;
    SceneManager& m_sceneManager;
    TextureID m_textureID;
    ShaderID m_computeShaderID;
};
