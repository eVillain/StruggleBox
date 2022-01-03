#pragma once

#include "InteractableNode.h"
#include "InputListener.h"
#include <functional>
#include <string>

class LabelNode;
class SpriteNode;
class GUI;
class Input;

class TextInputNode : public InteractableNode, public TextInputEventListener, public InputEventListener
{
public:
	TextInputNode(
		const GUI& gui,
		Input& input,
		const std::string& placeholder,
		const std::string& defaultFrame,
		const std::string& highlightFrame,
		const std::string& pressedFrame,
		const std::string& activeFrame);

	void setContentSize(const glm::vec2& contentSize) override;
	void setUpdateCallback(std::function<void(const std::string&)> callback) { m_updateCallback = callback; }
	void setFinishCallback(std::function<void(const std::string&, const bool)> callback) { m_finishCallback = callback; }

	bool onPress(const glm::vec2& relativeCursorPosition) override;
	bool onRelease(const glm::vec2& relativeCursorPosition) override;
	void onHighlight(bool inside, const glm::vec2& relativeCursorPosition) override;

	void startTextInput();
	void stopTextInput();
	void OnTextInput(const std::string& text) override;
	bool OnEvent(const InputEvent event, const float amount) override;

	void onFinish(const bool confirm);

	const std::string& getText() const;
	const void setText(const std::string& text);

private:
	enum State
	{
		DEFAULT = 0,
		HIGHLIGHTED,
		PRESSED,
		ACTIVE,
	};

	const GUI& m_gui;
	Input& m_input;

	SpriteNode* m_sprite;
	LabelNode* m_label;
	std::string m_defaultFrame;
	std::string m_highlightFrame;
	std::string m_pressedFrame;
	std::string m_activeFrame;

	std::function<void(const std::string&)> m_updateCallback;
	std::function<void(const std::string&, const bool)> m_finishCallback;
	State m_state;

	void updateSprite();
	const std::string& getFrameForCurrentState() const;
};

