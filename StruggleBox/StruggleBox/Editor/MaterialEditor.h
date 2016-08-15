#ifndef MATERIAL_EDITOR_H
#define MATERIAL_EDITOR_H

#include "EditorScene.h"
#include "Color.h"
#include "GFXDefines.h"
#include "Light3D.h"
#include "tb_select_item.h"
#include <memory>

class LightSystem3D;
class TBGUI;
class SceneManager;
class MaterialsWindow;
class Mesh;

class MaterialEditor : public EditorScene
{
public:
	MaterialEditor(
		std::shared_ptr<TBGUI> gui,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<Options> options,
		std::shared_ptr<Input> input,
		std::shared_ptr<LightSystem3D> lights,
		std::shared_ptr<SceneManager> sceneManager);

	void Initialize();
	void ReInitialize();
	void Release();
	void Pause();
	void Resume();
	void Update(const double deltaTime);

	void ShowEditor();
	void RemoveEditor();
	void Draw();

private:
	std::shared_ptr<LightSystem3D> _lighting;
	std::shared_ptr<SceneManager> _sceneManager;

	tb::TBGenericStringItemSource _file_menu_source;

	MaterialsWindow* _materialsWindow;

	void loadMaterials(const std::string& fileName);
	void saveMaterials(const std::string& fileName);

	bool OnEvent(const std::string& theEvent,
		const float& amount);
	bool OnMouse(const glm::ivec2& coord);
};

#endif
