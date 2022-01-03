#include "GUI.h"

#include "Renderer.h"
#include "FileUtil.h"
#include "TextureAtlas.h"
#include "TextureAtlasCache.h"
#include "TextureCache.h"
#include "TextAtlas.h"
#include "SpriteNode.h"
#include "InteractableNode.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "WindowNode.h"
#include "TextInputNode.h"
#include "SliderNode.h"
#include "ScrollerNode.h"
#include "Log.h"
#include "Input.h"
#include <algorithm>

const std::string GUI::FONT_DEFAULT = "Aldrich-Regular.ttf";
const std::string GUI::FONT_TITLE = "Orbitron-Regular.ttf";
const std::string GUI::BUTTON_DEFAULT = "Button-Default";
const std::string GUI::BUTTON_DISABLED = "Button-Disabled";
const std::string GUI::BUTTON_HIGHLIGHT = "Button-Highlight";
const std::string GUI::BUTTON_PRESSED = "Button-Pressed";
const std::string GUI::BUTTON_RECT_DEFAULT = "Button-Square-Default";
const std::string GUI::BUTTON_RECT_DISABLED = "Button-Square-Disabled";
const std::string GUI::BUTTON_RECT_HIGHLIGHT = "Button-Square-Highlight";
const std::string GUI::BUTTON_RECT_PRESSED = "Button-Square-Pressed";
const std::string GUI::RECT_DEFAULT = "Rect-Default";
const std::string GUI::RECT_HIGHLIGHT = "Rect-Highlight";
const std::string GUI::WINDOW_HEADER = "Window-Header";
const std::string GUI::WINDOW_CONTENT = "Window-Content";

const float GUI::DEFAULT_SLIDER_HEIGHT = 24.f;


GUI::GUI(Allocator& allocator, Renderer& renderer, Input& input)
	: m_allocator(allocator)
	, m_renderer(renderer)
	, m_input(input)
	, m_mouseCoord(glm::vec2(0.f, 0.f))
{
}

void GUI::initialize()
{
	const TextureAtlasID atlasID = m_renderer.getTextureAtlasID(FileUtil::GetPath() + "Data/GFX/DrudgeUI.plist");
	if (atlasID == 0)
	{
		Log::Error("[GUI] failed to init, couldnt load GUI atlas");
	}

}

void GUI::terminate()
{
}

void GUI::draw()
{
	static const glm::vec3 ROOT_POSITION = glm::vec3();
	static const glm::vec2 ROOT_SCALE = glm::vec2(1.f, 1.f);
	m_root.draw(m_renderer, ROOT_POSITION, ROOT_SCALE);
}

Node* GUI::createNode() const
{
	Node* node = CUSTOM_NEW(Node, m_allocator)();
	return node;
}

SpriteNode* GUI::createSpriteNode(const std::string& spriteFrame) const
{
	TextureID textureID;
	Rect2D frameRect;
	Rect2D frameTexRect;

	const TextureAtlasID atlasID = m_renderer.getTextureAtlasIDForFrame(spriteFrame);
	if (atlasID != TextureAtlasCache::NO_ATLAS_ID)
	{
		const TextureAtlas* atlas = m_renderer.getTextureAtlasByID(atlasID);
		textureID = atlas->getTextureID();
		frameRect = atlas->getRectForFrame(spriteFrame);
		frameTexRect = atlas->getTexRectForFrame(spriteFrame);
	}
	else
	{
		textureID = m_renderer.getTextureID(spriteFrame, true);
		if (textureID == TextureCache::NO_TEXTURE_ID)
		{
			Log::Error("[GUI] Couldnt load atlas or texture for frame: %s", spriteFrame.c_str());
			return nullptr;
		}
		const Texture2D* texture = m_renderer.getTextureByID(textureID);
		frameRect = Rect2D(0.f, 0.f, texture->getWidth(), texture->getHeight());
		frameTexRect = Rect2D(0.f, 0.f, 1.f, 1.f);
	}

	SpriteNode* spriteNode = CUSTOM_NEW(SpriteNode, m_allocator)(textureID, frameTexRect);
	spriteNode->setOriginalSize(frameRect.Size());
	spriteNode->setContentSize(frameRect.Size());
	return spriteNode;
}

ButtonNode* GUI::createButtonNode(const std::string& defaultFrame, const std::string& disabledFrame, const std::string& highlightFrame, const std::string& pressedFrame) const
{
	ButtonNode* button = CUSTOM_NEW(ButtonNode, m_allocator)(*this, defaultFrame, disabledFrame, highlightFrame, pressedFrame);
	return button;
}

ButtonNode* GUI::createDefaultButton(const glm::vec2& size, const std::string& title /*= ""*/) const
{
	ButtonNode* button = createButtonNode(BUTTON_DEFAULT, BUTTON_DISABLED, BUTTON_HIGHLIGHT, BUTTON_PRESSED);
	button->setContentSize(size);
	if (!title.empty())
	{
		LabelNode* buttonLabel = createLabelNode(title, GUI::FONT_TITLE, uint8_t(size.y * 0.45f));
		buttonLabel->setPosition(glm::vec3(size.x * 0.5f, size.y * 0.5f, 1.f));
		buttonLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		button->addChild(buttonLabel);
	}
	return button;
}

ButtonNode* GUI::createRectButton(const glm::vec2& size, const std::string& title /*= ""*/) const
{
	ButtonNode* button = createButtonNode(BUTTON_RECT_DEFAULT, BUTTON_RECT_DISABLED, BUTTON_RECT_HIGHLIGHT, BUTTON_RECT_PRESSED);
	button->setContentSize(size);
	if (!title.empty())
	{
		LabelNode* buttonLabel = createLabelNode(title, GUI::FONT_TITLE, uint8_t(size.y * 0.5f));
		buttonLabel->setPosition(glm::vec3(size.x * 0.5f, size.y * 0.5f, 1.f));
		buttonLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		button->addChild(buttonLabel);
	}
	return button;
}

LabelNode* GUI::createLabelNode(const std::string& text, const std::string& font, const uint8_t fontHeight) const
{
	const std::string fontPath = FileUtil::GetPath() + "Data/Fonts/" + font;

	LabelNode* label = CUSTOM_NEW(LabelNode, m_allocator)(*this, text, fontPath, fontHeight);
	return label;
}

TextInputNode* GUI::createTextInputNode(const std::string& placeholder, const std::string& defaultFrame, const std::string& highlightedFrame, const std::string& pressedFrame, const std::string& activeFrame) const
{
	TextInputNode* inputNode = CUSTOM_NEW(TextInputNode, m_allocator)(*this, m_input, placeholder, defaultFrame, highlightedFrame, pressedFrame, activeFrame);
	return inputNode;
}

TextInputNode* GUI::createDefaultTextInput(const std::string& placeholder) const
{
	TextInputNode* inputNode = createTextInputNode(placeholder, GUI::RECT_DEFAULT, GUI::RECT_HIGHLIGHT, GUI::RECT_DEFAULT, GUI::RECT_HIGHLIGHT);
	return inputNode;
}

SliderNode* GUI::createSliderNodeInt(const std::string& backgroundFrame, const std::string& defaultFrame, const std::string& highlightFrame, const std::string& pressedFrame, const std::string& activeFrame) const
{
	SliderNode* sliderNode = CUSTOM_NEW(SliderNode, m_allocator)(*this, SliderNode::Type::INTEGER, backgroundFrame, defaultFrame, highlightFrame, pressedFrame, activeFrame);
	return sliderNode;
}

SliderNode* GUI::createSliderNodeFloat(const std::string& backgroundFrame, const std::string& defaultFrame, const std::string& highlightFrame, const std::string& pressedFrame, const std::string& activeFrame) const
{
	SliderNode* sliderNode = CUSTOM_NEW(SliderNode, m_allocator)(*this, SliderNode::Type::FLOAT, backgroundFrame, defaultFrame, highlightFrame, pressedFrame, activeFrame);
	return sliderNode;
}

SliderNode* GUI::createDefaultSliderInt() const
{
	SliderNode* slider = createSliderNodeInt(GUI::RECT_DEFAULT/*"Slider-Rail"*/, "Slider-Dial-Default", "Slider-Dial-Highlight", "Slider-Dial-Pressed", "Slider-Dial-Active");
	return slider;
}

SliderNode* GUI::createDefaultSliderFloat() const
{
	SliderNode* slider = createSliderNodeFloat(GUI::RECT_DEFAULT/*"Slider-Rail"*/, "Slider-Dial-Default", "Slider-Dial-Highlight", "Slider-Dial-Pressed", "Slider-Dial-Active");
	return slider;
}

ScrollerNode* GUI::createScrollerNode(const std::string& frame) const
{
	ScrollerNode* scroller = CUSTOM_NEW(ScrollerNode, m_allocator)(*this, frame);
	return scroller;
}

WindowNode* GUI::createWindowNode(const glm::vec2 size, const std::string& headerFrame, const std::string& backgroundFrame, const std::string& title)
{
	WindowNode* window = CUSTOM_NEW(WindowNode, m_allocator)(*this, size, headerFrame, backgroundFrame, title);
	return window;
}

WindowNode* GUI::createDefaultWindow(const glm::vec2& size, const std::string& title)
{
	WindowNode* window = createWindowNode(size, WINDOW_HEADER, WINDOW_CONTENT, title);
	return window;
}

void GUI::updateSprite(SpriteNode* sprite, const std::string& spriteFrame, bool updateSize /* = false */) const
{
	const TextureAtlasID atlasID = m_renderer.getTextureAtlasIDForFrame(spriteFrame);
	if (atlasID == TextureAtlasCache::NO_ATLAS_ID)
	{
		Log::Error("[GUI] Couldnt load atlas for frame: %s", spriteFrame.c_str());
		return;
	}
	const TextureAtlas* atlas = m_renderer.getTextureAtlasByID(atlasID);
	const TextureID textureID = atlas->getTextureID();
	const Rect2D frameRect = atlas->getRectForFrame(spriteFrame);
	const Rect2D frameTexRect = atlas->getTexRectForFrame(spriteFrame);

	sprite->setTextureID(textureID);
	sprite->setTextureRect(frameTexRect);
	if (updateSize)
	{
		sprite->setContentSize(frameRect.Size());
	}
}

const glm::vec2 GUI::calculateTextSize(const std::string& text, const std::string& font, const uint8_t fontHeight) const
{
	TextAtlasID atlasID = m_renderer.getTextAtlasID(font, fontHeight);
	TextAtlas* atlas = m_renderer.getTextAtlasByID(atlasID);
	if (!atlas)
	{
		return glm::vec2();
	}
	const Glyph* g = atlas->getGlyphs();

	glm::vec2 returnSize;

	// Pre-calculate sizes and positions we will need
	float height = 0;   // Holds total height of all lines
	float width = 0;    // Holds width of current line
	float widthMax = 0; // Holds width of widest line

	// Loop through all characters
	for (const char* p = text.c_str(); *p; p++)
	{
		// Skip newline character
		if (strncmp((const char*)p, "\n", 1) == 0)
		{
			// Size calculation
			height += fontHeight;
			widthMax = max(widthMax, width);
			width = 0;

			continue;
		}

		float h = g[*p].bh;

		// Size calculation
		width += g[*p].ax;
		if (height < h) height = h;

		// Skip glyphs that have no pixels
		if (!g[*p].bw || !h)
		{
			continue;
		}
	}

	widthMax = max(widthMax, width);
	return glm::vec2(widthMax, height);
}

bool GUI::OnEvent(const InputEvent event, const float amount)
{
	if (event == InputEvent::Shoot)
	{
		if (amount > 0.f)
		{
			return onPressRecursive(&m_root, m_root.getPosition());
		}
		else
		{
			return onReleaseRecursive(&m_root, m_root.getPosition());
		}
	}
	else if (event == InputEvent::Scroll_Y)
	{
		return onScrollRecursive(&m_root, m_root.getPosition(), amount);
	}

	return false;
}

bool GUI::OnMouse(const glm::ivec2& coords)
{
	const glm::ivec2 windowSize = m_renderer.getWindowSize();
	m_mouseCoord = glm::vec2(coords.x, windowSize.y - coords.y);

	onMouseRecursive(&m_root, m_root.getPosition());

	return false;
}

void GUI::destroyNodeAndChildren(Node* node) const
{
	auto& children = node->getChildren();
	for (Node* child : children)
	{
		destroyNodeAndChildren(child);
	}

	CUSTOM_DELETE(node, m_allocator);
}

void GUI::destroyNodes(const std::vector<Node*>& nodes) const
{
	for (Node* node : nodes)
	{
		CUSTOM_DELETE(node, m_allocator);
	}
}

void GUI::onMouseRecursive(Node* node, const glm::vec3& parentPosition)
{
	const Rect2D bb = node->getBoundingBox();
	const glm::vec3 relativePosition = parentPosition + glm::vec3(bb.x, bb.y, node->getPosition().z);
	auto& children = node->getChildren();

	for (Node* child : children)
	{
		onMouseRecursive(child, relativePosition);

		InteractableNode* interactable = dynamic_cast<InteractableNode*>(child);
		if (!interactable)
		{
			continue;
		}
		Rect2D boundingBox = interactable->getBoundingBox();
		boundingBox.x += relativePosition.x;
		boundingBox.y += relativePosition.y;

		const bool contained = boundingBox.Contains(m_mouseCoord);
		interactable->onHighlight(contained, m_mouseCoord - glm::vec2(boundingBox.x, boundingBox.y));
	}
}

bool GUI::onPressRecursive(Node* node, const glm::vec3& parentPosition)
{
	const Rect2D bb = node->getBoundingBox();
	const glm::vec3 relativePosition = parentPosition + glm::vec3(bb.x, bb.y, node->getPosition().z);
	auto& children = node->getChildren();

	for (Node* child : children)
	{
		onPressRecursive(child, relativePosition);

		InteractableNode* interactable = dynamic_cast<InteractableNode*>(child);
		if (!interactable)
		{
			continue;
		}
		Rect2D boundingBox = interactable->getBoundingBox();
		boundingBox.x += relativePosition.x;
		boundingBox.y += relativePosition.y;

		const bool contained = boundingBox.Contains(m_mouseCoord);
		if (!contained)
		{
			continue;
		}
		interactable->onPress(m_mouseCoord - glm::vec2(boundingBox.x, boundingBox.y));
		return true;
	}
	return false;
}

bool GUI::onReleaseRecursive(Node* node, const glm::vec3& parentPosition)
{
	const Rect2D bb = node->getBoundingBox();
	const glm::vec3 relativePosition = parentPosition + glm::vec3(bb.x, bb.y, node->getPosition().z);
	auto& children = node->getChildren();

	for (Node* child : children)
	{
		if (onReleaseRecursive(child, relativePosition))
		{
			return true;
		}

		InteractableNode* interactable = dynamic_cast<InteractableNode*>(child);
		if (!interactable)
		{
			continue;
		}
		Rect2D boundingBox = interactable->getBoundingBox();
		boundingBox.x += relativePosition.x;
		boundingBox.y += relativePosition.y;

		if (interactable->onRelease(m_mouseCoord - glm::vec2(boundingBox.x, boundingBox.y)))
		{
			return true;
		}
	}
	return false;
}

bool GUI::onScrollRecursive(Node* node, const glm::vec3& parentPosition, const float amount)
{
	const Rect2D bb = node->getBoundingBox();
	const glm::vec3 relativePosition = parentPosition + glm::vec3(bb.x, bb.y, node->getPosition().z);
	auto& children = node->getChildren();

	for (Node* child : children)
	{
		if (onScrollRecursive(child, relativePosition, amount))
		{
			return true;
		}

		ScrollerNode* scroller = dynamic_cast<ScrollerNode*>(child);
		if (!scroller)
		{
			continue;
		}

		Rect2D boundingBox = scroller->getBoundingBox();
		boundingBox.x += relativePosition.x;
		boundingBox.y += relativePosition.y;

		const bool contained = boundingBox.Contains(m_mouseCoord);
		if (!contained)
		{
			continue;
		}

		scroller->scroll(amount);
		return true;
	}
	return false;
}