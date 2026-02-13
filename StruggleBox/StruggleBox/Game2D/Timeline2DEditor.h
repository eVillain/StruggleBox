#pragma once

#include "GUIScene.h"

class ButtonNode;
class TimelineNode;

class Timeline2DEditor : public GUIScene
{
public:
	Timeline2DEditor(Allocator& allocator, RenderCore& renderCore, Input& input, OSWindow& window, Options& options, StatTracker& statTracker);
	~Timeline2DEditor();

	void Initialize() override;
	void Update(const double delta) override;
	void Draw() override;

private:
	TimelineNode* m_timeline;

	ButtonNode* createMenuButton(const std::string& title);
};
