#include "TabWindow.h"

#include "Options.h"
#include "PathUtil.h"
#include "StringUtil.h"
#include "Log.h"

#include "GUI.h"
#include "WindowNode.h"
#include "LabelNode.h"
#include "SpriteNode.h"
#include "ButtonNode.h"
#include "SliderNode.h"
#include "TextInputNode.h"
#include "LayoutNode.h"

TabWindow::TabWindow(
	const GUI& gui,
	const glm::vec2 size,
	const std::string& headerFrame,
	const std::string& backgroundFrame,
	const std::string& title)
	: WindowNode(gui, size, headerFrame, backgroundFrame, title)
	, m_tabLayout(nullptr)
	, m_currentTab(0)
{
}

void TabWindow::setContentSize(const glm::vec2& contentSize)
{
	WindowNode::setContentSize(contentSize);
	m_tabLayout->setContentSize(glm::vec2(m_contentNode->getContentSize().x, 28.f));
	m_tabLayout->refresh();
}

void TabWindow::setTab(const uint32_t tab)
{
	m_currentTab = tab;

	setTabContent(tab);

	uint32_t tabID = 0;
	for (ButtonNode* tabButton : m_tabButtons)
	{
		tabButton->setToggled(tabID == m_currentTab);
		tabID++;
	}
}

void TabWindow::refreshTabs()
{
	if (m_tabLayout)
	{
		m_gui.destroyNodeAndChildren(m_tabLayout);
		m_tabLayout = nullptr;
		m_tabButtons.clear();
	}

	m_tabLayout = m_gui.createCustomNode<LayoutNode>();
	m_tabLayout->setContentSize(glm::vec2(m_contentNode->getContentSize().x, 28.f));
	m_tabLayout->setPosition(glm::vec3(0.f, m_contentNode->getContentSize().y - 30.f, 1.f));
	addChild(m_tabLayout);

	const std::vector<std::string>& tabs = getTabTitles();
	static const glm::vec2 TAB_BUTTON_SIZE = glm::vec2((m_tabLayout->getContentSize().x - ((tabs.size() + 1) * 2.f)) / tabs.size(), m_tabLayout->getContentSize().y - 4.f);
	uint32_t tabID = 0;
	for (const std::string& tab : tabs)
	{
		ButtonNode* tabButton = m_gui.createRectButton(TAB_BUTTON_SIZE, tab);
		tabButton->setToggleable(true);
		tabButton->setCallback([this, tabID](bool) {
			setTab(tabID);
			});
		m_tabLayout->addChild(tabButton);
		m_tabButtons.push_back(tabButton);
		tabID++;
	}

	m_tabLayout->refresh();
}
