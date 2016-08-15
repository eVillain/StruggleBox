#ifndef CONSOLE_DISPLAY_H
#define CONSOLE_DISPLAY_H

#include "ConsoleDefs.h"
#include <vector>
#include <string>
#include <memory>

class GUIDraw;
class Text;
class GUI;
class Options;
class Label;

class TextInput;

class ConsoleDisplay
{
public:
	ConsoleDisplay(
		std::shared_ptr<GUIDraw> guiDraw,
		std::shared_ptr<Text> text,
		std::shared_ptr<GUI> gui,
		std::shared_ptr<Options> options);

	void ToggleVisibility();
	bool isVisible() { return _visible; };

	void Draw();

private:
	std::shared_ptr<GUIDraw> _guiDraw;
	std::shared_ptr<Text> _text;
	std::shared_ptr<GUI> _gui;
	std::shared_ptr<Options> _options;


	std::vector<std::shared_ptr<Label>> _textLabels;
	std::shared_ptr<TextInput> _textInput;
	std::vector<std::string> _history;
	bool _visible;

	void Refresh();
	void Show();
	void Hide();
	void OnTextInput(const std::string& input);
};

#endif // !CONSOLE_DISPLAY_H

