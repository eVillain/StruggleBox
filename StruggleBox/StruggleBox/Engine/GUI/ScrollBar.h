#pragma once

#include "InteractableNode.h"
#include <functional>
#include <string>

class ButtonNode;
class SpriteNode;

class ScrollBar : public InteractableNode
{
public:
	ScrollBar(const GUI& gui, const std::string& trackFrame, const std::string& upFrame, const std::string& downFrame);
	~ScrollBar();

	void setValueExtents(size_t minValue, size_t maxValue);
	void setValue(size_t value);

	void setContentSize(const glm::vec2& contentSize) override;

	bool onPress(const glm::vec2& relativeCursorPosition) override;
	bool onRelease(const glm::vec2& relativeCursorPosition) override;
	bool onScroll(const float amount) override;
	void onHighlight(bool inside, const glm::vec2& relativeCursorPosition) override;

	void onScrollUpButton(bool pressed);
	void onScrollDownButton(bool pressed);
	void onScrollBoxButton(bool pressed);

	void setCallback(std::function<void(size_t)> callBack) { m_callBack = callBack; }
private:
	const GUI& m_gui;
	SpriteNode* m_scrollBarTrack;
	ButtonNode* m_scrollUpButton;
	ButtonNode* m_scrollDownButton;
	ButtonNode* m_scrollBox;

	size_t m_minValue;
	size_t m_maxValue;
	size_t m_value;

	bool m_draggingBox;

	std::function<void(size_t)> m_callBack;
	void refresh();
};
