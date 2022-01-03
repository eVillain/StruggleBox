#pragma once

#include "ArenaOperators.h"
#include "Allocator.h"
#include "Node.h"
#include "InputListener.h"
#include <string>
#include <vector>

class Allocator;
class Renderer;
class Input;

class Node;
class ButtonNode;
class SpriteNode;
class LabelNode;
class WindowNode;
class TextInputNode;
class SliderNode;
class ScrollerNode;

class GUI
{
public:
	static const std::string FONT_DEFAULT;
	static const std::string FONT_TITLE;
	static const std::string BUTTON_DEFAULT;
	static const std::string BUTTON_DISABLED;
	static const std::string BUTTON_HIGHLIGHT;
	static const std::string BUTTON_PRESSED;
	static const std::string BUTTON_RECT_DEFAULT;
	static const std::string BUTTON_RECT_DISABLED;
	static const std::string BUTTON_RECT_HIGHLIGHT;
	static const std::string BUTTON_RECT_PRESSED;
	static const std::string RECT_DEFAULT;
	static const std::string RECT_HIGHLIGHT;
	static const std::string WINDOW_HEADER;
	static const std::string WINDOW_CONTENT;

	static const float DEFAULT_SLIDER_HEIGHT;

	GUI(Allocator& allocator, Renderer& renderer, Input& input);

	void initialize();
	void terminate();
	void draw();

	Node* createNode() const;
	SpriteNode* createSpriteNode(const std::string& spriteFrame) const;
	ButtonNode* createButtonNode(const std::string& defaultFrame, const std::string& disabledFrame, const std::string& highlightFrame, const std::string& pressedFrame) const;
	ButtonNode* createDefaultButton(const glm::vec2& size, const std::string& title = "") const;
	ButtonNode* createRectButton(const glm::vec2& size, const std::string& title = "") const;
	LabelNode* createLabelNode(const std::string& text, const std::string& font, const uint8_t fontHeight) const;
	TextInputNode* createTextInputNode(const std::string& placeholder, const std::string& defaultFrame, const std::string& highlightedFrame, const std::string& pressedFrame, const std::string& activeFrame) const;
	TextInputNode* createDefaultTextInput(const std::string& placeholder) const;
	SliderNode* createSliderNodeInt(const std::string& backgroundFrame, const std::string& defaultFrame, const std::string& highlightFrame, const std::string& pressedFrame, const std::string& activeFrame) const;
	SliderNode* createSliderNodeFloat(const std::string& backgroundFrame, const std::string& defaultFrame, const std::string& highlightFrame, const std::string& pressedFrame, const std::string& activeFrame) const;
	SliderNode* createDefaultSliderInt() const;
	SliderNode* createDefaultSliderFloat() const;
	ScrollerNode* createScrollerNode(const std::string& frame) const;
	WindowNode* createWindowNode(const glm::vec2 size, const std::string& headerFrame, const std::string& backgroundFrame, const std::string& title);
	WindowNode* createDefaultWindow(const glm::vec2& size, const std::string& title);

	void destroyNodeAndChildren(Node* node) const;
	void destroyNodes(const std::vector<Node*>& nodes) const;

	// ALL CONSTRUCTOR AGRUMENTS MUST BE TAKEN AS A REFERENCE IN THE CUSTOM NODE CONSTRUCTORS!!!
	template <typename T, typename... Dependencies>
	T* createCustomNode(Dependencies&... deps) const
	{
		return CUSTOM_NEW(T, m_allocator)(deps...);
	}

	// Node helpers
	void updateSprite(SpriteNode* sprite, const std::string& spriteFrame, bool updateSize = false) const;
	const glm::vec2 calculateTextSize(const std::string& text, const std::string& font, const uint8_t fontHeight) const;

	Node& getRoot() { return m_root; }

	bool OnEvent(const InputEvent event, const float amount);
	bool OnMouse(const glm::ivec2& coords);

private:
	Allocator& m_allocator;
	Renderer& m_renderer;
	Input& m_input;

	Node m_root;
	glm::vec2 m_mouseCoord;

	void onMouseRecursive(Node* node, const glm::vec3& parentPosition);
	bool onPressRecursive(Node* node, const glm::vec3& parentPosition);
	bool onReleaseRecursive(Node* node, const glm::vec3& parentPosition);
	bool onScrollRecursive(Node* node, const glm::vec3& parentPosition, const float amount);
};

