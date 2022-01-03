#include "SliderNode.h"

#include "SpriteNode.h"
#include "GUI.h"

SliderNode::SliderNode(
	const GUI& gui,
	const Type type,
	const std::string& backgroundFrame,
	const std::string& defaultFrame,
	const std::string& highlightFrame,
	const std::string& pressedFrame,
	const std::string& activeFrame)
	: m_gui(gui)
	, m_type(type)
	, m_backgroundSprite(nullptr)
	, m_sliderSprite(nullptr)
	, m_defaultFrame(defaultFrame)
	, m_highlightFrame(highlightFrame)
	, m_pressedFrame(pressedFrame)
	, m_activeFrame(activeFrame)
	, m_intCallback(nullptr)
	, m_floatCallback(nullptr)
	, m_state(State::DEFAULT)
	, m_intMin(0)
	, m_intMax(100)
	, m_intValue(50)
	, m_floatMin(0.f)
	, m_floatMax(100.f)
	, m_floatValue(50.f)
{
	m_backgroundSprite = gui.createSpriteNode(backgroundFrame);
	if (m_backgroundSprite)
	{
		m_backgroundSprite->setEnable9Slice(true);
		m_contentSize = m_backgroundSprite->getContentSize();
		addChild(m_backgroundSprite);
	}
	m_sliderSprite = gui.createSpriteNode(defaultFrame);
	if (m_sliderSprite)
	{
		m_sliderSprite->setEnable9Slice(true);
		m_sliderSprite->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_sliderSprite->setContentSize(glm::vec2(m_sliderSprite->getContentSize().x, m_contentSize.y - 4.f));
		addChild(m_sliderSprite);
		updateSliderPosition();
	}
}

void SliderNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);
	m_backgroundSprite->setContentSize(contentSize);
	m_sliderSprite->setPositionY(contentSize.y * 0.5f);
	m_sliderSprite->setContentSize(glm::vec2(m_sliderSprite->getContentSize().x, m_contentSize.y - 4.f));
	updateSliderPosition();
}

void SliderNode::setIntValues(const int32_t min, const int32_t max, const int32_t value)
{
	assert(m_type != Type::FLOAT);
	m_intMin = min;
	m_intMax = max;
	m_intValue = value;
	updateSliderPosition();
}

void SliderNode::setFloatValues(const float min, const float max, const float value)
{
	assert(m_type != Type::INTEGER);
	m_floatMin = min;
	m_floatMax = max;
	m_floatValue = value;
	updateSliderPosition();
}

void SliderNode::onHighlight(bool inside, const glm::vec2& relativeCursorPosition)
{
	if (m_state == State::PRESSED)
	{
		m_sliderSprite->setPositionX(relativeCursorPosition.x + m_dragOffset.x);
		updateValue();
		updateSliderPosition();
		return;
	}

	Rect2D boundingBox = m_sliderSprite->getBoundingBox();

	const bool contained = boundingBox.Contains(relativeCursorPosition);
	const State prevState = m_state;
	m_state = contained ? State::HIGHLIGHTED : State::DEFAULT;
	if (m_state != prevState)
	{
		updateSprite();
	}
}

bool SliderNode::onPress(const glm::vec2& relativeCursorPosition)
{
	if (m_state == State::PRESSED)
	{
		return false;
	}

	Rect2D boundingBox = m_sliderSprite->getBoundingBox();
	const bool contained = boundingBox.Contains(relativeCursorPosition);
	if (!contained)
	{
		return false;
	}

	m_dragOffset = glm::vec2(m_sliderSprite->getPosition().x, m_sliderSprite->getPosition().y) - relativeCursorPosition;

	m_state = State::PRESSED;
	updateSprite();
	return true;
}

bool SliderNode::onRelease(const glm::vec2& relativeCursorPosition)
{
	if (m_state != State::PRESSED)
	{
		return false;
	}

	Rect2D boundingBox = m_sliderSprite->getBoundingBox();
	const bool contained = boundingBox.Contains(relativeCursorPosition);

	m_state = contained ? State::HIGHLIGHTED : State::DEFAULT;
	updateSprite();
	return true;
}

void SliderNode::updateSprite()
{
	if (!m_sliderSprite)
	{
		return;
	}
	m_gui.updateSprite(m_sliderSprite, getFrameForCurrentState(), false);
}

const std::string& SliderNode::getFrameForCurrentState() const
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
	return m_activeFrame;
}

void SliderNode::updateValue()
{
	const Rect2D bb = getBoundingBox();
	const float sliderRatio = std::min(1.f, std::max(0.f, m_sliderSprite->getPosition().x / bb.w));

	if (m_type == Type::FLOAT)
	{
		m_floatValue = m_floatMin + (sliderRatio * (m_floatMax - m_floatMin));
		if (m_floatCallback)
		{
			m_floatCallback(m_floatValue);
		}
	}
	else
	{
		m_intValue = m_intMin + (sliderRatio * (m_intMax - m_intMin));
		if (m_intCallback)
		{
			m_intCallback(m_intValue);
		}
	}
}

void SliderNode::updateSliderPosition()
{
	if (m_type == Type::FLOAT)
	{
		const float valueRatio = std::max(std::min((m_floatValue - m_floatMin) / (m_floatMax - m_floatMin), 1.f), 0.f);
		const float sliderPos = m_contentSize.x * valueRatio;
		m_sliderSprite->setPositionX(sliderPos);
	}
	else
	{
		const float valueRatio = std::max(std::min((float)(m_intValue - m_intMin) / float(m_intMax - m_intMin), 1.f), 0.f);
		const float sliderPos = m_contentSize.x * valueRatio;
		m_sliderSprite->setPositionX(sliderPos);
	}
}
