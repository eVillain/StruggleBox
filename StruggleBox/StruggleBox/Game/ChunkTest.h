#pragma once

#include "Color.h"
#include "EditorScene.h"
#include "GFXDefines.h"
#include "MaterialData.h"
#include <memory>

class Allocator;
class SceneManager;
class MaterialsWindow;

class ChunkTest : public EditorScene
{
public:
	ChunkTest(
		Allocator& allocator,
		VoxelRenderer& renderer,
		RenderCore& renderCore,
		OSWindow& osWindow,
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

	void Draw();

	const std::string getFileType() const override { return "*.plist"; }
	const std::string getFilePath() const override;
	void onFileLoad(const std::string& file) override { loadMaterials(file); }
	void onFileSave(const std::string& file) override { saveMaterials(file); }

private:
	SceneManager& m_sceneManager;

	MaterialData m_materialData;

	ShaderID m_voxelMeshShaderID;
	std::vector<DrawDataID> m_voxelMeshDrawDataIDs;

	void loadMaterials(const std::string& fileName);
	void saveMaterials(const std::string& fileName);

	bool OnEvent(const InputEvent event, const float amount);
	bool OnMouse(const glm::ivec2& coord);
};
