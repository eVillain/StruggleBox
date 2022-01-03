#include "ObjectWindow.h"

#include "GUI.h"
#include "ButtonNode.h"
#include "MaterialPicker.h"
#include "LabelNode.h"
#include "SpriteNode.h"

#include "PathUtil.h"
#include "Log.h"
#include "StringUtil.h"
#include "MathUtils.h"
#include "MaterialData.h"
#include "VoxelData.h"
#include "Renderer.h"
#include <string>

const glm::vec2 ObjectWindow::WINDOW_SIZE = glm::vec2(260.f, 600.f);

ObjectWindow::ObjectWindow(
	const GUI& gui,
	Renderer& renderer,
	MaterialData& materials)
	: WindowNode(gui, WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, "Object")
	, m_renderer(renderer)
	, m_materials(materials)
	, m_voxels(nullptr)
	, m_currentID(1)
	, m_editTool(ObjectTool_None)
	, m_resizeCallback(nullptr)
	, m_rescaleCallback(nullptr)
	, m_materialLabel(nullptr)
	, m_materialSprite(nullptr)
	, m_pickerButton(nullptr)
	, m_materialPicker(nullptr)
	, m_voxelSizeLabel(nullptr)
	, m_voxelSizeIncXButton(nullptr)
	, m_voxelSizeDecXButton(nullptr)
	, m_voxelSizeIncYButton(nullptr)
	, m_voxelSizeDecYButton(nullptr)
	, m_voxelSizeIncZButton(nullptr)
	, m_voxelSizeDecZButton(nullptr)
	, m_voxelScaleLabel(nullptr)
	, m_voxelScaleIncButton(nullptr)
	, m_voxelScaleDecButton(nullptr)
	, m_fillButton(nullptr)
	, m_clearButton(nullptr)
	, m_generateTreeButton(nullptr)
{
	Log::Debug("[ObjectWindow] constructor, instance at %p", this);

	setupMaterialOptions();
	setupVoxelOptions();
}

void ObjectWindow::setVoxels(VoxelData* voxels)
{
	m_voxels = voxels;

	refresh();
}

//bool ObjectWindow::OnEvent(const tb::TBWidgetEvent & ev)
//{
//	if (ev.type == tb::EVENT_TYPE_CLICK)
//	{
//		if (ev.target->GetID() == TBIDC("button-material-select"))
//		{
//			_currentID = ev.target->data.GetInt();
//		}
//		else if (ev.target->GetID() == TBIDC("button-cursor"))
//		{
//			_editTool = ObjectTool_None;
//		}
//		else if (ev.target->GetID() == TBIDC("button-selection"))
//		{
//			_editTool = ObjectTool_Selection;
//		}
//		else if (ev.target->GetID() == TBIDC("button-block"))
//		{
//			_editTool = ObjectTool_Block;
//		}
//		else if (ev.target->GetID() == TBIDC("button-column"))
//		{
//			_editTool = ObjectTool_Column;
//		}
//		else if (ev.target->GetID() == TBIDC("button-clear"))
//		{
//			_voxels->clear();
//		}
//		else if (ev.target->GetID() == TBIDC("button-generateTree"))
//		{
//			_voxels->generateGrass(0);
//		}
//		else if (ev.target->GetID() == TBIDC("button-rotateYPos"))
//		{
//			_voxels->rotateY(false);
//		}
//		else if (ev.target->GetID() == TBIDC("button-rotateYNeg"))
//		{
//			_voxels->rotateY(true);
//		}
//		else if (ev.target->GetID() == TBIDC("button-scalePos"))
//		{
//			_voxels->rotateY(false);
//		}
//		else if (ev.target->GetID() == TBIDC("button-scaleYNeg"))
//		{
//			_voxels->rotateY(true);
//		}
//		return true;
//	}
//}
//
void ObjectWindow::setCurrentID(const uint8_t newID)
{
	m_currentID = newID;

	//if (m_materialPicker)
	//{
	//	removeChild(m_materialPicker);
	//	m_gui.destroyNodeAndChildren(m_materialPicker);
	//	m_materialPicker = nullptr;
	//	m_pickerButton->setToggled(false);
	//}

	refresh();
}

void ObjectWindow::setupMaterialOptions()
{
	m_materialLabel = m_gui.createLabelNode("Current Material ID: " + std::to_string(m_currentID), GUI::FONT_DEFAULT, 12);
	m_materialLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	addChild(m_materialLabel);

	const std::string path = FileUtil::GetPath().append("Data/Materials/") + "Materials-Albedo.png";
	const TextureID albedoID = m_renderer.getTextureID(path, false);
	const Rect2D texRect = Rect2D(
		MaterialData::texOffsetX(m_currentID),
		MaterialData::texOffsetY(m_currentID),
		1.f / 16.f, 1.f / 16.f
	);
	m_materialSprite = m_gui.createCustomNode<SpriteNode>(albedoID, texRect);
	m_materialSprite->setContentSize(glm::vec2(256.f, 256.f));
	addChild(m_materialSprite);

	m_pickerButton = m_gui.createRectButton(glm::vec2(120.f, 30.f), "Material");
	m_pickerButton->setToggleable(true);
	m_pickerButton->setCallback(std::bind(&ObjectWindow::onMaterialPickerButton, this, std::placeholders::_1));
	addChild(m_pickerButton);
}

void ObjectWindow::setupVoxelOptions()
{
	//const std::string voxelVolStr =
	//	"Voxel Volume Size X:" + std::to_string(m_voxels->getSizeX()) +
	//	", Y: " + std::to_string(m_voxels->getSizeY()) + 
	//	", Z: " + std::to_string(m_voxels->getSizeZ());
	m_voxelSizeLabel = m_gui.createLabelNode("", GUI::FONT_DEFAULT, 12);
	m_voxelSizeLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	addChild(m_voxelSizeLabel);

	const glm::vec2 BUTTON_SIZE = glm::vec2(40.f, 30.f);
	m_voxelSizeIncXButton = m_gui.createRectButton(BUTTON_SIZE, "X+");
	m_voxelSizeDecXButton = m_gui.createRectButton(BUTTON_SIZE, "X-");
	m_voxelSizeIncYButton = m_gui.createRectButton(BUTTON_SIZE, "Y+");
	m_voxelSizeDecYButton = m_gui.createRectButton(BUTTON_SIZE, "Y-");
	m_voxelSizeIncZButton = m_gui.createRectButton(BUTTON_SIZE, "Z+");
	m_voxelSizeDecZButton = m_gui.createRectButton(BUTTON_SIZE, "Z-");
	m_voxelSizeIncXButton->setCallback([this](bool) {
		if (m_resizeCallback)
		{
			m_resizeCallback(glm::ivec3(1,0,0));
		}
		});
	m_voxelSizeDecXButton->setCallback([this](bool) {
		if (m_resizeCallback)
		{
			m_resizeCallback(glm::ivec3(-1, 0, 0));
		}
		});
	m_voxelSizeIncYButton->setCallback([this](bool) {
		if (m_resizeCallback)
		{
			m_resizeCallback(glm::ivec3(0, 1, 0));
		}
		});
	m_voxelSizeDecYButton->setCallback([this](bool) {
		if (m_resizeCallback)
		{
			m_resizeCallback(glm::ivec3(0, -1, 0));
		}
		});
	m_voxelSizeIncZButton->setCallback([this](bool) {
		if (m_resizeCallback)
		{
			m_resizeCallback(glm::ivec3(0, 0, 1));
		}
		});
	m_voxelSizeDecZButton->setCallback([this](bool) {
		if (m_resizeCallback)
		{
			m_resizeCallback(glm::ivec3(0, 0, -1));
		}
		});
	addChild(m_voxelSizeIncXButton);
	addChild(m_voxelSizeDecXButton);
	addChild(m_voxelSizeIncYButton);
	addChild(m_voxelSizeDecYButton);
	addChild(m_voxelSizeIncZButton);
	addChild(m_voxelSizeDecZButton);

	//const std::string voxelScaleStr = "Voxel Volume Scale:" + std::to_string(m_voxels->getScale());
	m_voxelScaleLabel = m_gui.createLabelNode("", GUI::FONT_DEFAULT, 12);
	m_voxelScaleLabel->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	addChild(m_voxelScaleLabel);

	m_voxelScaleIncButton = m_gui.createRectButton(BUTTON_SIZE, "S+");
	m_voxelScaleDecButton = m_gui.createRectButton(BUTTON_SIZE, "S-");
	m_voxelScaleIncButton->setCallback([this](bool) {
		if (m_rescaleCallback)
		{
			const float newScale = m_voxels->getScale() * 2.f;
			m_rescaleCallback(newScale);
			refresh();
		}
		});
	m_voxelScaleDecButton->setCallback([this](bool) {
		if (m_rescaleCallback)
		{
			const float newScale = m_voxels->getScale() * 0.5f;
			m_rescaleCallback(newScale);
			refresh();
		}
		});
	addChild(m_voxelScaleIncButton);
	addChild(m_voxelScaleDecButton);

	m_clearButton = m_gui.createRectButton(glm::vec2(80.f, 30.f), "Clear");
	m_fillButton = m_gui.createRectButton(glm::vec2(80.f, 30.f), "Fill");
	m_generateTreeButton = m_gui.createRectButton(glm::vec2(80.f, 30.f), "Tree");
	m_clearButton->setCallback([this](bool) {
		m_voxels->clear();
		if (m_updateCallback) { m_updateCallback(); }
		});
	m_fillButton->setCallback([this](bool) {
		m_voxels->fill(m_currentID);
		if (m_updateCallback) { m_updateCallback(); }
		});
	m_generateTreeButton->setCallback([this](bool) {
		m_voxels->generateTree(m_voxels->getVolume(0.25f) * 0.5f, 1337);
		if (m_updateCallback) { m_updateCallback(); }
		});
	addChild(m_clearButton);
	addChild(m_fillButton);
	addChild(m_generateTreeButton);
}

void ObjectWindow::onMaterialPickerButton(bool state)
{
	if (!m_materialPicker)
	{
		m_materialPicker = m_gui.createCustomNode<MaterialPicker>(m_gui, m_renderer, m_materials, glm::vec2(512, 512));
		m_materialPicker->setPosition(glm::vec3(-512.f, 0.f, 10.f));
		m_materialPicker->setMaterialID(m_currentID);
		m_materialPicker->setCallback(std::bind(&ObjectWindow::setCurrentID, this, std::placeholders::_1));
		addChild(m_materialPicker);
	}
	else
	{
		removeChild(m_materialPicker);
		m_gui.destroyNodeAndChildren(m_materialPicker);
		m_materialPicker = nullptr;
	}
}

void ObjectWindow::refresh()
{
	m_materialLabel->setText("Current Material ID: " + std::to_string(m_currentID));
	Rect2D texRect = Rect2D(
		MaterialData::texOffsetX(m_currentID),
		MaterialData::texOffsetY(m_currentID),
		1.f / 16.f, 1.f / 16.f
	);
	m_materialSprite->setTextureRect(texRect);

	float positionY = m_contentNode->getContentSize().y;
	positionY -= 10.f;
	m_materialLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY, 1.f));
	positionY -= 10.f + m_pickerButton->getContentSize().y;

	m_pickerButton->setPosition(glm::vec3(2.f, positionY, 1.f));
	positionY -= 260.f;

	m_materialSprite->setPosition(glm::vec3(2.f, positionY, 1.f));

	const std::string voxelVolStr =
		"Voxel Volume Size X:" + std::to_string(m_voxels->getSizeX()) +
		", Y: " + std::to_string(m_voxels->getSizeY()) +
		", Z: " + std::to_string(m_voxels->getSizeZ());
	m_voxelSizeLabel->setText(voxelVolStr);

	positionY -= 10.f;
	m_voxelSizeLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY, 1.f));
	positionY -= 10.f + m_voxelSizeIncXButton->getContentSize().y;

	float positionX = 2.f;
	m_voxelSizeIncXButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_voxelSizeIncXButton->getContentSize().x + 2;
	m_voxelSizeDecXButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_voxelSizeDecXButton->getContentSize().x + 2;
	m_voxelSizeIncYButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_voxelSizeIncYButton->getContentSize().x + 2;
	m_voxelSizeDecYButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_voxelSizeDecYButton->getContentSize().x + 2;
	m_voxelSizeIncZButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_voxelSizeIncZButton->getContentSize().x + 2;
	m_voxelSizeDecZButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_voxelSizeDecZButton->getContentSize().x + 2;

	const std::string voxelScaleStr = "Voxel Volume Scale:" + std::to_string(m_voxels->getScale());
	m_voxelScaleLabel->setText(voxelScaleStr);

	positionY -= 10.f;
	m_voxelScaleLabel->setPosition(glm::vec3(m_contentSize.x * 0.5f, positionY, 1.f));
	positionY -= 10.f + m_voxelScaleIncButton->getContentSize().y;

	positionX = 2.f;
	m_voxelScaleIncButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_voxelScaleIncButton->getContentSize().x + 2;
	m_voxelScaleDecButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_voxelScaleDecButton->getContentSize().x + 2;

	positionX = 2.f;
	positionY -= m_clearButton->getContentSize().y + 2;
	m_clearButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_clearButton->getContentSize().x + 2;
	m_fillButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_fillButton->getContentSize().x + 2;
	m_generateTreeButton->setPosition(glm::vec3(positionX, positionY, 1.f));
	positionX += m_generateTreeButton->getContentSize().x + 2;
}