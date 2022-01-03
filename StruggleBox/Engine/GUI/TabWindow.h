#pragma once

#include "WindowNode.h"
#include <string>
#include <vector>

class GUI;
class ButtonNode;
class WindowNode;
class LayoutNode;

class TabWindow : public WindowNode
{
public:
	TabWindow(
		const GUI& gui,
		const glm::vec2 size,
		const std::string& headerFrame,
		const std::string& backgroundFrame,
		const std::string& title);

	void setContentSize(const glm::vec2& contentSize) override;

	virtual const std::vector<std::string>& getTabTitles() = 0;
	virtual void setTabContent(const uint32_t tab) = 0;

protected:
	LayoutNode* m_tabLayout;
	std::vector<ButtonNode*> m_tabButtons;

	uint32_t m_currentTab;

	void setTab(const uint32_t tab);
	void refreshTabs();
};
