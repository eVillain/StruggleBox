#pragma once

#include "GUIScene.h"
#include "Material.h"

class Renderer2D;
class Renderer3DDeferred;
class RenderCore;
class SceneManager;

class RenderPBRTestScene : public GUIScene
{
public:
	RenderPBRTestScene(
		Allocator& allocator,
		Renderer2D& renderer2D,
		Renderer3DDeferred& renderer3D,
		RenderCore& renderCore,
		Input& input,
		SceneManager& sceneManager,
		OSWindow& window,
		Options& options,
		StatTracker& statTracker);
	~RenderPBRTestScene();

	void Initialize() override;
	void Release() override;
	void Update(const double delta) override;
	void Draw() override;
	bool OnMouse(const glm::ivec2& coord) override;
	bool OnEvent(const InputEvent event, const float amount) override;

private:

	Renderer3DDeferred& m_renderer3D;
	RenderCore& m_renderCore;
	SceneManager& m_sceneManager;
	Material m_material;
	glm::vec2 m_inputMove;
	glm::vec2 m_inputRotate;
};
