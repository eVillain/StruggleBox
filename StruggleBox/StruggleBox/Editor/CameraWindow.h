#pragma once

#include "WindowNode.h"
#include <functional>
#include <memory>

class Camera;
class ButtonNode;
class SpriteNode;
class LayoutNode;

class CameraWindow : public WindowNode
{
public:
	CameraWindow(
		const GUI& gui,
		Camera& camera);

private:
	static const glm::vec2 WINDOW_SIZE;

	Camera& m_camera;

	LayoutNode* m_layoutNode;

	void refresh();
};
