#include "OptionsWindow.h"

#include "Options.h"

#include "GUI.h"
#include "WindowNode.h"
#include "ScrollerNode.h"
#include "ValueEditNode.h"

const glm::vec2 OptionsWindow::OPTIONS_WINDOW_SIZE = glm::vec2(560.f, 600.f);
const glm::vec2 OptionsWindow::OPTIONS_TAB_SIZE = glm::vec2(90.f, 30.f);

OptionsWindow::OptionsWindow(const GUI& gui, Options& options)
	: TabWindow(gui, OPTIONS_WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, "Options")
	, m_options(options)
	, m_scrollerNode(gui.createScrollerNode(GUI::RECT_DEFAULT))
{
	m_scrollerNode->setContentSize(glm::vec2(m_contentNode->getContentSize().x, m_contentNode->getContentSize().y - 32.f));
	addChild(m_scrollerNode);

	refreshTabs();

	setTab(0);
}

void OptionsWindow::setContentSize(const glm::vec2& size)
{
	TabWindow::setContentSize(size);
	m_scrollerNode->setContentSize(glm::vec2(m_contentNode->getContentSize().x, m_contentNode->getContentSize().y - 32.f));
}

const std::vector<std::string>& OptionsWindow::getTabTitles()
{
	static const std::vector<std::string> OPTIONS_TABS = {
		"General", "Audio", "Debug", "Editor", "Input", "Rendering"
	};

	return OPTIONS_TABS;
}

void OptionsWindow::setTabContent(const uint32_t tab)
{
	std::vector<Node*> nodes;

	const glm::vec2 OPTION_SIZE = glm::vec2(m_contentNode->getContentSize().x, 32.f);
	const std::map<const std::string, Attribute*>& allOptions = m_options.getAllOptions();
	for (auto it = allOptions.begin(); it != allOptions.end(); it++)
	{
		const std::string& name = it->first;
		if (getTabForOption(name) != (OptionsWindow::Tab)m_currentTab)
		{
			continue;
		}

		Attribute* attribute = it->second;
		if (attribute->IsType<bool>())
		{
			ValueEditNode<bool>* editNode = m_gui.createCustomNode<ValueEditNode<bool>>(m_gui);
			editNode->setContentSize(OPTION_SIZE);
			editNode->setValue(name, attribute->as<bool>(), false, true);
			nodes.push_back(editNode);
		}
		else if (attribute->IsType<int>())
		{
			ValueEditNode<int>* editNode = m_gui.createCustomNode<ValueEditNode<int>>(m_gui);
			editNode->setContentSize(OPTION_SIZE);
			editNode->setValue(name, attribute->as<int>(), 0, 255);
			nodes.push_back(editNode);
		}
		else if (attribute->IsType<float>())
		{
			ValueEditNode<float>* editNode = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
			editNode->setContentSize(OPTION_SIZE);
			editNode->setValue(name, attribute->as<float>(), 0.f, 100.f);
			nodes.push_back(editNode);
		}
		else if (attribute->IsType<std::string>())
		{
			ValueEditNode<std::string>* editNode = m_gui.createCustomNode<ValueEditNode<std::string>>(m_gui);
			editNode->setContentSize(OPTION_SIZE);
			editNode->setValue(name, attribute->as<std::string>(), "", "");
			nodes.push_back(editNode);
		}
	}

	m_scrollerNode->setContent(nodes, ScrollStrategy::SCROLL_TO_TOP);
}

OptionsWindow::Tab OptionsWindow::getTabForOption(const std::string& name) const
{
	static std::map<std::string, Tab> TAB_NAME_MAP = {
		{"a_", Tab::Audio},
		{"d_", Tab::Debug},
		{"e_", Tab::Editor},
		{"i_", Tab::Input},
		{"r_", Tab::Rendering},
	};

	const std::string namePrefix = name.substr(0, 2);
	if (TAB_NAME_MAP.find(namePrefix) != TAB_NAME_MAP.end())
	{
		return TAB_NAME_MAP.at(namePrefix);
	}
	return Tab::General;
}
