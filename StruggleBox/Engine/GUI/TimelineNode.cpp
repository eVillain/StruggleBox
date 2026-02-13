#include "TimelineNode.h"

#include "GUI.h"
#include "DrawLinesNode.h"
#include "SpriteNode.h"
#include "LabelNode.h"
#include "MathUtils.h"
#include "StringUtil.h"

const float HEADER_HEIGHT = 26.f;
const float DEFAULT_PIXELS_PER_SECOND = 10.f;

TimelineNode::TimelineNode(GUI& gui, const glm::vec2 size)
	: m_gui(gui)
	, m_backgroundSprite(m_gui.createSpriteNode(GUI::RECT_BORDER))
	, m_headerSprite(m_gui.createSpriteNode(GUI::WINDOW_HEADER))
	, m_drawLinesNode(m_gui.createCustomNode<DrawLinesNode>(gui))
	, m_titleLabel(m_gui.createLabelNode("Timeline", GUI::FONT_TITLE, 22))
	, m_contentNode(m_gui.createNode())
	, m_draggingCursor(false)
	, m_currentTime(50.f)
	, m_zoom(1.f)
	, m_zoomMax(10.f)
	, m_zoomMin(0.1f)
{
	m_headerSprite->setEnable9Slice(true);
	m_titleLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_backgroundSprite->setEnable9Slice(true);
	m_drawLinesNode->setPositionZ(10.f);

	addChild(m_backgroundSprite);
	addChild(m_headerSprite);
	addChild(m_drawLinesNode);
	addChild(m_titleLabel);
	addChild(m_contentNode);

	setContentSize(size);
}

TimelineNode::~TimelineNode()
{
}

void TimelineNode::draw(GUI& gui, const glm::vec3& parentPosition, const glm::vec2& parentScale)
{
	Node::draw(gui, parentPosition, parentScale);

}

bool TimelineNode::onPress(const glm::vec2& relativeCursorPosition)
{
	const Rect2D contentBB = m_contentNode->getBoundingBox();
	if (contentBB.contains(relativeCursorPosition))
	{
		const float width = m_contentNode->getContentSize().x;
		const float visibleTime = width / (m_zoom * DEFAULT_PIXELS_PER_SECOND);
		const float positionRatio = (relativeCursorPosition.x - m_contentNode->getPosition().x) / width;
		m_currentTime = positionRatio * visibleTime;
		m_draggingCursor = true;
		refresh();
		return true;
	}
	return false;
}

bool TimelineNode::onRelease(const glm::vec2& relativeCursorPosition)
{
	const Rect2D contentBB = m_contentNode->getBoundingBox();
	if (contentBB.contains(relativeCursorPosition))
	{
		const float width = m_contentNode->getContentSize().x;
		const float visibleTime = width / (m_zoom * DEFAULT_PIXELS_PER_SECOND);
		const float positionRatio = (relativeCursorPosition.x - m_contentNode->getPosition().x) / width;
		m_currentTime = positionRatio * visibleTime;
		m_draggingCursor = false;
		refresh();
		return true;
	}
	return false;
}

bool TimelineNode::onScroll(const float amount)
{
	m_zoom += amount * 0.01f;
	m_zoom = MathUtils::Clamp(m_zoom, m_zoomMin, m_zoomMax);
	refresh();
	return true;
}

void TimelineNode::onHighlight(bool inside, const glm::vec2& relativeCursorPosition)
{
	if (!inside || !m_draggingCursor)
	{
		return;
	}
	const Rect2D contentBB = m_contentNode->getBoundingBox();
	if (contentBB.contains(relativeCursorPosition))
	{
		const float width = m_contentNode->getContentSize().x;
		const float visibleTime = width / (m_zoom * DEFAULT_PIXELS_PER_SECOND);
		const float positionRatio = (relativeCursorPosition.x - m_contentNode->getPosition().x) / width;
		m_currentTime = positionRatio * visibleTime;
		refresh();
	}
}

void TimelineNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);

	const uint32_t bodyHeight = m_contentSize.y - HEADER_HEIGHT;
	m_headerSprite->setContentSize(glm::vec2(m_contentSize.x, HEADER_HEIGHT));
	m_headerSprite->setPosition(glm::vec3(0.f, bodyHeight, 0.f));
	m_titleLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, m_contentSize.y - (HEADER_HEIGHT * 0.5f), 1.f));
	m_backgroundSprite->setContentSize(glm::vec2(m_contentSize.x, bodyHeight));
	m_contentNode->setContentSize(glm::vec2(m_contentSize.x, bodyHeight));
	m_contentNode->setPosition(glm::vec3(0.f, 0.f, 1.f));

	refresh();
}

void TimelineNode::refresh()
{
	m_drawLinesNode->clear();

	const float width = m_contentNode->getContentSize().x;
	const float height = m_contentNode->getContentSize().y;
	const float visibleTime = width / (m_zoom * DEFAULT_PIXELS_PER_SECOND); // default of 10px per second of time
	const float cursorPosX = (m_currentTime / visibleTime) * width;
	m_drawLinesNode->addLine(glm::vec2(cursorPosX, 1), glm::vec2(cursorPosX, height), COLOR_RED, COLOR_RED);

	const float rulerLineGap = m_zoom * 10.f; // default of 10px gaps between lines at 1.f zoom
	const uint32_t rulerLines = width / (rulerLineGap); 
	for (uint32_t x = 0; x < rulerLines; x++)
	{
		float rulerHeight = x % 10 == 0 ? 20.f : x % 5 == 0 ? 15.f : 10.f;
		float xPos = x * rulerLineGap;
		m_drawLinesNode->addLine(glm::vec2(xPos, height), glm::vec2(xPos, height - rulerHeight), COLOR_WHITE, COLOR_WHITE);
	}

	const std::string timeText = "Time: " + StringUtil::TimeFormatMinutesSecondsMS(m_currentTime);
	m_titleLabel->setText(timeText);
}