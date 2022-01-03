#include "WindowNode.h"

#include "LabelNode.h"
#include "SpriteNode.h"
#include "GUI.h"

const uint32_t WindowNode::HEADER_HEIGHT = 32;

WindowNode::WindowNode(
	const GUI& gui,
	const glm::vec2 size,
	const std::string& headerFrame,
	const std::string& backgroundFrame,
	const std::string& title)
	: m_gui(gui)
	, m_headerSprite(nullptr)
	, m_titleLabel(nullptr)
	, m_backgroundSprite(nullptr)
	, m_contentNode(nullptr)
{
	m_headerSprite = m_gui.createSpriteNode(headerFrame);
	m_headerSprite->setEnable9Slice(true);
	m_titleLabel = m_gui.createLabelNode(title, GUI::FONT_TITLE, 22);
	m_titleLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_backgroundSprite = m_gui.createSpriteNode(backgroundFrame);
	m_backgroundSprite->setEnable9Slice(true);
	m_contentNode = m_gui.createNode();

	addChild(m_headerSprite);
	addChild(m_titleLabel);
	addChild(m_backgroundSprite);
	addChild(m_contentNode);

	setContentSize(size);
}

void WindowNode::setContentSize(const glm::vec2& contentSize)
{
	Node::setContentSize(contentSize);

	const int bodyHeight = m_contentSize.y - HEADER_HEIGHT;
	m_headerSprite->setContentSize(glm::vec2(m_contentSize.x, HEADER_HEIGHT));
	m_headerSprite->setPosition(glm::vec3(0.f, bodyHeight, 0.f));
	m_titleLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, m_contentSize.y - (HEADER_HEIGHT * 0.5f), 1.f));
	m_backgroundSprite->setContentSize(glm::vec2(m_contentSize.x, bodyHeight));
	m_contentNode->setContentSize(glm::vec2(m_contentSize.x, bodyHeight));
	m_contentNode->setPosition(glm::vec3(0.f, 0.f, 1.f));
}
