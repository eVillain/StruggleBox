#include "DrawLinesNode.h"

#include "DefaultShaders.h"
#include "GUI.h"

DrawLinesNode::DrawLinesNode(GUI& gui)
	: m_gui(gui)
{
}

DrawLinesNode::~DrawLinesNode()
{
}

void DrawLinesNode::addLine(const glm::vec2 begin, const glm::vec2 end, const Color& beginColor, const Color& endColor)
{
	m_lineVerts.push_back({ glm::vec3(begin.x, begin.y, m_position.z), beginColor });
	m_lineVerts.push_back({ glm::vec3(end.x, end.y, m_position.z), endColor });
}

void DrawLinesNode::clear()
{
	m_lineVerts.clear();
}

void DrawLinesNode::draw(GUI& gui, const glm::vec3& parentPosition, const glm::vec2& parentScale)
{
	Node::draw(gui, parentPosition, parentScale);

	ColoredVertex3DData* buffer = m_gui.bufferColoredLines(m_lineVerts.size());
	memcpy(buffer, m_lineVerts.data(), m_lineVerts.size() * sizeof(ColoredVertex3DData));
}
