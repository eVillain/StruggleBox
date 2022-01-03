#pragma once

#include "Node.h"

class InteractableNode : public Node
{
public:
	virtual bool onPress(const glm::vec2& relativeCursorPosition) = 0;
	virtual bool onRelease(const glm::vec2& relativeCursorPosition) = 0;
	virtual	void onHighlight(bool inside, const glm::vec2& relativeCursorPosition) = 0;
};

