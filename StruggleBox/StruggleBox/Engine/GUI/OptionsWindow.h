#pragma once

#include "Node.h"
#include "TabWindow.h"
#include <string>
#include <map>

class Attribute;
class GUI;
class Options;
class ScrollerNode;

class OptionsWindow : public TabWindow
{
public:
	enum class Tab {
		General,
		Audio,
		Debug,
		Editor,
		Input,
		Rendering
	};

	OptionsWindow(const GUI& gui, Options& options);

	void setContentSize(const glm::vec2& size) override;

	const std::vector<std::string>& getTabTitles() override;
	void setTabContent(const uint32_t tab) override;

private:
	static const glm::vec2 OPTIONS_WINDOW_SIZE;
	static const glm::vec2 OPTIONS_TAB_SIZE;

	Options& m_options;
	ScrollerNode* m_scrollerNode;

	std::map<std::string, Attribute*> m_currentData;

	OptionsWindow::Tab getTabForOption(const std::string& name) const;
};
