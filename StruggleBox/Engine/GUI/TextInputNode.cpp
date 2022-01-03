#include "TextInputNode.h"

#include "LabelNode.h"
#include "SpriteNode.h"
#include "GUI.h"
#include "Input.h"

TextInputNode::TextInputNode(
	const GUI& gui,
	Input& input,
	const std::string& placeholder,
	const std::string& defaultFrame,
	const std::string& highlightFrame,
	const std::string& pressedFrame,
	const std::string& activeFrame)
	: m_gui(gui)
	, m_input(input)
	, m_sprite(nullptr)
	, m_label(nullptr)
	, m_defaultFrame(defaultFrame)
	, m_highlightFrame(highlightFrame)
	, m_pressedFrame(pressedFrame)
	, m_activeFrame(activeFrame)
	, m_updateCallback(nullptr)
	, m_finishCallback(nullptr)
	, m_state(State::DEFAULT)
{
	m_sprite = gui.createSpriteNode(defaultFrame);
	if (m_sprite)
	{
		m_sprite->setEnable9Slice(true);
		addChild(m_sprite);
		m_contentSize = m_sprite->getContentSize();
	}
	m_label = gui.createLabelNode(placeholder, GUI::FONT_DEFAULT, 16.f);
	if (m_label)
	{
		m_label->setAnchorPoint(glm::vec2(0.f, 0.5f));
		addChild(m_label);
	}
}

void TextInputNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);
	m_sprite->setContentSize(contentSize);
	m_label->setPosition(glm::vec3(2.0f, m_contentSize.y * 0.5f, 1.f));
}

void TextInputNode::startTextInput()
{
	if (m_state == ACTIVE)
	{
		return;
	}
	m_state = ACTIVE;
	updateSprite();
	m_input.StartTextInput(this);
	m_input.RegisterEventObserver(this);
}

void TextInputNode::stopTextInput()
{
	if (m_state != ACTIVE)
	{
		return;
	}
	m_state = State::DEFAULT;
	m_input.UnRegisterEventObserver(this);
	m_input.StopTextInput(this);
}

void TextInputNode::OnTextInput(const std::string& text)
{
	setText(text);
}

bool TextInputNode::OnEvent(const InputEvent event, const float amount)
{
	if (event == InputEvent::Start && amount < 0.f)
	{
		onFinish(true);
		return true;
	}
	else if (event == InputEvent::Back && amount < 0.f)
	{
		onFinish(false);
		return true;
	}
	return false;
}

void TextInputNode::onFinish(const bool confirm)
{
	stopTextInput();
	if (m_finishCallback)
	{
		m_finishCallback(m_label->getText(), confirm);
	}
}

const std::string& TextInputNode::getText() const
{
	return m_label->getText();
}

const void TextInputNode::setText(const std::string& text)
{
	m_label->setText(text);
	if (m_updateCallback)
	{
		m_updateCallback(text);
	}
}

bool TextInputNode::onPress(const glm::vec2& relativeCursorPosition)
{
	if (m_state == State::ACTIVE || m_state == State::PRESSED)
	{
		return false;
	}
	m_state = State::PRESSED;
	updateSprite();
	return true;
}

bool TextInputNode::onRelease(const glm::vec2& relativeCursorPosition)
{
	if (m_state != State::PRESSED)
	{
		return false;
	}

	startTextInput();

	return true;
}

void TextInputNode::onHighlight(bool inside, const glm::vec2& relativeCursorPosition)
{
	if (m_state == ACTIVE)
	{
		return;
	}
	if (m_state == State::PRESSED && inside)
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

void TextInputNode::updateSprite()
{
	m_gui.updateSprite(m_sprite, getFrameForCurrentState(), false);
}

const std::string& TextInputNode::getFrameForCurrentState() const
{
	if (m_state == State::DEFAULT)
	{
		return m_defaultFrame;
	}
	if (m_state == State::ACTIVE)
	{
		return m_activeFrame;
	}
	if (m_state == State::PRESSED)
	{
		return m_pressedFrame;
	}
	return m_highlightFrame;
}
