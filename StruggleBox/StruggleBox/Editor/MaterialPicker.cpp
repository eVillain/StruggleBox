#include "MaterialPicker.h"

#include "SpriteNode.h"
#include "GUI.h"
#include "Renderer.h"
#include "PathUtil.h"
#include "Log.h"
#include "StringUtil.h"
#include "MathUtils.h"
#include <string>

MaterialPicker::MaterialPicker(
	const GUI& gui,
	Renderer& renderer,
	MaterialData& materials,
	const glm::vec2& size)
	: m_gui(gui)
	, m_renderer(renderer)
	, m_materials(materials)
	, m_materialSprite(nullptr)
	, m_materialSelectorSprite(nullptr)
	, m_currentID(1)
{
	Log::Debug("[MaterialPicker] constructor, instance at %p", this);
	setContentSize(size);
	setup();
	refresh();
}

bool MaterialPicker::onPress(const glm::vec2&)
{
	return false;
}

bool MaterialPicker::onRelease(const glm::vec2& relativeCursorPosition)
{
	const Rect2D boundingBox = m_materialSprite->getBoundingBox();
	const bool contained = boundingBox.Contains(relativeCursorPosition);
	if (!contained)
	{
		return false;
	}

	const glm::ivec2 coords = glm::ivec2(
		(relativeCursorPosition.x / boundingBox.w) * 16.f,
		(relativeCursorPosition.y / boundingBox.h) * 16.f);

	const uint8_t materialID = MaterialData::coordsToIndex(coords);

	setMaterialID(materialID);

	if (m_callback)
	{
		m_callback(materialID);
	}

	return true;
}

void MaterialPicker::onHighlight(bool inside, const glm::vec2&)
{

}

void MaterialPicker::setMaterialID(uint8_t materialID)
{
	m_currentID = materialID;

	refresh();
}

void MaterialPicker::setup()
{
	if (!m_materialSprite)
	{
		const std::string path = FileUtil::GetPath().append("Data/Materials/") + "Materials-Albedo.png";
		const TextureID albedoID = m_renderer.getTextureID(path, false);
		const Rect2D texRect = Rect2D(0.f, 0.f, 1.f, 1.f);
		m_materialSprite = m_gui.createCustomNode<SpriteNode>(albedoID, texRect);
		m_materialSprite->setContentSize(m_contentSize);
		m_materialSprite->setAnchorPoint(glm::vec2(0.f, 0.f));
		addChild(m_materialSprite);
	}
	if (!m_materialSelectorSprite)
	{
		m_materialSelectorSprite = m_gui.createSpriteNode("Rect-Border");
		m_materialSelectorSprite->setContentSize(m_contentSize / 16.f);
		m_materialSelectorSprite->setAnchorPoint(glm::vec2(0.f, 0.f));
		addChild(m_materialSelectorSprite);
	}
}

void MaterialPicker::remove()
{
	if (m_materialSprite)
	{
		removeChild(m_materialSprite);
		m_gui.destroyNodeAndChildren(m_materialSprite);
		m_materialSprite = nullptr;
	}
	if (m_materialSelectorSprite)
	{
		removeChild(m_materialSelectorSprite);
		m_gui.destroyNodeAndChildren(m_materialSelectorSprite);
		m_materialSelectorSprite = nullptr;
	}
}

void MaterialPicker::refresh()
{
	m_materialSprite->setPosition(glm::vec3(0.f, 0.f, 1.f));

	glm::vec2 selectorPos = glm::vec2(
		MaterialData::texOffsetX(m_currentID) * m_contentSize.x,
		MaterialData::texOffsetY(m_currentID) * m_contentSize.y);
	m_materialSelectorSprite->setPosition(glm::vec3(selectorPos.x, selectorPos.y, 2.f));
}
