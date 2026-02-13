#include "GUI.h"

#include "DefaultShaders.h"
#include "RenderCore.h"
#include "FileUtil.h"
#include "OSWindow.h"
#include "TextureAtlas.h"
#include "TextureAtlasCache.h"
#include "TextureCache.h"
#include "TextAtlas.h"
#include "SpriteNode.h"
#include "InteractableNode.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "ScrollBar.h"
#include "ScrollerNode.h"
#include "SliderNode.h"
#include "TextInputNode.h"
#include "WindowNode.h"
#include "Log.h"
#include "Input.h"
#include "MathUtils.h"
#include <algorithm>

const std::string GUI::FONT_DEFAULT = "Aldrich-Regular.ttf";
const std::string GUI::FONT_TITLE = "Orbitron-Regular.ttf";
const std::string GUI::FONT_SEGMENT = "Segment14.otf";
const std::string GUI::FONT_MONOSPACE = "Monospace.ttf";
const std::string GUI::BUTTON_DEFAULT = "Button-Default";
const std::string GUI::BUTTON_DISABLED = "Button-Disabled";
const std::string GUI::BUTTON_HIGHLIGHT = "Button-Highlight";
const std::string GUI::BUTTON_PRESSED = "Button-Pressed";
const std::string GUI::BUTTON_RECT_DEFAULT = "Button-Square-Default";
const std::string GUI::BUTTON_RECT_DISABLED = "Button-Square-Disabled";
const std::string GUI::BUTTON_RECT_HIGHLIGHT = "Button-Square-Highlight";
const std::string GUI::BUTTON_RECT_PRESSED = "Button-Square-Pressed";
const std::string GUI::RECT_ACTIVE = "Rect-Active";
const std::string GUI::RECT_BORDER = "Rect-Border";
const std::string GUI::RECT_DEFAULT = "Rect-Default";
const std::string GUI::RECT_DISABLED = "Rect-Disabled";
const std::string GUI::RECT_HIGHLIGHT = "Rect-Highlight";
const std::string GUI::RECT_PRESSED = "Rect-Pressed";
const std::string GUI::RECT_ARROW_DEFAULT = "Rect-Arrow-Default";
const std::string GUI::RECT_ROUNDED_DEFAULT = " Rect-Rounded-Default";
const std::string GUI::SLIDER_DIAL_ACTIVE = "Slider-Dial-Active";
const std::string GUI::SLIDER_DIAL_DEFAULT = "Slider-Dial-Default";
const std::string GUI::SLIDER_DIAL_HIGHLIGHT = "Slider-Dial-Highlight";
const std::string GUI::SLIDER_DIAL_PRESSED = "Slider-Dial-Pressed";
const std::string GUI::SLIDER_RAIL = "Slider-Rail";
const std::string GUI::WINDOW_HEADER = "Window-Header";
const std::string GUI::WINDOW_CONTENT = "Window-Content";
const std::string GUI::DEFAULT_PLIST = "Data/GFX/DrudgeUI.plist";
const float GUI::DEFAULT_SLIDER_HEIGHT = 24.f;

GUI::GUI(Allocator& allocator, RenderCore& renderCore, Input& input, OSWindow& window)
	: m_allocator(allocator)
	, m_renderCore(renderCore)
	, m_input(input)
	, m_window(window)
	, m_mouseCoord(glm::vec2(0.f, 0.f))
	, m_texturedVertsDrawDataID(0)
	, m_texturedVertsShaderID(0)
	, m_textVertsDrawDataID(0)
	, m_textVertsShaderID(0)
	, m_coloredLinesDrawDataID(0)
	, m_coloredLinesShaderID(0)
	, m_lineVertsBuffer()
{
}

void GUI::initialize()
{
	const TextureAtlasID atlasID = m_renderCore.getTextureAtlasID(FileUtil::GetPath() + DEFAULT_PLIST);
	if (atlasID == 0)
	{
		Log::Error("[GUI] failed to init, couldnt load GUI texture atlas");
	}

	m_texturedVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
	m_textVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
	m_coloredLinesDrawDataID = m_renderCore.createDrawData(ColoredVertexConfig);
	m_texturedVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, texturedFragmentShaderSource, "TexturedVertsShader");
	m_textVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, textFragmentShaderSource, "TextVertsShader");
	m_coloredLinesShaderID = m_renderCore.getShaderIDFromSource(coloredVertexShaderSource, coloredFragmentShaderSource, "ColoredVertsShader");
}

void GUI::terminate()
{
	m_renderCore.removeShader(m_texturedVertsShaderID);
	m_renderCore.removeShader(m_textVertsShaderID);
	m_renderCore.removeShader(m_coloredLinesShaderID);

	std::vector<Node*> children = m_root.getChildren();
	for (Node* child : children)
	{
		destroyNodeAndChildren(child);
	}
	m_root.removeAllChildren();

	const TextureAtlasID atlasID = m_renderCore.getTextureAtlasID(FileUtil::GetPath() + DEFAULT_PLIST);
	m_renderCore.removeTextureAtlas(atlasID);
}

void GUI::draw()
{
	static const glm::vec3 ROOT_POSITION = glm::vec3();
	static const glm::vec2 ROOT_SCALE = glm::vec2(1.f, 1.f);
	m_root.draw(*this, ROOT_POSITION, ROOT_SCALE);

	const glm::ivec2 windowSize = m_renderCore.getRenderResolution();
	const glm::mat4 projection = glm::ortho<float>(0.f, windowSize.x, 0.f, windowSize.y, ORTHO_NEARDEPTH, ORTHO_FARDEPTH);

	for (const auto& pair : m_texturedTriVertsBuffers)
	{
		const TextureID textureID = pair.first;
		const TempVertBuffer& buffer = pair.second;
		m_renderCore.draw(m_texturedVertsShaderID, textureID, m_texturedVertsDrawDataID, projection, DrawMode::Triangles, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
	}
	for (const auto& pair : m_textTriVertsBuffers)
	{
		const TextureID textureID = pair.first;
		const TempVertBuffer& buffer = pair.second;
		m_renderCore.draw(m_textVertsShaderID, textureID, m_textVertsDrawDataID, projection, DrawMode::Triangles, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
	}

	if (m_lineVertsBuffer.count)
	{
		m_renderCore.draw(m_coloredLinesShaderID, 0, m_coloredLinesDrawDataID, projection, DrawMode::Lines, m_lineVertsBuffer.data, 0, m_lineVertsBuffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
	}

	m_texturedTriVertsBuffers.clear();
	m_textTriVertsBuffers.clear();
	m_renderCore.clearTempVertBuffer(m_lineVertsBuffer);
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

	const TextureAtlasID atlasID = m_renderCore.getTextureAtlasIDForFrame(spriteFrame);
	if (atlasID != TextureAtlasCache::NO_ATLAS_ID)
	{
		const TextureAtlas* atlas = m_renderCore.getTextureAtlasByID(atlasID);
		textureID = atlas->getTextureID();
		frameRect = atlas->getRectForFrame(spriteFrame);
		frameTexRect = atlas->getTexRectForFrame(spriteFrame);
	}
	else
	{
		textureID = m_renderCore.getTextureID(spriteFrame, true);
		if (textureID == TextureCache::NO_TEXTURE_ID)
		{
			Log::Error("[GUI] Couldnt load atlas or texture for frame: %s", spriteFrame.c_str());
			return nullptr;
		}
		const Texture2D* texture = m_renderCore.getTextureByID(textureID);
		frameRect = Rect2D(0.f, 0.f, texture->getWidth(), texture->getHeight());
		frameTexRect = Rect2D(0.f, 0.f, 1.f, 1.f);
	}

	SpriteNode* spriteNode = CUSTOM_NEW(SpriteNode, m_allocator)(textureID, frameTexRect);
	spriteNode->setOriginalSize(frameRect.size());
	spriteNode->setContentSize(frameRect.size());
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
		LabelNode* buttonLabel = createLabelNode(title, GUI::FONT_TITLE, uint8_t(size.y / 2));
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
	SliderNode* slider = createSliderNodeInt(GUI::SLIDER_RAIL, GUI::SLIDER_DIAL_DEFAULT, GUI::SLIDER_DIAL_HIGHLIGHT, GUI::SLIDER_DIAL_PRESSED, GUI::SLIDER_DIAL_ACTIVE);
	return slider;
}

SliderNode* GUI::createDefaultSliderFloat() const
{
	SliderNode* slider = createSliderNodeFloat(GUI::SLIDER_RAIL, GUI::SLIDER_DIAL_DEFAULT, GUI::SLIDER_DIAL_HIGHLIGHT, GUI::SLIDER_DIAL_PRESSED, GUI::SLIDER_DIAL_ACTIVE);
	return slider;
}

ScrollerNode* GUI::createScrollerNode(const std::string& frame) const
{
	ScrollerNode* scroller = CUSTOM_NEW(ScrollerNode, m_allocator)(*this, frame);
	return scroller;
}

ScrollBar* GUI::createDefaultScrollBar() const
{
	ScrollBar* scrollBar = CUSTOM_NEW(ScrollBar, m_allocator)(*this, GUI::RECT_DEFAULT, GUI::RECT_ARROW_DEFAULT, GUI::RECT_ARROW_DEFAULT);
	return scrollBar;
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
	const TextureAtlasID atlasID = m_renderCore.getTextureAtlasIDForFrame(spriteFrame);
	if (atlasID == TextureAtlasCache::NO_ATLAS_ID)
	{
		Log::Error("[GUI] Couldnt load texture atlas for frame: %s", spriteFrame.c_str());
		return;
	}
	const TextureAtlas* atlas = m_renderCore.getTextureAtlasByID(atlasID);
	const TextureID textureID = atlas->getTextureID();
	const Rect2D frameRect = atlas->getRectForFrame(spriteFrame);
	const Rect2D frameTexRect = atlas->getTexRectForFrame(spriteFrame);

	sprite->setTextureID(textureID);
	sprite->setTextureRect(frameTexRect);
	if (updateSize)
	{
		sprite->setContentSize(frameRect.size());
	}
}

const glm::vec2 GUI::calculateTextSize(const std::string& text, const std::string& font, const uint8_t fontHeight) const
{
	TextAtlasID atlasID = m_renderCore.getTextAtlasID(font, fontHeight);
	TextAtlas* atlas = m_renderCore.getTextAtlasByID(atlasID);
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

TexturedVertex3DData* GUI::bufferTexturedTriangles(const size_t count, const TextureID textureID)
{
	auto it = m_texturedTriVertsBuffers.find(textureID);
	if (it == m_texturedTriVertsBuffers.end())
	{
		const size_t bufferSize = MathUtils::Max(count, (size_t)4096);
		TempVertBuffer buffer;
		m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(buffer, bufferSize);
		m_texturedTriVertsBuffers[textureID] = buffer;
	}
	const TempVertBuffer& oldBuffer = m_texturedTriVertsBuffers.at(textureID);
	if (oldBuffer.count + count > oldBuffer.capacity)
	{
		const uint32_t nextBufferSize = MathUtils::round_up_to_power_of_2(oldBuffer.count + count);
		TempVertBuffer newBuffer;
		m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(newBuffer, nextBufferSize);
		memcpy(newBuffer.data, oldBuffer.data, oldBuffer.count * sizeof(TexturedVertex3DData));
		newBuffer.count = oldBuffer.count;
		m_texturedTriVertsBuffers[textureID] = newBuffer;
		Log::Warn("[GUI::bufferTexturedTriangles] Trying to buffer too many verts, increasing buffer size to %lu!", nextBufferSize);
	}
	TempVertBuffer& buffer = m_texturedTriVertsBuffers.at(textureID);
	TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)buffer.data;
	dataPtr += buffer.count;
	buffer.count += count;
	return dataPtr;
}

TexturedVertex3DData* GUI::bufferTextTriangles(const size_t count, const TextureID textureID)
{
	auto it = m_textTriVertsBuffers.find(textureID);
	if (it == m_textTriVertsBuffers.end())
	{
		const size_t bufferSize = MathUtils::Max(count, (size_t)4096);
		TempVertBuffer buffer;
		m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(buffer, bufferSize);
		m_textTriVertsBuffers[textureID] = buffer;
	}
	const TempVertBuffer& oldBuffer = m_textTriVertsBuffers.at(textureID);
	if (oldBuffer.count + count > oldBuffer.capacity)
	{
		const uint32_t nextBufferSize = MathUtils::round_up_to_power_of_2(oldBuffer.count + count);
		TempVertBuffer newBuffer;
		m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(newBuffer, nextBufferSize);
		memcpy(newBuffer.data, oldBuffer.data, oldBuffer.count * sizeof(TexturedVertex3DData));
		newBuffer.count = oldBuffer.count;
		m_textTriVertsBuffers[textureID] = newBuffer;
		Log::Warn("[GUI::bufferTextTriangles] Trying to buffer too many verts, increasing buffer size to %lu!", nextBufferSize);
	}
	TempVertBuffer& buffer = m_textTriVertsBuffers.at(textureID);
	TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)buffer.data;
	dataPtr += buffer.count;
	buffer.count += count;
	return dataPtr;
}

ColoredVertex3DData* GUI::bufferColoredLines(const size_t count)
{
	if (!m_lineVertsBuffer.data)
	{
		m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(m_lineVertsBuffer, MathUtils::Max(count, (size_t)16));
	}
	if (!m_lineVertsBuffer.data)
	{
		Log::Error("[GUI::bufferColoredLines] Out of memory!");
		return nullptr;
	}
	if (m_lineVertsBuffer.count + count > m_lineVertsBuffer.capacity)
	{
		const uint32_t nextBufferSize = MathUtils::round_up_to_power_of_2(m_lineVertsBuffer.count + count);
		TempVertBuffer newBuffer;
		m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(newBuffer, nextBufferSize);
		if (!newBuffer.data)
		{
			Log::Error("[GUI::bufferColoredLines] Out of memory!");
			return nullptr;
		}
		memcpy(newBuffer.data, m_lineVertsBuffer.data, m_lineVertsBuffer.count * sizeof(ColoredVertex3DData));
		newBuffer.count = m_lineVertsBuffer.count;
		m_lineVertsBuffer = newBuffer;
		Log::Warn("[GUI::bufferColoredLines] Trying to buffer too many verts, increasing buffer size to %lu!", nextBufferSize);
	}
	ColoredVertex3DData* dataPtr = (ColoredVertex3DData*)m_lineVertsBuffer.data;
	dataPtr += m_lineVertsBuffer.count;
	m_lineVertsBuffer.count += count;
	return dataPtr;
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
	const glm::ivec2 windowSize = glm::ivec2(m_window.GetWidth(), m_window.GetHeight());
	m_mouseCoord = glm::vec2(coords.x, windowSize.y - coords.y);

	onMouseMoveRecursive(&m_root, m_root.getPosition());

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

void GUI::onMouseMoveRecursive(Node* node, const glm::vec3& parentPosition)
{
	const Rect2D bb = node->getBoundingBox();
	const glm::vec3 relativePosition = parentPosition + glm::vec3(bb.x, bb.y, node->getPosition().z);
	auto& children = node->getChildren();

	for (Node* child : children)
	{
		onMouseMoveRecursive(child, relativePosition);

		InteractableNode* interactable = dynamic_cast<InteractableNode*>(child);
		if (!interactable)
		{
			continue;
		}
		Rect2D boundingBox = interactable->getBoundingBox();
		boundingBox.x += relativePosition.x;
		boundingBox.y += relativePosition.y;

		const bool contained = boundingBox.contains(m_mouseCoord);
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
		if (onPressRecursive(child, relativePosition))
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

		const bool contained = boundingBox.contains(m_mouseCoord);
		if (!contained)
		{
			continue;
		}
		if (interactable->onPress(m_mouseCoord - glm::vec2(boundingBox.x, boundingBox.y)))
		{
			return true;
		}
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

		InteractableNode* interactable = dynamic_cast<InteractableNode*>(child);
		if (!interactable)
		{
			continue;
		}

		Rect2D boundingBox = interactable->getBoundingBox();
		boundingBox.x += relativePosition.x;
		boundingBox.y += relativePosition.y;

		const bool contained = boundingBox.contains(m_mouseCoord);
		if (!contained)
		{
			continue;
		}

		if (interactable->onScroll(amount))
		{
			return true;
		}
	}
	return false;
}