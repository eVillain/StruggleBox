#include "ScrollBar.h"

#include "GUI.h"
#include "ButtonNode.h"
#include "SpriteNode.h"
#include "MathUtils.h"

const glm::vec2 BUTTON_SIZE = glm::vec2(24.f, 24.f);

ScrollBar::ScrollBar(const GUI& gui,
	const std::string& trackFrame,
	const std::string& upFrame,
	const std::string& downFrame)
	: m_gui(gui)
	, m_scrollBarTrack(gui.createSpriteNode(trackFrame))
	, m_scrollUpButton(gui.createRectButton(BUTTON_SIZE))
	, m_scrollDownButton(gui.createRectButton(BUTTON_SIZE))
	, m_scrollBox(gui.createRectButton(BUTTON_SIZE))
	, m_minValue(0)
	, m_maxValue(1)
	, m_value(0)
	, m_draggingBox(false)
	, m_callBack(nullptr)
{
	addChild(m_scrollBarTrack);
	addChild(m_scrollUpButton);
	addChild(m_scrollDownButton);
	m_scrollBarTrack->addChild(m_scrollBox);
	m_scrollBarTrack->setEnable9Slice(true);

	SpriteNode* upSprite = m_gui.createSpriteNode(upFrame);
	const Rect2D& texRect = upSprite->getTextureRect();
	Rect2D flippedRect = Rect2D(texRect.x, texRect.y + texRect.h, texRect.w, -texRect.h);
	upSprite->setTextureRect(flippedRect);
	upSprite->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	upSprite->setPosition(glm::vec3(BUTTON_SIZE.x / 2.f, BUTTON_SIZE.y / 2.f, 1.f));
	upSprite->setScale(glm::vec2(2.f, 2.f));
	m_scrollUpButton->addChild(upSprite);
	m_scrollUpButton->setCallback(std::bind(&ScrollBar::onScrollUpButton, this, std::placeholders::_1));
	SpriteNode* downSprite = m_gui.createSpriteNode(downFrame);
	downSprite->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	downSprite->setPosition(glm::vec3(BUTTON_SIZE.x / 2.f, BUTTON_SIZE.y / 2.f, 1.f));
	downSprite->setScale(glm::vec2(2.f, 2.f));
	m_scrollDownButton->addChild(downSprite);
	m_scrollDownButton->setCallback(std::bind(&ScrollBar::onScrollDownButton, this, std::placeholders::_1));

	m_scrollBox->setCallback(std::bind(&ScrollBar::onScrollBoxButton, this, std::placeholders::_1));
}

ScrollBar::~ScrollBar()
{
}

void ScrollBar::setValueExtents(size_t minValue, size_t maxValue)
{
	m_minValue = minValue;
	m_maxValue = MathUtils::Max(maxValue, (size_t)1);
	m_value = MathUtils::Clamp(m_value, m_minValue, m_maxValue);
	refresh();
}

void ScrollBar::setValue(size_t value)
{
	m_value = value;
	refresh();
}

void ScrollBar::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);
	m_scrollBarTrack->setContentSize(contentSize - glm::vec2(0.f, BUTTON_SIZE.y * 2.f));
	m_scrollBarTrack->setPositionY(BUTTON_SIZE.y);
	m_scrollUpButton->setPositionY(contentSize.y - BUTTON_SIZE.y);
	refresh();
}

bool ScrollBar::onPress(const glm::vec2& relativeCursorPosition)
{
	if (relativeCursorPosition.y > m_scrollBox->getPosition().y)
	{
		onScrollUpButton(true);
	}
	else if (relativeCursorPosition.y < m_scrollBox->getPosition().y)
	{
		onScrollDownButton(true);
	}

	return false;
}

bool ScrollBar::onRelease(const glm::vec2& relativeCursorPosition)
{
	if (m_draggingBox)
	{
		m_draggingBox = false;
		return true;
	}
	return false;
}

bool ScrollBar::onScroll(const float amount)
{
	return false;
}

void ScrollBar::onHighlight(bool inside, const glm::vec2& relativeCursorPosition)
{
	if (!m_draggingBox)
	{
		return;
	}
	const float slideRange = m_scrollBarTrack->getContentSize().y;
	const float posRatio = 1.f - MathUtils::Clamp(relativeCursorPosition.y / slideRange, 0.f, 1.f);
	const size_t newValue = (size_t)((float)(m_maxValue - m_minValue) * posRatio) + m_minValue;
	if (newValue != m_value)
	{
		if (m_callBack)
		{
			m_callBack(m_value);
		}
	}
	setValue(newValue);
}

void ScrollBar::onScrollUpButton(bool pressed)
{
	if (m_value == m_minValue)
	{
		return;
	}
	m_value--;
	if (m_callBack)
	{
		m_callBack(m_value);
	}
	refresh();
}

void ScrollBar::onScrollDownButton(bool pressed)
{
	if (m_value == m_maxValue)
	{
		return;
	}
	m_value++;
	if (m_callBack)
	{
		m_callBack(m_value);
	}
	refresh();
}

void ScrollBar::onScrollBoxButton(bool pressed)
{
	m_draggingBox = pressed;
}

void ScrollBar::refresh()
{
	const float range = m_scrollBarTrack->getContentSize().y;
	const size_t valueRange = MathUtils::Max(m_maxValue - m_minValue, (size_t)1);
	const float boxHeight = MathUtils::Max((range / valueRange), BUTTON_SIZE.y);
	m_scrollBox->setContentSize(glm::vec2(m_scrollBox->getContentSize().x, boxHeight));
	const float posRatio = MathUtils::Max(0.f, float(m_value - m_minValue) / float(m_maxValue - m_minValue));
	const float slideRange = range - boxHeight;
	m_scrollBox->setPositionY((slideRange - (slideRange * posRatio)));
}