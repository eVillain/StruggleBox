#pragma once

#include "EditorScene.h"
#include "Skeleton.h"
#include "VoxelCache.h"

class SkeletonWindow;

class AnimationEditor : public EditorScene
{
public:
	AnimationEditor(
		Allocator& allocator,
		VoxelRenderer& renderer,
		RenderCore& renderCore,
		OSWindow& osWindow,
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
	void onFileLoad(const std::string& file) override;
	void onFileSave(const std::string& file) override;

private:
	SkeletonWindow* m_skeletonWindow;
	Skeleton m_skeleton;
	//InstancedTriangleMesh* m_mesh;
	VoxelCache m_voxels;

	void ShowEditor();
	void RemoveEditor();

	bool OnEvent(const InputEvent event, const float amount) override;
	bool OnMouse(const glm::ivec2& coord) override;

	void drawBones(
		const std::vector<InstanceTransformData3D>& data);

	bool isLoadingMesh() const;
};
