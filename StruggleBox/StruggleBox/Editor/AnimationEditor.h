#pragma once

#include "EditorScene.h"
#include "Skeleton.h"

class SkeletonWindow;

class AnimationEditor : public EditorScene
{
public:
	AnimationEditor(
		Camera& camera,
		Allocator& allocator,
		Renderer& renderer,
		Options& options,
		Input& input,
		StatTracker& statTracker);
	virtual ~AnimationEditor();

	// Overridden from Scene
	void Initialize() override;
	void ReInitialize() override;
	void Pause() override;
	void Resume() override;
	void Update(const double delta) override;
	void Draw() override;

private:
	SkeletonWindow* m_skeletonWindow;
	Skeleton m_skeleton;

	void ShowEditor();
	void RemoveEditor();

	bool OnEvent(const InputEvent event, const float amount) override;
	bool OnMouse(const glm::ivec2& coord) override;

	void drawBones(
		const std::vector<InstanceTransformData>& data);
};
