#ifndef MATERIAL_EDITOR_H
#define MATERIAL_EDITOR_H

#include "EditorScene.h"
#include "Color.h"
#include "GFXDefines.h"
#include <memory>

class Allocator;
class SceneManager;
class MaterialsWindow;
class Mesh;

class MaterialEditor : public EditorScene
{
public:
	MaterialEditor(
		Camera& camera,
		Allocator& allocator,
		Renderer& renderer,
		Options& options,
		Input& input,
		SceneManager& sceneManager,
		StatTracker& statTracker);

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
	SceneManager& m_sceneManager;

	MaterialsWindow* m_materialsWindow;

	void loadMaterials(const std::string& fileName);
	void saveMaterials(const std::string& fileName);

	bool OnEvent(const InputEvent event, const float amount);
	bool OnMouse(const glm::ivec2& coord);
};

#endif
