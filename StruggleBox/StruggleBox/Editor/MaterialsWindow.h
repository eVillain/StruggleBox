#pragma once

#include "MaterialData.h"
#include "WindowNode.h"
#include <functional>
#include <string>

class ButtonNode;
class LabelNode;
class ScrollerNode;
class SliderNode;
class SpriteNode;
class Renderer;
class MaterialPicker;

class MaterialsWindow : public WindowNode
{
public:
	MaterialsWindow(
		const GUI& gui,
		Renderer& renderer,
		MaterialData& materials);

	uint8_t getCurrentID() const { return m_currentID; }
	Color getCurrentColor() const { return m_currentColor; }
	float getCurrentRoughness() const { return m_currentRoughness; }
	float getCurrentMetalness() const { return m_currentMetalness; }
	float getCurrentEmissiveness() const { return m_currentEmissiveness; }

	bool getDisplayColorCube() const { return m_displayColorCube; }
	bool getDisplaySphere() { return m_displaySphere; }
	bool getDisplayArray() { return m_displayArray; }
	float getDisplaySize() const { return m_displaySize; }


private:
	static const glm::vec2 WINDOW_SIZE;

	Renderer& m_renderer;
	MaterialData& m_materials;

	LabelNode* m_materialLabel;
	SliderNode* m_materialSlider;
	SpriteNode* m_materialSprite;
	SpriteNode* m_materialSelectorSprite;
	ButtonNode* m_pickerButton;
	MaterialPicker* m_materialPicker;

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

	ButtonNode* m_colorCubeButton;
	ButtonNode* m_sphereButton;
	ButtonNode* m_arrayButton;

	LabelNode* m_sizeLabel;
	SliderNode* m_sizeSlider;

	uint8_t m_currentID;
	Color m_currentColor;
	float m_currentRoughness;
	float m_currentMetalness;
	float m_currentEmissiveness;

	bool m_displayColorCube;
	bool m_displaySphere;
	bool m_displayArray;

	float m_displaySize;

	void setMaterialID(uint8_t materialID);
	void updateColorText();

	void setupMaterialSliders(float& positionY);
	void removeMaterialSliders();
	
	void setupColorSliders(float& positionY);
	void removeColorSliders();

	void setupButtons(float& positionY);

	void onMaterialPickerButton(bool state);
	void refresh();
};

