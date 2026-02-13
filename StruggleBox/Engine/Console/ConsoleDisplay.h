#pragma once

#include "ConsoleDefs.h"
#include "Node.h"
#include <vector>
#include <string>
#include <memory>

class GUI;
class Options;
class LabelNode;
class TextInputNode;
class SpriteNode;
class LayoutNode;

class ConsoleDisplay : public Node
{
public:
	ConsoleDisplay(
		GUI& gui,
		Options& options);

	void clear();

private:
	GUI& m_gui;
	Options& m_options;

	SpriteNode* m_backgroundSprite;
	TextInputNode* m_textInput;
	LayoutNode* m_layoutNode;
	std::vector<LabelNode*> m_textLabels;
	std::vector<std::string> m_history;

	void Refresh();
	void Show();
	void OnTextInputUpdate(const std::string& input);
	void OnTextInputFinish(const std::string& input, const bool confirm);
};
