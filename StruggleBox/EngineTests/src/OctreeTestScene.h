#pragma once

#include "GUIScene.h"
#include "Octree.h"

class Renderer2D;
class Renderer3D;
class RenderCore;
class SceneManager;

class OctreeTestScene : public GUIScene
{
public:
	OctreeTestScene(
		Allocator& allocator,
		Renderer2D& renderer2D,
		Renderer3D& renderer3D,
		RenderCore& renderCore,
		SceneManager& sceneManager,
		Input& input,
		OSWindow& window,
		Options& options,
		StatTracker& statTracker);
	~OctreeTestScene();

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

	DrawDataID m_cubeDrawDataID;
	Octree m_octree;

	void recursiveOctreeTraversal(const uint8_t level, const uint8_t bits, const glm::ivec3& localCoord, const glm::vec3& topLevelPos);

};
