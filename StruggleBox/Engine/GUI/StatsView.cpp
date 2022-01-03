#include "StatsView.h"

#include "GUI.h"
#include "StatTracker.h"
#include "LabelNode.h"
#include "StringUtil.h"

StatsView::StatsView(GUI& gui, StatTracker& statTracker)
	: m_gui(gui)
	, m_statTracker(statTracker)
{
}

StatsView::~StatsView()
{
}

void StatsView::refresh()
{
	const std::map<std::string, float>& floatStats = m_statTracker.getFloatStats();
	const std::map<std::string, int32_t>& intStats = m_statTracker.getIntStats();

	size_t currentLabel = 0;
	float posY = m_contentSize.y - 14.f;
	for (const auto& pair : floatStats)
	{
		if (m_labels.size() <= currentLabel)
		{
			LabelNode* label = m_gui.createLabelNode(pair.first + " " + StringUtil::FloatToString(pair.second), GUI::FONT_DEFAULT, 12);
			label->setPosition(glm::vec3(2.f, posY, 1.f));
			addChild(label);
			m_labels.push_back(label);
		}
		else
		{
			LabelNode* label = m_labels.at(currentLabel);
			label->setText(pair.first + " " + StringUtil::FloatToString(pair.second));
		}
		posY -= 14.f;
		currentLabel++;
	}
	for (const auto& pair : intStats)
	{
		if (m_labels.size() <= currentLabel)
		{
			LabelNode* label = m_gui.createLabelNode(pair.first + " " + StringUtil::IntToString(pair.second), GUI::FONT_DEFAULT, 12);
			label->setPosition(glm::vec3(2.f, posY, 1.f));
			addChild(label);
			m_labels.push_back(label);
		}
		else
		{
			LabelNode* label = m_labels.at(currentLabel);
			label->setText(pair.first + " " + StringUtil::IntToString(pair.second));
		}
		posY -= 14.f;
		currentLabel++;
	}
}
