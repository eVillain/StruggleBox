#pragma once

#include "GUIScene.h"

class Renderer2D;
class RenderCore;
class SceneManager;

class DDATestScene : public GUIScene
{
public:
    DDATestScene(
        Allocator& allocator,
        Renderer2D& renderer,
        RenderCore& renderCore,
        SceneManager& sceneManager,
        Input& input,
        OSWindow& window,
        Options& options,
        StatTracker& statTracker);
    ~DDATestScene();

    void Initialize() override;
    void ReInitialize() override;

    void Pause() override;
    void Resume() override;

    void Update(const double delta) override;
    void Draw() override;

    bool OnEvent(const InputEvent event, const float amount) override;
    bool OnMouse(const glm::ivec2& coord) override;

private:
    Renderer2D& m_renderer;
    RenderCore& m_renderCore;
    SceneManager& m_sceneManager;
    TextureID m_textureID;

    glm::vec2 m_mouseCoord;
    bool m_dragStart;
    bool m_dragEnd;

    double m_time;
    std::vector<glm::vec2> m_edgePoints;
    std::vector<glm::ivec2> m_tracedCells;
    glm::vec2 m_startPoint;
    glm::vec2 m_endPoint;

    void raycast();
};
