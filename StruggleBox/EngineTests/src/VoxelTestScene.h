#pragma once

#include "GUIScene.h"
#include "Material.h"

class VoxelRenderer;
class RenderCore;
class SceneManager;

class VoxelTestScene : public GUIScene
{
public:
	VoxelTestScene(
		Allocator& allocator,
		VoxelRenderer& renderer3D,
		RenderCore& renderCore,
		SceneManager& sceneManager,
		Input& input,
		OSWindow& window,
		Options& options,
		StatTracker& statTracker);
	~VoxelTestScene();

	void Initialize() override;
	void Release() override;
	void Update(const double delta) override;
	void Draw() override;
	bool OnMouse(const glm::ivec2& coord) override;
	bool OnEvent(const InputEvent event, const float amount) override;

private:
	VoxelRenderer& m_renderer3D;
	RenderCore& m_renderCore;
	SceneManager& m_sceneManager;
	Material m_material;
	DrawDataID m_voxelInstancesDrawDataID;
	ShaderID m_voxelInstancesShaderID;

	DrawDataID m_simplerCubeDrawDataID;
	ShaderID m_simplerCubeShaderID;

	glm::vec2 m_inputMove;
	glm::vec2 m_inputRotate;

	bool m_renderTexturedCubes;
};
