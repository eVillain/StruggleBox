#pragma once

#include "WindowNode.h"
#include <functional>
#include <memory>

class MaterialData;
class VoxelData;
class MaterialPicker;
class ButtonNode;
class SpriteNode;

enum ObjectTool {
	ObjectTool_None = 0,        // No tool selected
	ObjectTool_Block = 1,       // Single block editing
	ObjectTool_Column = 2,      // Column of blocks
	ObjectTool_Selection = 3,   // Select volume
};

class ObjectWindow : public WindowNode
{
public:
	ObjectWindow(
		const GUI& gui,
		Renderer& renderer, 
		MaterialData& materials);

	void setVoxels(VoxelData* voxels);
	void setResizeCallback(const std::function<void(const glm::ivec3&)>& cb) { m_resizeCallback = cb; }
	void setRescaleCallback(const std::function<void(float)>& cb) { m_rescaleCallback = cb; }
	void setUpdateCallback(const std::function<void()>& cb) { m_updateCallback = cb; }
	void setCurrentID(const uint8_t newID);
	const uint8_t getCurrentID() { return m_currentID; }
	const ObjectTool getObjectTool() { return m_editTool; }

private:
	static const glm::vec2 WINDOW_SIZE;

	Renderer& m_renderer;
	MaterialData& m_materials;
	VoxelData* m_voxels;
	uint8_t m_currentID;
	ObjectTool m_editTool;

	std::function<void(const glm::ivec3&)> m_resizeCallback;
	std::function<void(float)> m_rescaleCallback;
	std::function<void()> m_updateCallback;

	LabelNode* m_materialLabel;
	SpriteNode* m_materialSprite;
	ButtonNode* m_pickerButton;
	MaterialPicker* m_materialPicker;

	LabelNode* m_voxelSizeLabel;
	ButtonNode* m_voxelSizeIncXButton;
	ButtonNode* m_voxelSizeDecXButton;
	ButtonNode* m_voxelSizeIncYButton;
	ButtonNode* m_voxelSizeDecYButton;
	ButtonNode* m_voxelSizeIncZButton;
	ButtonNode* m_voxelSizeDecZButton;

	LabelNode* m_voxelScaleLabel;
	ButtonNode* m_voxelScaleIncButton;
	ButtonNode* m_voxelScaleDecButton;

	ButtonNode* m_clearButton;
	ButtonNode* m_fillButton;
	ButtonNode* m_generateTreeButton;

	void setupMaterialOptions();
	void setupVoxelOptions();

	void onMaterialPickerButton(bool state);
	void refresh();
};
