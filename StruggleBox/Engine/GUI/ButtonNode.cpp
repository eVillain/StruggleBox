#include "ButtonNode.h"

#include "SpriteNode.h"
#include "GUI.h"
#include "Log.h"

ButtonNode::ButtonNode(
	const GUI& gui,
	const std::string& defaultFrame,
	const std::string& disabledFrame,
	const std::string& highlightFrame,
	const std::string& pressedFrame)
	: m_gui(gui)
	, m_sprite(nullptr)
	, m_defaultFrame(defaultFrame)
	, m_disabledFrame(disabledFrame)
	, m_highlightFrame(highlightFrame)
	, m_pressedFrame(pressedFrame)
	, m_callback(nullptr)
	, m_state(State::DEFAULT)
	, m_toggleable(false)
	, m_toggled(false)
{
	//Log::Debug("ButtonNode constructed at %p with GUI %p", this, &gui);
	m_sprite = gui.createSpriteNode(defaultFrame);
	if (m_sprite)
	{
		m_sprite->setEnable9Slice(true);
		m_contentSize = m_sprite->getContentSize();
		addChild(m_sprite);
	}
}

void ButtonNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);
	m_sprite->setContentSize(contentSize);
}

void ButtonNode::setToggled(bool toggled)
{
	m_toggled = toggled;
	m_state = m_toggled ? State::PRESSED : State::DEFAULT;
	updateSprite();
}

void ButtonNode::setEnabled(bool enabled)
{
	m_state = State::DISABLED;
	updateSprite();
}

bool ButtonNode::onPress(const glm::vec2&)
{
	if (m_state == State::DISABLED || m_state == State::PRESSED)
	{
		return false;
	}
	m_state = State::PRESSED;
	updateSprite();
	return true;
}

bool ButtonNode::onRelease(const glm::vec2& relativeCursorPosition)
{
	if (m_state != State::PRESSED)
	{
		return false;
	}

	const Rect2D boundingBox = m_sprite->getBoundingBox();
	const bool contained = boundingBox.Contains(relativeCursorPosition);
	if (!contained)
	{
		return false;
	}

	if (m_toggleable)
	{
		m_toggled = !m_toggled;
		m_state = m_toggled ? State::PRESSED : State::HIGHLIGHTED;
	}
	else 
	{
		m_state = HIGHLIGHTED;
	}

	updateSprite();

	if (m_callback)
	{
		m_callback(m_state == State::PRESSED);
	}

	return true;
}

void ButtonNode::onHighlight(bool inside, const glm::vec2&)
{
	if (m_state == State::PRESSED && inside)
	{
		return;
	}
	if (m_toggleable && m_toggled)
	{
		return;
	}

	const State prevState = m_state;
	m_state = inside ? State::HIGHLIGHTED : State::DEFAULT;
	if (m_state != prevState)
	{
		updateSprite();
	}
}

void ButtonNode::updateSprite()
{
	if (!m_sprite)
	{
		return;
	}
	m_gui.updateSprite(m_sprite, getFrameForCurrentState(), false);
}

const std::string& ButtonNode::getFrameForCurrentState() const
{
	if (m_state == State::DEFAULT)
	{
		return m_defaultFrame;
	}
	if (m_state == State::HIGHLIGHTED)
	{
		return m_highlightFrame;
	}
	if (m_state == State::PRESSED)
	{
		return m_pressedFrame;
	}
	return m_disabledFrame;
}
