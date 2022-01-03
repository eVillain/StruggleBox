#include "ConsoleDisplay.h"

#include "GUI.h"
#include "SpriteNode.h"
#include "TextInputNode.h"
#include "LabelNode.h"
#include "LayoutNode.h"

#include "Console.h"
#include "Options.h"
#include "FileUtil.h"
#include "Timer.h"

//#include <cstdarg>
//#include <iostream>
//#include <sstream>
//#include <fstream>

ConsoleDisplay::ConsoleDisplay(
	GUI& gui,
	Options& options)
	: m_gui(gui)
	, m_options(options)
	, m_backgroundSprite(nullptr)
	, m_textInput(nullptr)
	, m_layoutNode(nullptr)
{
	Show();
}

void ConsoleDisplay::clear()
{
	//m_scrollerNode->setContent({}, ScrollStrategy::KEEP_OFFSET);
	m_textInput->stopTextInput();
}

void ConsoleDisplay::Show()
{
	int winWidth = m_options.getOption<int>("r_resolutionX");
	int winHeight = m_options.getOption<int>("r_resolutionY");
	setContentSize(glm::vec2(winWidth, winHeight));
	m_backgroundSprite = m_gui.createSpriteNode(GUI::RECT_DEFAULT);
	m_backgroundSprite->setEnable9Slice(true);
	m_backgroundSprite->setContentSize(glm::vec2(m_contentSize.x, m_contentSize.y));
	addChild(m_backgroundSprite);

	std::string consoleInfo = "Console:  Ingenium v.";
	consoleInfo.append(m_options.getOption<std::string>("version"));
	m_textInput = m_gui.createDefaultTextInput(consoleInfo);
	m_textInput->setContentSize(glm::vec2(m_contentSize.x - 4, 26.f));
	m_textInput->setAnchorPoint(glm::vec2(0.f, 0.f));
	m_textInput->setPosition(glm::vec3(2.f, 2.f, 1.f));
	m_textInput->setUpdateCallback(std::bind(&ConsoleDisplay::OnTextInputUpdate, this, std::placeholders::_1));
	m_textInput->setFinishCallback(std::bind(&ConsoleDisplay::OnTextInputFinish, this, std::placeholders::_1, std::placeholders::_2));
	m_textInput->startTextInput();
	addChild(m_textInput);

	m_layoutNode = m_gui.createCustomNode<LayoutNode>();
	m_layoutNode->setLayoutType(LayoutType::Column);
	m_layoutNode->setContentSize(glm::vec2(m_contentSize.x - 4, m_contentSize.y - 32.f));
	m_layoutNode->setPosition(glm::vec3(0.f, 30.f, 1.f));
	addChild(m_layoutNode);

	Refresh();
}

void ConsoleDisplay::Refresh()
{
	const size_t msgCount = Console::_textLines.size();	
	for (int i = 0; i < msgCount; i++)
	{
		LabelNode* label;
		if (m_textLabels.size() <= i)
		{
			label = m_gui.createLabelNode(Console::_textLines[i].text, GUI::FONT_DEFAULT, 12);
			//label->setColor(Console::_textLines[i].color);
			m_textLabels.push_back(label);
			m_layoutNode->addChild(label);
		}
		else
		{
			label = m_textLabels.at(i);
			label->setText(Console::_textLines[i].text);
		}
	}
	m_layoutNode->refresh();
}

void ConsoleDisplay::OnTextInputUpdate(const std::string& input)
{
	if (input.empty())
	{
		return;
	}
}

void ConsoleDisplay::OnTextInputFinish(const std::string& input, const bool confirm)
{
	if (confirm)
	{
		m_history.push_back(input);
		Console::Process(input);
		Refresh();
	}

	m_textInput->setText("");
	//m_textInput->startTextInput();
}
