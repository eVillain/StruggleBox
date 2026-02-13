#pragma once

#include "ArenaOperators.h"
#include "Allocator.h"
#include "Node.h"
#include "InputListener.h"
#include "RenderCore.h"
#include <string>
#include <vector>

class RendererCore;
class Input;
class OSWindow;
class Node;
class ButtonNode;
class SpriteNode;
class LabelNode;
class WindowNode;
class TextInputNode;
class SliderNode;
class ScrollerNode;
class ScrollBar;

class GUI
{
public:
	static const std::string FONT_DEFAULT;
	static const std::string FONT_TITLE;
	static const std::string FONT_SEGMENT;
	static const std::string FONT_MONOSPACE;
	static const std::string BUTTON_DEFAULT;
	static const std::string BUTTON_DISABLED;
	static const std::string BUTTON_HIGHLIGHT;
	static const std::string BUTTON_PRESSED;
	static const std::string BUTTON_RECT_DEFAULT;
	static const std::string BUTTON_RECT_DISABLED;
	static const std::string BUTTON_RECT_HIGHLIGHT;
	static const std::string BUTTON_RECT_PRESSED;
	static const std::string RECT_ACTIVE;
	static const std::string RECT_BORDER;
	static const std::string RECT_DEFAULT;
	static const std::string RECT_DISABLED;
	static const std::string RECT_HIGHLIGHT;
	static const std::string RECT_PRESSED;
	static const std::string RECT_ARROW_DEFAULT;
	static const std::string RECT_ROUNDED_DEFAULT;
	static const std::string SLIDER_DIAL_ACTIVE;
	static const std::string SLIDER_DIAL_DEFAULT;
	static const std::string SLIDER_DIAL_HIGHLIGHT;
	static const std::string SLIDER_DIAL_PRESSED;
	static const std::string SLIDER_RAIL;
	static const std::string WINDOW_HEADER;
	static const std::string WINDOW_CONTENT;
	static const std::string DEFAULT_PLIST;
	static const float DEFAULT_SLIDER_HEIGHT;

	GUI(Allocator& allocator, RenderCore& renderCore, Input& input, OSWindow& window);

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
	ScrollBar* createDefaultScrollBar() const;
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

	TexturedVertex3DData* bufferTexturedTriangles(const size_t count, const TextureID textureID);
	TexturedVertex3DData* bufferTextTriangles(const size_t count, const TextureID textureID);
	ColoredVertex3DData* bufferColoredLines(const size_t count);

	RenderCore& getRenderCore() { return m_renderCore; }
	Node& getRoot() { return m_root; }

	bool OnEvent(const InputEvent event, const float amount);
	bool OnMouse(const glm::ivec2& coords);

private:
	Allocator& m_allocator;
	RenderCore& m_renderCore;
	Input& m_input;
	OSWindow& m_window;

	Node m_root;
	glm::vec2 m_mouseCoord;
	DrawDataID m_texturedVertsDrawDataID;
	DrawDataID m_textVertsDrawDataID;
	DrawDataID m_coloredLinesDrawDataID;

	ShaderID m_texturedVertsShaderID;
	ShaderID m_textVertsShaderID;
	ShaderID m_coloredLinesShaderID;
	std::map<TextureID, TempVertBuffer> m_texturedTriVertsBuffers;
	std::map<TextureID, TempVertBuffer> m_textTriVertsBuffers;
	TempVertBuffer m_lineVertsBuffer;

	void onMouseMoveRecursive(Node* node, const glm::vec3& parentPosition);
	bool onPressRecursive(Node* node, const glm::vec3& parentPosition);
	bool onReleaseRecursive(Node* node, const glm::vec3& parentPosition);
	bool onScrollRecursive(Node* node, const glm::vec3& parentPosition, const float amount);
};

