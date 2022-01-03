#pragma once

#include "InteractableNode.h"
#include <functional>
#include <string>

class SpriteNode;
class GUI;

class ButtonNode : public InteractableNode
{
public:
	ButtonNode(
		const GUI& gui,
		const std::string& defaultFrame,
		const std::string& disabledFrame,
		const std::string& highlightFrame,
		const std::string& pressedFrame);

	void setContentSize(const glm::vec2& contentSize) override;
	void setCallback(std::function<void(bool)> callback) { m_callback = callback; }
	void setToggleable(bool isToggleable) { m_toggleable = isToggleable; }
	void setToggled(bool toggled);
	void setEnabled(bool enabled);

	bool onPress(const glm::vec2&) override;
	bool onRelease(const glm::vec2& relativeCursorPosition) override;
	void onHighlight(bool inside, const glm::vec2&) override;

private:
	enum State
	{
		DEFAULT = 0,
		HIGHLIGHTED,
		PRESSED,
		DISABLED
	};

	const GUI& m_gui;
	SpriteNode* m_sprite;
	std::string m_defaultFrame;
	std::string m_disabledFrame;
	std::string m_highlightFrame;
	std::string m_pressedFrame;

	std::function<void(bool)> m_callback;
	State m_state;
	bool m_toggleable;
	bool m_toggled;

	void updateSprite();
	const std::string& getFrameForCurrentState() const;
};

