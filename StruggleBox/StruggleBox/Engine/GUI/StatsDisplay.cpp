#include "StatsDisplay.h"

#include "DrawLinesNode.h"
#include "GUI.h"
#include "LabelNode.h"
#include "LayoutNode.h"
#include "Options.h"
#include "StatTracker.h"
#include "StringUtil.h"

StatsDisplay::StatsDisplay(GUI& gui, StatTracker& statTracker, Options& options)
	: m_gui(gui)
	, m_statTracker(statTracker)
	, m_options(options)
	, m_layoutNode(gui.createCustomNode<LayoutNode>())
	, m_drawLinesNode(gui.createCustomNode<DrawLinesNode>(gui))
{
	const int windowWidth = m_options.getOption<int>("r_resolutionX");
	const int windowHeight = m_options.getOption<int>("r_resolutionY");
	setContentSize(glm::vec2(windowWidth, windowHeight));
	setPositionY(windowHeight - m_contentSize.y);

	m_layoutNode->setLayoutType(LayoutType::Column);
	m_layoutNode->setAlignmentV(LayoutAlignmentV::Top);
	m_layoutNode->setContentSize(m_contentSize);
	m_layoutNode->setPadding(8.f);

	addChild(m_layoutNode);
	addChild(m_drawLinesNode);
}

void StatsDisplay::update()
{
	const std::map<std::string, float>& floatStats = m_statTracker.getFloatStats();
	const std::map<std::string, int32_t>& intStats = m_statTracker.getIntStats();
	const std::map<std::string, StatBuffer>& bufferedStats = m_statTracker.getBufferedStats();

	size_t labelIndex = 0;
	for (const auto& pair : floatStats)
	{
		const std::string& name = pair.first;
		const float value = pair.second;
		const std::string text = StringUtil::Format("%s: %.3f", name.c_str(), value);
		LabelNode* label = getLabelAtIndex(labelIndex, true);
		label->setText(text);
		labelIndex++;
	}

	for (const auto& pair : intStats)
	{
		const std::string& name = pair.first;
		const int32_t value = pair.second;
		const std::string text = StringUtil::Format("%s: %i", name.c_str(), value);
		LabelNode* label = getLabelAtIndex(labelIndex, true);
		label->setText(text);
		labelIndex++;
	}

	m_layoutNode->refresh();

	m_drawLinesNode->clear();
	const glm::vec2 textSize = m_layoutNode->getUsedSize() + glm::vec2(0, 16.f);
	const glm::vec2 boxSize = glm::vec2(m_contentSize.x - 1.f, (m_contentSize.y - textSize.y) / bufferedStats.size());
	glm::vec2 nextBoxPos = glm::vec2(1.f, m_position.y + m_contentSize.y - (textSize.y + boxSize.y));
	for (const auto& pair : bufferedStats)
	{
		const std::string& name = pair.first;
		const StatBuffer& buffer = pair.second;

		const glm::vec2 boxBL = nextBoxPos;
		const glm::vec2 boxTR = boxBL + boxSize;
		m_drawLinesNode->addLine(boxBL, glm::vec2(boxBL.x, boxTR.y), COLOR_WHITE, COLOR_WHITE); // Left
		m_drawLinesNode->addLine(boxBL, glm::vec2(boxTR.x, boxBL.y), COLOR_WHITE, COLOR_WHITE); // Bottom
		m_drawLinesNode->addLine(boxTR, glm::vec2(boxTR.x, boxBL.y), COLOR_WHITE, COLOR_WHITE); // Right
		m_drawLinesNode->addLine(boxTR, glm::vec2(boxBL.x, boxTR.y), COLOR_WHITE, COLOR_WHITE); // Top

		float maxValue = 0.f;
		float averageValue = 0.f;
		for (size_t i = 0; i < buffer.maxValues; i++)
		{
			const float value = buffer.values[i];
			if (value > maxValue) maxValue = value;
			averageValue += value;
		}
		averageValue /= buffer.maxValues;

		const float yScale = boxSize.y / maxValue;
		const float xScale = boxSize.x / buffer.maxValues;
		const glm::vec2 averageLeft = boxBL + glm::vec2(0.f, averageValue * yScale);
		const glm::vec2 averageRight = boxBL + glm::vec2(boxSize.x, averageValue * yScale);
		m_drawLinesNode->addLine(averageLeft, averageRight, COLOR_BLUE, COLOR_BLUE);		
		glm::vec2 lastPos = glm::vec2();
		for (size_t i = 0; i < buffer.maxValues; i++)
		{
			const size_t index = (buffer.index + i) % buffer.maxValues;
			const float value = buffer.values[index];
			glm::vec2 pointPos = boxBL + glm::vec2(i * xScale, value * yScale);
			if (i > 0)
				m_drawLinesNode->addLine(lastPos, pointPos, COLOR_WHITE, COLOR_WHITE);
			lastPos = pointPos;
		}

		LabelNode* label = getLabelAtIndex(labelIndex++, false);
		const std::string labelText = StringUtil::Format("%s: %.3f, Average: %.3f, Max: %.3f", name.c_str(), buffer.values[buffer.index], averageValue, maxValue);
		label->setText(labelText);
		label->setPosition(glm::vec3(8.f + nextBoxPos.x - m_position.x, 8.f + nextBoxPos.y - m_position.y, 0.f));

	}
}

LabelNode* StatsDisplay::getLabelAtIndex(const size_t index, bool addToLayout)
{
	LabelNode* label = nullptr;
	if (m_textLabels.size() <= index)
	{
		label = m_gui.createLabelNode("", GUI::FONT_MONOSPACE, 24);
		m_textLabels.push_back(label);
		if (addToLayout)
		{
			m_layoutNode->addChild(label);
		}
		else
		{
			addChild(label);
		}
	}
	else
	{
		label = m_textLabels[index];
	}
	return label;
}
