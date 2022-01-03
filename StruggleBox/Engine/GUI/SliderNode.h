#pragma once

#include "InteractableNode.h"
#include <functional>
#include <string>

class SpriteNode;
class GUI;

class SliderNode : public InteractableNode
{
public:
	enum Type
	{
		INTEGER,
		FLOAT
	};

	SliderNode(
		const GUI& gui,
		const Type type,
		const std::string& backgroundFrame,
		const std::string& defaultFrame,
		const std::string& highlightFrame,
		const std::string& pressedFrame,
		const std::string& activeFrame);

	void setContentSize(const glm::vec2& contentSize) override;
	void setIntCallback(std::function<void(int32_t)> callback) { m_intCallback = callback; }
	void setFloatCallback(std::function<void(float)> callback) { m_floatCallback = callback; }

	void setIntValues(const int32_t min, const int32_t max, const int32_t value);
	void setFloatValues(const float min, const float max, const float value);

	bool onPress(const glm::vec2& relativeCursorPosition) override;
	bool onRelease(const glm::vec2& relativeCursorPosition) override;
	void onHighlight(bool inside, const glm::vec2& relativeCursorPosition) override;

private:
	enum State
	{
		DEFAULT = 0,
		HIGHLIGHTED,
		PRESSED,
		ACTIVE
	};

	const GUI& m_gui;
	const Type m_type;
	SpriteNode* m_backgroundSprite;
	SpriteNode* m_sliderSprite;
	std::string m_defaultFrame;
	std::string m_highlightFrame;
	std::string m_pressedFrame;
	std::string m_activeFrame;

	State m_state;
	glm::vec2 m_dragOffset;

	int32_t m_intMin;
	int32_t m_intMax;
	int32_t m_intValue;

	float m_floatMin;
	float m_floatMax;
	float m_floatValue;

	std::function<void(int32_t)> m_intCallback;
	std::function<void(float)> m_floatCallback;

	void updateSprite();
	const std::string& getFrameForCurrentState() const;
	void updateValue();
	void updateSliderPosition();
};

