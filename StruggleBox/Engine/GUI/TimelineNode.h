#pragma once

#include "InteractableNode.h"

class DrawLinesNode;
class LabelNode;
class SpriteNode;
class GUI;

class TimelineNode : public InteractableNode
{
public:
	TimelineNode(GUI& gui, const glm::vec2 size);
	~TimelineNode();

	void draw(GUI& gui, const glm::vec3& parentPosition, const glm::vec2& parentScale) override;

	bool onPress(const glm::vec2& relativeCursorPosition) override;
	bool onRelease(const glm::vec2& relativeCursorPosition) override;
	bool onScroll(const float amount) override;
	void onHighlight(bool inside, const glm::vec2& relativeCursorPosition) override;

	void setContentSize(const glm::vec2& contentSize) override;

	void refresh();

protected:
	GUI& m_gui;
	SpriteNode* m_backgroundSprite;
	SpriteNode* m_headerSprite;
	DrawLinesNode* m_drawLinesNode;
	LabelNode* m_titleLabel;
	Node* m_contentNode;

	bool m_draggingCursor;
	float m_currentTime;
	float m_zoom;
	float m_zoomMax;
	float m_zoomMin;
};
