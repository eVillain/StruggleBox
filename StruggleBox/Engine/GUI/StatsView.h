#pragma once

#include "Node.h"

class GUI;
class StatTracker;
class LabelNode;

class StatsView : public Node
{
public:
	StatsView(GUI& gui, StatTracker& statTracker);
	~StatsView();

	void refresh();

private:
	GUI& m_gui;
	StatTracker& m_statTracker;

	std::vector<LabelNode*> m_labels;
};
