#pragma once

#include "Node.h"
#include "RenderCore.h"

class GUI;

class DrawLinesNode : public Node
{
public:
	DrawLinesNode(GUI& gui);
	~DrawLinesNode();

	void addLine(const glm::vec2 begin, const glm::vec2 end, const Color& beginColor, const Color& endColor);
	void clear();

	void draw(GUI& gui, const glm::vec3& parentPosition, const glm::vec2& parentScale) override;

private:
	GUI& m_gui;

	std::vector<ColoredVertex3DData> m_lineVerts;
};
