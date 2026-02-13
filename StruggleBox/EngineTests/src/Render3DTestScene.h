#pragma once

#include "GUIScene.h"

class Renderer2D;
class Renderer3D;
class RenderCore;
class SceneManager;

class Render3DTestScene : public GUIScene
{
public:
	Render3DTestScene(
		Allocator& allocator,
		Renderer2D& renderer2D,
		Renderer3D& renderer3D,
		RenderCore& renderCore,
		SceneManager& sceneManager,
		Input& input,
		OSWindow& window,
		Options& options,
		StatTracker& statTracker);
	~Render3DTestScene();

	void Initialize() override;
	void Update(const double delta) override;
	void Draw() override;
	bool OnMouse(const glm::ivec2& coord) override;
	bool OnEvent(const InputEvent event, const float amount) override;


private:
	Renderer2D& m_renderer2D;
	Renderer3D& m_renderer3D;
	RenderCore& m_renderCore;
	SceneManager& m_sceneManager;

	glm::vec2 m_inputMove;
	glm::vec2 m_inputRotate;
};
