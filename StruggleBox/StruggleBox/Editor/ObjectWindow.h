#ifndef OBJECT_WINDOW_H
#define OBJECT_WINDOW_H

#include "Window.h"
#include <memory>

class MaterialData;
class VoxelData;

enum ObjectTool {
	ObjectTool_None = 0,        // No tool selected
	ObjectTool_Block = 1,       // Single block editing
	ObjectTool_Column = 2,      // Column of blocks
	ObjectTool_Selection = 3,   // Select volume
};

class ObjectWindow : public Window
{
public:
	ObjectWindow(
		tb::TBWidget* root,
		std::shared_ptr<VoxelData> voxels,
		MaterialData* materials);
	~ObjectWindow();

	bool OnEvent(const tb::TBWidgetEvent & ev);
	const int GetCurrentID() { return _currentID; }
	void SetCurrentID(const int newID);

	const bool GetObjectTool() { return _editTool; }
	const bool GetColorLights() { return _colorLights; }

	void refresh();

private:
	std::shared_ptr<VoxelData> _voxels;
	MaterialData* _materials;
	int _currentID;

	ObjectTool _editTool;                // Block or Instance
	bool _colorLights;                   // Test colored lights on object

	void refreshMaterials();
};

#endif