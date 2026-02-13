#pragma once

#include "Node.h"

class DrawLinesNode;
class GUI;
class LabelNode;
class LayoutNode;
class Options;
class StatTracker;

class StatsDisplay : public Node
{
public:
	StatsDisplay(GUI& gui, StatTracker& statTracker, Options& options);

	void update();

private:
	GUI& m_gui;
	StatTracker& m_statTracker;
	Options& m_options;
	LayoutNode* m_layoutNode;
	DrawLinesNode* m_drawLinesNode;
	std::vector<LabelNode*> m_textLabels;

	LabelNode* getLabelAtIndex(const size_t index, bool addToLayout);
};
