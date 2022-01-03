#pragma once

#include "EditorScene.h"
#include "EntityManager.h"
#include "Particles.h"
#include "Physics.h"
#include "VoxelFactory.h"
#include "Color.h"
#include "GFXDefines.h"
#include <memory>

class Allocator;
class SceneManager;
class EntityWindow;
class Entity;

class EntityEditor : public EditorScene
{
public:
	EntityEditor(
		Camera& camera,
		Allocator& allocator,
		Renderer& renderer,
		Options& options,
		Input& input,
		SceneManager& sceneManager,
		StatTracker& statTracker);

	void Initialize() override;
	void ReInitialize() override;
	void Pause() override;
	void Resume() override;
	void Update(const double deltaTime) override;

	void ShowEditor();
	void RemoveEditor();
	void Draw() override;

	const std::string getFileType() const override { return "*.plist"; }
	const std::string getFilePath() const override;
	void onFileLoad(const std::string& file) override { loadEntity(file); }
	void onFileSave(const std::string& file) override { saveEntity(file); }

private:
	SceneManager& m_sceneManager;
	Particles m_particles;
	Physics m_physics;
	VoxelFactory m_voxels;
	EntityManager m_entityManager;
	Entity* m_entity;
	EntityID m_entityID;
	EntityWindow* m_entityWindow;

	bool OnEvent(const InputEvent event, const float amount);
	bool OnMouse(const glm::ivec2& coord);

	void loadEntity(const std::string fileName);
	void saveEntity(const std::string fileName);
};
