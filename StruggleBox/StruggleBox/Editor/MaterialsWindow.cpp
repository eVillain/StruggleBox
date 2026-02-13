#include "MaterialsWindow.h"

#include "ButtonNode.h"
#include "GUI.h"
#include "LabelNode.h"
#include "LayoutNode.h"
#include "Log.h"
#include "MaterialPicker.h"
#include "MathUtils.h"
#include "PathUtil.h"
#include "ScrollerNode.h"
#include "SliderNode.h"
#include "SpriteNode.h"
#include "StringUtil.h"
#include <string>

const glm::vec2 MaterialsWindow::WINDOW_SIZE = glm::vec2(260.f, 700.f);
const int TEXT_SIZE = 16;
const float BUTTON_SIZE = 24.f;

MaterialsWindow::MaterialsWindow(
	const GUI& gui,
	MaterialData& materials)
	: WindowNode(gui, WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, "Materials")
	, m_materials(materials)
	, m_layoutNode(gui.createCustomNode<LayoutNode>())
	, m_buttonsLayoutNode(gui.createCustomNode<LayoutNode>())
	, m_materialLabel(nullptr)
	, m_materialSlider(nullptr)
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
	, m_sphereButton(nullptr)
	, m_arrayButton(nullptr)
	, m_sizeLabel(nullptr)
	, m_sizeSlider(nullptr)
	, m_currentID(1)
	, m_currentColor(RGBAColor(1.f, 1.f, 1.f, 1.f))
	, m_currentRoughness(1.f)
	, m_currentMetalness(0.f)
	, m_currentEmissiveness(0.f)
	, m_displaySphere(false)
	, m_displayArray(false)
	, m_displaySize(1.f)
	, m_listScroller(gui.createScrollerNode(GUI::WINDOW_CONTENT))
{
	Log::Debug("[MaterialsWindow] constructor, instance at %p", this);

	m_layoutNode->setContentSize(m_contentNode->getContentSize());
	m_layoutNode->setLayoutType(LayoutType::Column);
	m_layoutNode->setAlignmentV(LayoutAlignmentV::Top);
	m_layoutNode->setPadding(2.f);

	m_buttonsLayoutNode->setContentSize(glm::vec2(m_contentSize.x - 4.f, 28.f));
	m_buttonsLayoutNode->setLayoutType(LayoutType::Row);
	m_buttonsLayoutNode->setAlignmentH(LayoutAlignmentH::Left);
	m_buttonsLayoutNode->setPadding(2.f);

	m_listScroller->setContentSize(glm::vec2(m_contentSize.x - 12.f, 200.f));
	m_listScroller->setShowScrollBar(true);

	m_contentNode->addChild(m_layoutNode);
	m_layoutNode->addChild(m_buttonsLayoutNode);
	m_layoutNode->addChild(m_listScroller);
	refresh();
}

void MaterialsWindow::setMaterialID(uint8_t materialID)
{
	m_currentID = materialID;
	if (m_materialLabel)
	{
		m_materialLabel->setText("Current Material ID: " + std::to_string(m_currentID));
	}
	m_materialSlider->setIntValues(1, 255, m_currentID);
	const MaterialDef& material = m_materials[m_currentID - 1];
	m_currentColor = material.albedo;
	m_currentRoughness = material.roughness;
	m_currentMetalness = material.metalness;
	m_currentEmissiveness = 0.f;

	updateColorText();
	updateColorSliders();

	const std::vector<Node*> materialButtons = m_listScroller->getContent();
	for (size_t i = 0; i < materialButtons.size(); i++)
	{
		ButtonNode* materialButton = (ButtonNode*)materialButtons.at(i);
		materialButton->setToggled(i == m_currentID - 1);
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

void MaterialsWindow::updateColorSliders()
{
	m_colorRSlider->setFloatValues(0.f, 1.f, m_currentColor.r);
	m_colorGSlider->setFloatValues(0.f, 1.f, m_currentColor.g);
	m_colorBSlider->setFloatValues(0.f, 1.f, m_currentColor.b);
	m_colorASlider->setFloatValues(0.f, 1.f, m_currentColor.a);
	m_roughnessSlider->setFloatValues(0.f, 1.f, m_currentRoughness);
	m_metalnessSlider->setFloatValues(0.f, 1.f, m_currentMetalness);
	m_emissivenessSlider->setFloatValues(0.f, 1.f, m_currentEmissiveness);
}

void MaterialsWindow::setupMaterialSliders()
{
	if (!m_materialLabel)
	{
		m_materialLabel = m_gui.createLabelNode("Current Material ID: " + std::to_string(m_currentID), GUI::FONT_DEFAULT, TEXT_SIZE);
		m_materialLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_materialLabel);
	}
	if (!m_materialSlider)
	{
		m_materialSlider = m_gui.createDefaultSliderInt();
		m_materialSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_materialSlider->setIntCallback([this](int32_t value) {
			setMaterialID(value);
			});
		m_layoutNode->addChild(m_materialSlider);
	}

	m_materialSlider->setIntValues(1, 255, m_currentID);
}

void MaterialsWindow::removeMaterialSliders()
{
	if (m_materialLabel)
	{
		m_layoutNode->removeChild(m_materialLabel);
		m_gui.destroyNodeAndChildren(m_materialLabel);
		m_materialLabel = nullptr;
	}
	if (m_materialSlider)
	{
		m_layoutNode->removeChild(m_materialSlider);
		m_gui.destroyNodeAndChildren(m_materialSlider);
		m_materialSlider = nullptr;
	}
}

void MaterialsWindow::setupColorSliders()
{
	if (!m_colorRLabel)
	{
		m_colorRLabel = m_gui.createLabelNode("Red: " + StringUtil::FloatToString(m_currentColor.r), GUI::FONT_DEFAULT, TEXT_SIZE);
		m_colorRLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_colorRLabel);
	}
	if (!m_colorRSlider)
	{
		m_colorRSlider = m_gui.createDefaultSliderFloat();
		m_colorRSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_colorRSlider->setFloatCallback([this](float value) {
			m_currentColor.r = value;
			updateColorText();
			});
		m_layoutNode->addChild(m_colorRSlider);
	}
	if (!m_colorGLabel)
	{
		m_colorGLabel = m_gui.createLabelNode("Green: " + StringUtil::FloatToString(m_currentColor.g), GUI::FONT_DEFAULT, TEXT_SIZE);
		m_colorGLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_colorGLabel);
	}
	if (!m_colorGSlider)
	{
		m_colorGSlider = m_gui.createDefaultSliderFloat();
		m_colorGSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_colorGSlider->setFloatCallback([this](float value) {
			m_currentColor.g = value;
			updateColorText();
			});
		m_layoutNode->addChild(m_colorGSlider);
	}
	if (!m_colorBLabel)
	{
		m_colorBLabel = m_gui.createLabelNode("Blue: " + StringUtil::FloatToString(m_currentColor.b), GUI::FONT_DEFAULT, TEXT_SIZE);
		m_colorBLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_colorBLabel);
	}
	if (!m_colorBSlider)
	{
		m_colorBSlider = m_gui.createDefaultSliderFloat();
		m_colorBSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_colorBSlider->setFloatCallback([this](float value) {
			m_currentColor.b = value;
			updateColorText();
			});
		m_layoutNode->addChild(m_colorBSlider);
	}
	if (!m_colorALabel)
	{
		m_colorALabel = m_gui.createLabelNode("Alpha: " + StringUtil::FloatToString(m_currentColor.a), GUI::FONT_DEFAULT, TEXT_SIZE);
		m_colorALabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_colorALabel);
	}
	if (!m_colorASlider)
	{
		m_colorASlider = m_gui.createDefaultSliderFloat();
		m_colorASlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_colorASlider->setFloatCallback([this](float value) {
			m_currentColor.a = value;
			updateColorText();
			});
		m_layoutNode->addChild(m_colorASlider);
	}
	if (!m_roughnessLabel)
	{
		m_roughnessLabel = m_gui.createLabelNode("Roughness: " + StringUtil::FloatToString(m_currentRoughness), GUI::FONT_DEFAULT, TEXT_SIZE);
		m_roughnessLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_roughnessLabel);
	}
	if (!m_roughnessSlider)
	{
		m_roughnessSlider = m_gui.createDefaultSliderFloat();
		m_roughnessSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_roughnessSlider->setFloatCallback([this](float value) {
			m_currentRoughness = value;
			updateColorText();
			});
		m_layoutNode->addChild(m_roughnessSlider);
	}
	if (!m_metalnessLabel)
	{
		m_metalnessLabel = m_gui.createLabelNode("Metalness: " + StringUtil::FloatToString(m_currentMetalness), GUI::FONT_DEFAULT, TEXT_SIZE);
		m_metalnessLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_metalnessLabel);
	}
	if (!m_metalnessSlider)
	{
		m_metalnessSlider = m_gui.createDefaultSliderFloat();
		m_metalnessSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_metalnessSlider->setFloatCallback([this](float value) {
			m_currentMetalness = value;
			updateColorText();
			});
		m_layoutNode->addChild(m_metalnessSlider);
	}
	if (!m_emissivenessLabel)
	{
		m_emissivenessLabel = m_gui.createLabelNode("Emissiveness: " + StringUtil::FloatToString(m_currentEmissiveness), GUI::FONT_DEFAULT, TEXT_SIZE);
		m_emissivenessLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_emissivenessLabel);
	}
	if (!m_emissivenessSlider)
	{
		m_emissivenessSlider = m_gui.createDefaultSliderFloat();
		m_emissivenessSlider->setContentSize(glm::vec2(m_contentSize.x - 24.f, 24.f));
		m_emissivenessSlider->setFloatCallback([this](float value) {
			m_currentEmissiveness = value;
			updateColorText();
			});
		m_layoutNode->addChild(m_emissivenessSlider);
	}
	updateColorSliders();
}

void MaterialsWindow::removeColorSliders()
{
	if (m_colorRLabel)
	{
		m_layoutNode->removeChild(m_colorRLabel);
		m_gui.destroyNodeAndChildren(m_colorRLabel);
		m_colorRLabel = nullptr;
	}
	if (m_colorGLabel)
	{
		m_layoutNode->removeChild(m_colorGLabel);
		m_gui.destroyNodeAndChildren(m_colorGLabel);
		m_colorGLabel = nullptr;
	}
	if (m_colorBLabel)
	{
		m_layoutNode->removeChild(m_colorBLabel);
		m_gui.destroyNodeAndChildren(m_colorBLabel);
		m_colorBLabel = nullptr;
	}
	if (m_colorALabel)
	{
		m_layoutNode->removeChild(m_colorALabel);
		m_gui.destroyNodeAndChildren(m_colorALabel);
		m_colorALabel = nullptr;
	}
	if (m_colorRSlider)
	{
		m_layoutNode->removeChild(m_colorRSlider);
		m_gui.destroyNodeAndChildren(m_colorRSlider);
		m_colorRSlider = nullptr;
	}
	if (m_colorGSlider)
	{
		m_layoutNode->removeChild(m_colorGSlider);
		m_gui.destroyNodeAndChildren(m_colorGSlider);
		m_colorGSlider = nullptr;
	}
	if (m_colorBSlider)
	{
		m_layoutNode->removeChild(m_colorBSlider);
		m_gui.destroyNodeAndChildren(m_colorBSlider);
		m_colorBSlider = nullptr;
	}
	if (m_colorASlider)
	{
		m_layoutNode->removeChild(m_colorASlider);
		m_gui.destroyNodeAndChildren(m_colorASlider);
		m_colorASlider = nullptr;
	}

	if (m_roughnessLabel)
	{
		m_layoutNode->removeChild(m_roughnessLabel);
		m_gui.destroyNodeAndChildren(m_roughnessLabel);
		m_roughnessLabel = nullptr;
	}
	if (m_metalnessLabel)
	{
		m_layoutNode->removeChild(m_metalnessLabel);
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
		m_layoutNode->removeChild(m_roughnessSlider);
		m_gui.destroyNodeAndChildren(m_roughnessSlider);
		m_roughnessSlider = nullptr;
	}
	if (m_metalnessSlider)
	{
		m_layoutNode->removeChild(m_metalnessSlider);
		m_gui.destroyNodeAndChildren(m_metalnessSlider);
		m_metalnessSlider = nullptr;
	}
	if (m_emissivenessSlider)
	{
		m_layoutNode->removeChild(m_emissivenessSlider);
		m_gui.destroyNodeAndChildren(m_emissivenessSlider);
		m_emissivenessSlider = nullptr;
	}
}

void MaterialsWindow::setupButtons()
{
	if (!m_sphereButton)
	{
		m_sphereButton = m_gui.createRectButton(glm::vec2(80.f, BUTTON_SIZE), "Sphere");
		m_sphereButton->setCallback([this](bool value) {
			m_displaySphere = value;
			});
		m_buttonsLayoutNode->addChild(m_sphereButton);
	}
	if (!m_arrayButton)
	{
		m_arrayButton = m_gui.createRectButton(glm::vec2(80.f, BUTTON_SIZE), "Array");
		m_arrayButton->setCallback([this](bool value) {
			m_displayArray = value;
			});
		m_buttonsLayoutNode->addChild(m_arrayButton);
	}
	m_buttonsLayoutNode->refresh();

	if (!m_sizeLabel)
	{
		m_sizeLabel = m_gui.createLabelNode("Size: " + std::to_string(m_displaySize), GUI::FONT_DEFAULT, 12);
		m_sizeLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
		m_layoutNode->addChild(m_sizeLabel);
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
		m_layoutNode->addChild(m_sizeSlider);
	}
	m_sphereButton->setToggleable(true);
	m_sphereButton->setToggled(m_displaySphere);
	m_arrayButton->setToggleable(true);
	m_arrayButton->setToggled(m_displayArray);
}

void MaterialsWindow::setupList()
{
	const glm::vec2 MATERIAL_NODE_SIZE = glm::vec2(m_contentNode->getContentSize().x - 2, 24.f);

	std::vector<Node*> nodes;
	for (size_t i = 0; i < 256; i++)
	{
		const std::string& materialName = m_materials.getName(i);
		if (materialName.empty())
		{
			continue;
		}
		ButtonNode* materialButton = m_gui.createRectButton(MATERIAL_NODE_SIZE, materialName);
		LabelNode* buttonLabel = dynamic_cast<LabelNode*>(materialButton->getChildren().at(0));
		if (buttonLabel)
		{
			buttonLabel->setAnchorPoint(glm::vec2(0.f, 0.5f));
		}
		materialButton->setCallback([this, i](bool) {
			setMaterialID(i);
			});
		materialButton->setToggleable(true);
		nodes.push_back(materialButton);
	}
	m_listScroller->setContent(nodes, ScrollStrategy::KEEP_OFFSET);
}

void MaterialsWindow::refresh()
{
	setupColorSliders();
	setupMaterialSliders();
	setupButtons();
	setupList();
	m_layoutNode->refresh();
}
