#ifndef MATERIALS_WINDOW_H
#define MATERIALS_WINDOW_H

#include "Window.h"
#include "MaterialData.h"

class MaterialsWindow : public Window
{
public:
	MaterialsWindow(
		tb::TBWidget* root, 
		MaterialData* materials);
	~MaterialsWindow();

	bool OnEvent(const tb::TBWidgetEvent & ev);
	const int GetCurrentID() { return _currentID; }
	const bool GetRotateMesh() { return _rotateMesh; }
	const bool GetDisplaySphere() { return _displaySphere; }
	const bool GetDisplayArray() { return _displayArray; }
	const bool GetRotateLight() { return _rotateLight; }
	const bool GetColorLight() { return _colorLight; }

	void refresh();
private:
	MaterialData* _materials;
	int _currentID;

	bool _rotateMesh;
	bool _displaySphere;
	bool _displayArray;
	bool _rotateLight;
	bool _colorLight;

	void refreshMaterials();
	void refreshProperties();
};

#endif // !MATERIALS_WINDOW_H