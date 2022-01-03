#include "MaterialsWindow.h"

#include "ButtonNode.h"
#include "LabelNode.h"
#include "SliderNode.h"
#include "SpriteNode.h"
#include "MaterialPicker.h"
#include "GUI.h"
#include "Renderer.h"
#include "PathUtil.h"
#include "Log.h"
#include "StringUtil.h"
#include "MathUtils.h"
#include <string>

const glm::vec2 MaterialsWindow::WINDOW_SIZE = glm::vec2(260.f, 600.f);

MaterialsWindow::MaterialsWindow(
	const GUI& gui,
	Renderer& renderer,
	MaterialData& materials)
	: WindowNode(gui, WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, "Materials")
	, m_renderer(renderer)
	, m_materials(materials)
	, m_materialLabel(nullptr)
	, m_materialSlider(nullptr)
	, m_materialSprite(nullptr)
	, m_materialSelectorSprite(nullptr)
	, m_pickerButton(nullptr)
	, m_materialPicker(nullptr)
	, m_colorRLabel(nullptr)
	, m_colorGLabel(nullptr)
	, m_colorBLabel(nullptr)
	, m_colorALabel(nullptr)
	, m_colorRSlider(nullptr)
	, m_colorGSlider(nullptr)
	, m_colorBSlider(nullptr)
	, m_colorASlider(nullptr)
	, m_roughnessLabel(nullptr)
	, m_metalnessLabel(nullptr)
	, m_emissivenessLabel(nullptr)
	, m_roughnessSlider(nullptr)
	, m_metalnessSlider(nullptr)
	, m_emissivenessSlider(nullptr)
	, m_colorCubeButton(nullptr)
	, m_sphereButton(nullptr)
	, m_arrayButton(nullptr)
	, m_sizeLabel(nullptr)
	, m_sizeSlider(nullptr)
	, m_currentID(1)
	, m_currentColor(RGBAColor(1.f, 1.f, 1.f, 1.f))
	, m_currentRoughness(1.f)
	, m_currentMetalness(0.f)
	, m_currentEmissiveness(0.f)
	, m_displayColorCube(false)
	, m_displaySphere(false)
	, m_displayArray(false)
	, m_displaySize(1.f)
{
	Log::Debug("[MaterialsWindow] constructor, instance at %p", this);

	refresh();
}

void MaterialsWindow::setMaterialID(uint8_t materialID)
{
	m_currentID = materialID;
	if (m_materialLabel)
	{
		m_materialLabel->setText("Current Material ID: " + std::to_string(m_currentID));
	}
	if (m_materialSprite)
	{
		Rect2D texRect = Rect2D(
			MaterialData::texOffsetX(m_currentID),
			MaterialData::texOffsetY(m_currentID),
			1.f / 16.f, 1.f / 16.f
		);
		m_materialSprite->setTextureRect(texRect);
	}
}

void MaterialsWindow::updateColorText()
{
	m_colorRLabel->setText("Red: " + StringUtil::FloatToString(m_currentColor.r));
	m_colorGLabel->setText("Green: " + StringUtil::FloatToString(m_currentColor.g));
	m_colorBLabel->setText("Blue: " + StringUtil::FloatToString(m_currentColor.b));
	m_colorALabel->setText("Alpha: " + StringUtil::FloatToString(m_currentColor.a));
	m_roughnessLabel->setText("Roughness: " + StringUtil::FloatToString(m_currentRoughness));
	m_metalnessLabel->setText("Metalness: " + StringUtil::FloatToString(m_currentMetalness));
	m_emissivenessLabel->setText("Emissiveness: " + StringUtil::FloatToString(m_currentEmissiveness));
}

void MaterialsWindow::setupMaterialSliders(float& positionY)
{
	if (!m_materialLabel)
	{
		m_materialLabel = m_gui.createLabelNode("Current Material ID: " + std::to_string(m_currentID), GUI::FONT_DEFAULT, 12);
		m_materialLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_materialLabel);
	}
	if (!m_materialSlider)
	{
		m_materialSlider = m_gui.createDefaultSliderInt();
		m_materialSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_materialSlider->setIntCallback([this](int32_t value) {
			setMaterialID(value);
			});
		addChild(m_materialSlider);
	}
	if (!m_materialSprite)
	{
		const std::string path = FileUtil::GetPath().append("Data/Materials/") + "Materials-Albedo.png";
		TextureID albedoID = m_renderer.getTextureID(path, false);
		Rect2D texRect = Rect2D(
			MaterialData::texOffsetX(m_currentID),
			MaterialData::texOffsetY(m_currentID),
			1.f / 16.f, 1.f / 16.f
		);
		m_materialSprite = m_gui.createCustomNode<SpriteNode>(albedoID, texRect);
		m_materialSprite->setContentSize(glm::vec2(256.f, 256.f));
		addChild(m_materialSprite);
	}
	if (!m_pickerButton)
	{
		m_pickerButton = m_gui.createRectButton(glm::vec2(120.f, 30.f), "Material");
		m_pickerButton->setToggleable(true);
		m_pickerButton->setCallback(std::bind(&MaterialsWindow::onMaterialPickerButton, this, std::placeholders::_1));
		addChild(m_pickerButton);
	}

	positionY -= 10.f;
	m_materialLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY, 1.f));
	positionY -= 16.f;

	m_materialSlider->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_materialSlider->setIntValues(1, 255, m_currentID);
	positionY -= 260.f;

	m_materialSprite->setPosition(glm::vec3(4.f, positionY, 1.f));
	positionY -= m_pickerButton->getContentSize().y + 2;
	m_pickerButton->setPosition(glm::vec3(4.f, positionY, 1.f));
}

void MaterialsWindow::removeMaterialSliders()
{
	if (m_materialLabel)
	{
		removeChild(m_materialLabel);
		m_gui.destroyNodeAndChildren(m_materialLabel);
		m_materialLabel = nullptr;
	}
	if (m_materialSlider)
	{
		removeChild(m_materialSlider);
		m_gui.destroyNodeAndChildren(m_materialSlider);
		m_materialSlider = nullptr;
	}
	if (m_materialSprite)
	{
		removeChild(m_materialSprite);
		m_gui.destroyNodeAndChildren(m_materialSprite);
		m_materialSprite = nullptr;
	}
}

void MaterialsWindow::setupColorSliders(float& positionY)
{
	if (!m_colorRLabel)
	{
		m_colorRLabel = m_gui.createLabelNode("Red: " + StringUtil::FloatToString(m_currentColor.r), GUI::FONT_DEFAULT, 12);
		m_colorRLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_colorRLabel);
	}
	if (!m_colorGLabel)
	{
		m_colorGLabel = m_gui.createLabelNode("Green: " + StringUtil::FloatToString(m_currentColor.g), GUI::FONT_DEFAULT, 12);
		m_colorGLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_colorGLabel);
	}
	if (!m_colorBLabel)
	{
		m_colorBLabel = m_gui.createLabelNode("Blue: " + StringUtil::FloatToString(m_currentColor.b), GUI::FONT_DEFAULT, 12);
		m_colorBLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_colorBLabel);
	}
	if (!m_colorALabel)
	{
		m_colorALabel = m_gui.createLabelNode("Alpha: " + StringUtil::FloatToString(m_currentColor.a), GUI::FONT_DEFAULT, 12);
		m_colorALabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_colorALabel);
	}
	if (!m_colorRSlider)
	{
		m_colorRSlider = m_gui.createDefaultSliderFloat();
		m_colorRSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_colorRSlider->setFloatCallback([this](float value) {
			m_currentColor.r = value;
			updateColorText();
			});
		addChild(m_colorRSlider);
	}
	if (!m_colorGSlider)
	{
		m_colorGSlider = m_gui.createDefaultSliderFloat();
		m_colorGSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_colorGSlider->setFloatCallback([this](float value) {
			m_currentColor.g = value;
			updateColorText();
			});
		addChild(m_colorGSlider);
	}
	if (!m_colorBSlider)
	{
		m_colorBSlider = m_gui.createDefaultSliderFloat();
		m_colorBSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_colorBSlider->setFloatCallback([this](float value) {
			m_currentColor.b = value;
			updateColorText();
			});
		addChild(m_colorBSlider);
	}
	if (!m_colorASlider)
	{
		m_colorASlider = m_gui.createDefaultSliderFloat();
		m_colorASlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_colorASlider->setFloatCallback([this](float value) {
			m_currentColor.a = value;
			updateColorText();
			});
		addChild(m_colorASlider);
	}
	if (!m_roughnessLabel)
	{
		m_roughnessLabel = m_gui.createLabelNode("Roughness: " + StringUtil::FloatToString(m_currentRoughness), GUI::FONT_DEFAULT, 12);
		m_roughnessLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_roughnessLabel);
	}
	if (!m_metalnessLabel)
	{
		m_metalnessLabel = m_gui.createLabelNode("Metalness: " + StringUtil::FloatToString(m_currentMetalness), GUI::FONT_DEFAULT, 12);
		m_metalnessLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_metalnessLabel);
	}
	if (!m_emissivenessLabel)
	{
		m_emissivenessLabel = m_gui.createLabelNode("Emissiveness: " + StringUtil::FloatToString(m_currentEmissiveness), GUI::FONT_DEFAULT, 12);
		m_emissivenessLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_emissivenessLabel);
	}
	if (!m_roughnessSlider)
	{
		m_roughnessSlider = m_gui.createDefaultSliderFloat();
		m_roughnessSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_roughnessSlider->setFloatCallback([this](float value) {
			m_currentRoughness = value;
			updateColorText();
			});
		addChild(m_roughnessSlider);
	}
	if (!m_metalnessSlider)
	{
		m_metalnessSlider = m_gui.createDefaultSliderFloat();
		m_metalnessSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_metalnessSlider->setFloatCallback([this](float value) {
			m_currentMetalness = value;
			updateColorText();
			});
		addChild(m_metalnessSlider);
	}
	if (!m_emissivenessSlider)
	{
		m_emissivenessSlider = m_gui.createDefaultSliderFloat();
		m_emissivenessSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_emissivenessSlider->setFloatCallback([this](float value) {
			m_currentEmissiveness = value;
			updateColorText();
			});
		addChild(m_emissivenessSlider);
	}

	positionY -= 20.f;
	m_colorRLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY + 10.f, 1.f));
	positionY -= 26.f;
	m_colorRSlider->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_colorRSlider->setFloatValues(0.f, 1.f, m_currentColor.r);
	positionY -= 20.f;
	m_colorGLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY + 10.f, 1.f));
	positionY -= 26.f;
	m_colorGSlider->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_colorGSlider->setFloatValues(0.f, 1.f, m_currentColor.g);
	positionY -= 20.f;
	m_colorBLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY + 10.f, 1.f));
	positionY -= 26.f;
	m_colorBSlider->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_colorBSlider->setFloatValues(0.f, 1.f, m_currentColor.b);
	positionY -= 20.f;
	m_colorALabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY + 10.f, 1.f));
	positionY -= 26.f;
	m_colorASlider->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_colorASlider->setFloatValues(0.f, 1.f, m_currentColor.a);
	positionY -= 26.f;

	m_roughnessLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY + 10.f, 1.f));
	positionY -= 26.f;
	m_roughnessSlider->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_roughnessSlider->setFloatValues(0.f, 1.f, m_currentRoughness);
	positionY -= 20.f;
	m_metalnessLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY + 10.f, 1.f));
	positionY -= 26.f;
	m_metalnessSlider->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_metalnessSlider->setFloatValues(0.f, 1.f, m_currentMetalness);
	positionY -= 20.f;
	m_emissivenessLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY + 10.f, 1.f));
	positionY -= 26.f;
	m_emissivenessSlider->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_emissivenessSlider->setFloatValues(0.f, 1.f, m_currentEmissiveness);
}

void MaterialsWindow::removeColorSliders()
{
	if (m_colorRLabel)
	{
		removeChild(m_colorRLabel);
		m_gui.destroyNodeAndChildren(m_colorRLabel);
		m_colorRLabel = nullptr;
	}
	if (m_colorGLabel)
	{
		removeChild(m_colorGLabel);
		m_gui.destroyNodeAndChildren(m_colorGLabel);
		m_colorGLabel = nullptr;
	}
	if (m_colorBLabel)
	{
		removeChild(m_colorBLabel);
		m_gui.destroyNodeAndChildren(m_colorBLabel);
		m_colorBLabel = nullptr;
	}
	if (m_colorALabel)
	{
		removeChild(m_colorALabel);
		m_gui.destroyNodeAndChildren(m_colorALabel);
		m_colorALabel = nullptr;
	}
	if (m_colorRSlider)
	{
		removeChild(m_colorRSlider);
		m_gui.destroyNodeAndChildren(m_colorRSlider);
		m_colorRSlider = nullptr;
	}
	if (m_colorGSlider)
	{
		removeChild(m_colorGSlider);
		m_gui.destroyNodeAndChildren(m_colorGSlider);
		m_colorGSlider = nullptr;
	}
	if (m_colorBSlider)
	{
		removeChild(m_colorBSlider);
		m_gui.destroyNodeAndChildren(m_colorBSlider);
		m_colorBSlider = nullptr;
	}
	if (m_colorASlider)
	{
		removeChild(m_colorASlider);
		m_gui.destroyNodeAndChildren(m_colorASlider);
		m_colorASlider = nullptr;
	}

	if (m_roughnessLabel)
	{
		removeChild(m_roughnessLabel);
		m_gui.destroyNodeAndChildren(m_roughnessLabel);
		m_roughnessLabel = nullptr;
	}
	if (m_metalnessLabel)
	{
		removeChild(m_metalnessLabel);
		m_gui.destroyNodeAndChildren(m_metalnessLabel);
		m_metalnessLabel = nullptr;
	}
	if (m_emissivenessLabel)
	{
		removeChild(m_emissivenessLabel);
		m_gui.destroyNodeAndChildren(m_emissivenessLabel);
		m_emissivenessLabel = nullptr;
	}

	if (m_roughnessSlider)
	{
		removeChild(m_roughnessSlider);
		m_gui.destroyNodeAndChildren(m_roughnessSlider);
		m_roughnessSlider = nullptr;
	}
	if (m_metalnessSlider)
	{
		removeChild(m_metalnessSlider);
		m_gui.destroyNodeAndChildren(m_metalnessSlider);
		m_metalnessSlider = nullptr;
	}
	if (m_emissivenessSlider)
	{
		removeChild(m_emissivenessSlider);
		m_gui.destroyNodeAndChildren(m_emissivenessSlider);
		m_emissivenessSlider = nullptr;
	}
}

void MaterialsWindow::setupButtons(float& positionY)
{
	if (!m_colorCubeButton)
	{
		m_colorCubeButton = m_gui.createRectButton(glm::vec2(80.f, 24.f), "Colored");
		m_colorCubeButton->setCallback([this](bool value) {
			m_displayColorCube = value;
			refresh();
			});
		addChild(m_colorCubeButton);
	}
	if (!m_sphereButton)
	{
		m_sphereButton = m_gui.createRectButton(glm::vec2(80.f, 24.f), "Sphere");
		m_sphereButton->setCallback([this](bool value) {
			m_displaySphere = value;
			});
		addChild(m_sphereButton);
	}
	if (!m_arrayButton)
	{
		m_arrayButton = m_gui.createRectButton(glm::vec2(80.f, 24.f), "Array");
		m_arrayButton->setCallback([this](bool value) {
			m_displayArray = value;
			});
		addChild(m_arrayButton);
	}
	if (!m_sizeLabel)
	{
		m_sizeLabel = m_gui.createLabelNode("Size: " + std::to_string(m_displaySize), GUI::FONT_DEFAULT, 12);
		m_sizeLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		addChild(m_sizeLabel);
	}
	if (!m_sizeSlider)
	{
		m_sizeSlider = m_gui.createDefaultSliderFloat();
		m_sizeSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_sizeSlider->setFloatValues(0.01f, 2.f, m_displaySize);
		m_sizeSlider->setFloatCallback([this](float value) {
			m_displaySize = value;
			m_sizeLabel->setText("Size: " + StringUtil::FloatToString(m_displaySize));
			});
		addChild(m_sizeSlider);
	}
	positionY -= 26.f;
	m_colorCubeButton->setPosition(glm::vec3(4.f, positionY, 1.f));
	m_colorCubeButton->setToggleable(true);
	m_sphereButton->setPosition(glm::vec3(88.f, positionY, 1.f));
	m_sphereButton->setToggleable(true);
	m_arrayButton->setPosition(glm::vec3(172.f, positionY, 1.f));
	m_arrayButton->setToggleable(true);
	positionY -= 26.f;
	m_sizeLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY + 10.f, 1.f));
	m_sizeSlider->setPosition(glm::vec3(4.f, positionY, 1.f));
}

void MaterialsWindow::onMaterialPickerButton(bool state)
{
	if (!m_materialPicker)
	{
		m_materialPicker = m_gui.createCustomNode<MaterialPicker>(m_gui, m_renderer, m_materials, glm::vec2(512, 512));
		m_materialPicker->setPosition(glm::vec3(-512.f, 0.f, 10.f));
		m_materialPicker->setMaterialID(m_currentID);
		m_materialPicker->setCallback(std::bind(&MaterialsWindow::setMaterialID, this, std::placeholders::_1));
		addChild(m_materialPicker);
	}
	else
	{
		removeChild(m_materialPicker);
		m_gui.destroyNodeAndChildren(m_materialPicker);
		m_materialPicker = nullptr;
	}
}

void MaterialsWindow::refresh()
{
	float positionY = m_contentNode->getContentSize().y;
	if (m_displayColorCube)
	{
		removeMaterialSliders();
		setupColorSliders(positionY);
	}
	else
	{
		removeColorSliders();
		setupMaterialSliders(positionY);
	}
	setupButtons(positionY);
}
