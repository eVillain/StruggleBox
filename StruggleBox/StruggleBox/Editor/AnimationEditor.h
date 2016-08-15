#ifndef ANIMATION_EDITOR_H
#define ANIMATION_EDITOR_H

#include "EditorScene.h"
#include "SkeletonWindow.h"
#include "Skeleton.h"
#include "tb_select_item.h"

class AnimationEditor : public EditorScene
{
public:
	AnimationEditor(
		std::shared_ptr<TBGUI> gui,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<Options> options,
		std::shared_ptr<Input> input);
	virtual ~AnimationEditor();

	// Overridden from Scene
	void Initialize();
	void ReInitialize();
	void Release();
	void Pause();
	void Resume();
	void Update(const double delta);
	void Draw();

private:
	tb::TBGenericStringItemSource _file_menu_source;
	SkeletonWindow* _skeletonWindow;
	Skeleton _skeleton;

	void ShowEditor();
	void RemoveEditor();

	bool OnEvent(const std::string& theEvent,
		const float& amount);
	bool OnMouse(const glm::ivec2& coord);

	void drawBones(
		const std::vector<InstanceData>& data);
};

#endif // !ANIMATION_EDITOR_H

