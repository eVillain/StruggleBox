#include "ConsoleDisplay.h"
//#include "GUIDraw.h"
#include "CommandProcessor.h"
#include "Options.h"
#include "FileUtil.h"
#include "Timer.h"
#include "Text.h"
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Console.h"

ConsoleDisplay::ConsoleDisplay(
	std::shared_ptr<GUIDraw> guiDraw,
	std::shared_ptr<Text> text,
	std::shared_ptr<GUI> gui,
	std::shared_ptr<Options> options) :
	_guiDraw(guiDraw),
	_text(text),
	_gui(gui),
	_options(options),
	_visible(false)
{
	CommandProcessor::AddCommand("toggleConsole", Command<>([&]() { ToggleVisibility(); }));
}


void ConsoleDisplay::ToggleVisibility()
{
	if (_visible) { Hide(); }
	else { Show(); }
}

void ConsoleDisplay::Draw()
{
	if (!_visible) return;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int winWidth = _options->getOption<int>("r_resolutionX");
	int winHeight = _options->getOption<int>("r_resolutionY");
	// Draw background box
	//_guiDraw->rect(
	//	glm::ivec2(-winWidth/2, 0 + 1),
	//	glm::ivec2(winWidth/2 - 1, winHeight / 2),
	//	CONSOLE_BG_DEPTH,
	//	LAColor((GLfloat)0.1f, (GLfloat)0.5f));
}


void ConsoleDisplay::Show()
{
	int winWidth = _options->getOption<int>("r_resolutionX");
	std::string consoleInfo = "Console:  Ingenium v.";
	consoleInfo.append(_options->getOption<std::string>("version"));
	//_textInput = _gui->CreateWidget<TextInput, GUIDraw, Text, Input>();
//	_textInput->GetTransform().SetPosition(glm::vec3(0, 11, CONSOLE_TEXT_DEPTH));
	//_textInput->setSize(glm::ivec2(winWidth - 1, 22));
	//_textInput->setDefaultText(consoleInfo);
	//TextInputBehavior* textInputBehavior = new TextInputBehaviorCallback<ConsoleDisplay>(this, &ConsoleDisplay::OnTextInput);
	//_textInput->setBehavior(textInputBehavior);
	//_textInput->StartTextInput();
	_visible = true;
	Refresh();
}

void ConsoleDisplay::Hide()
{
	for (auto label : _textLabels)
	{
		_text->DestroyLabel(label);
		label = nullptr;
	}
	_textLabels.clear();

	//_gui->DestroyWidget(_textInput);
	_textInput = nullptr;

	_visible = false;
}

void ConsoleDisplay::Refresh()
{
	if (!_visible) return;

	size_t msgCount = Console::_textLines.size();
	int winWidth = _options->getOption<int>("r_resolutionX");
	//    int winHeight = _locator->Get<Options>()->getOption<int>("r_resolutionY");
	//    int maxMessages = winHeight / CONSOLE_FONT_SIZE;
	double labelPosX = -(winWidth / 2) + 8;    // left edge of screen
	double labelPosY = 22 + (msgCount + 2)*CONSOLE_FONT_SIZE;

	// Move existing labels up
	for (int i = 0; i < msgCount; i++) {
		if (_textLabels.size() > i) {
			// Move text
			_textLabels[i]->getTransform().SetPosition(glm::vec3(labelPosX, labelPosY, CONSOLE_TEXT_DEPTH));
		}
		else {
			// Add line
			auto label = _text->CreateLabel(Console::_textLines[i].text);
			label->setFont(Fonts::FONT_PIXEL);
			label->setFontSize(CONSOLE_FONT_SIZE);
			label->setAlignment(Align_Left);
			label->setColor(Console::_textLines[i].color);
			label->getTransform().SetPosition(glm::vec3(labelPosX, labelPosY, CONSOLE_TEXT_DEPTH));
			_textLabels.push_back(label);
		}
		labelPosY -= CONSOLE_FONT_SIZE;
	}
}

void ConsoleDisplay::OnTextInput(const std::string& input)
{
	_history.push_back(input);
	Console::Process(input);
	Refresh();
	//_textInput->ClearText();
	//_textInput->StopTextInput();
	//_textInput->StartTextInput();
}
