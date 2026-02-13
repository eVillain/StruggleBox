#pragma once

#include "Color.h"
#include "MaterialData.h"
#include "WindowNode.h"
#include <functional>
#include <string>

class ButtonNode;
class LabelNode;
class LayoutNode;
class ScrollerNode;
class SliderNode;
class SpriteNode;
class Renderer2D;
class MaterialPicker;

class MaterialsWindow : public WindowNode
{
public:
	MaterialsWindow(
		const GUI& gui,
		MaterialData& materials);

	uint8_t getCurrentID() const { return m_currentID; }
	Color getCurrentColor() const { return m_currentColor; }
	float getCurrentRoughness() const { return m_currentRoughness; }
	float getCurrentMetalness() const { return m_currentMetalness; }
	float getCurrentEmissiveness() const { return m_currentEmissiveness; }

	bool getDisplaySphere() { return m_displaySphere; }
	bool getDisplayArray() { return m_displayArray; }
	float getDisplaySize() const { return m_displaySize; }

	void refresh();

private:
	static const glm::vec2 WINDOW_SIZE;

	MaterialData& m_materials;

	LayoutNode* m_layoutNode;
	LayoutNode* m_buttonsLayoutNode;
	LabelNode* m_materialLabel;
	SliderNode* m_materialSlider;

	LabelNode* m_colorRLabel;
	LabelNode* m_colorGLabel;
	LabelNode* m_colorBLabel;
	LabelNode* m_colorALabel;
	SliderNode* m_colorRSlider;
	SliderNode* m_colorGSlider;
	SliderNode* m_colorBSlider;
	SliderNode* m_colorASlider;

	LabelNode* m_roughnessLabel;
	LabelNode* m_metalnessLabel;
	LabelNode* m_emissivenessLabel;
	SliderNode* m_roughnessSlider;
	SliderNode* m_metalnessSlider;
	SliderNode* m_emissivenessSlider;

	ButtonNode* m_sphereButton;
	ButtonNode* m_arrayButton;

	LabelNode* m_sizeLabel;
	SliderNode* m_sizeSlider;

	ScrollerNode* m_listScroller;

	uint8_t m_currentID;
	Color m_currentColor;
	float m_currentRoughness;
	float m_currentMetalness;
	float m_currentEmissiveness;

	bool m_displaySphere;
	bool m_displayArray;

	float m_displaySize;

	void setMaterialID(uint8_t materialID);
	void updateColorText();
	void updateColorSliders();

	void setupMaterialSliders();
	void removeMaterialSliders();
	
	void setupColorSliders();
	void removeColorSliders();

	void setupButtons();
	void setupList();
};

