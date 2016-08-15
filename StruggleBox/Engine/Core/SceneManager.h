#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <vector>
#include <string>
#include <functional>
#include <memory>

class Scene;

class SceneManager
{
public:
	SceneManager();
    virtual ~SceneManager();

    void AddActiveScene(std::shared_ptr<Scene> scene);
    void AddInactiveScene(std::shared_ptr<Scene> scene);
	std::shared_ptr<Scene> GetActiveScene();

    void InactivateActiveScene();
    void DropActiveScene();
    void ResetActiveScene();
    void RemoveActiveScene();
    void SetActiveScene(std::string theSceneID);
    void Cleanup();
    bool IsEmpty();
    std::string GetPreviousSceneName();
    void KillPreviousScene();
    size_t NumScenes();

	void SetHaltFunction(std::function<void(const std::string)> haltFunc);
private:
    // Stack to store the current and previously active scenes
    std::vector<std::shared_ptr<Scene>>   _stack;
    // Stack to store the dead scenes until they properly cleaned up
    std::vector<std::shared_ptr<Scene>>   _dead;
    // Halt function to execute when out of scenes and engine must terminate
	std::function<void(const std::string)> _haltFunc;

	void halt(const std::string& error);

    SceneManager(const SceneManager&);              // Intentionally undefined
    SceneManager& operator=(const SceneManager&);   // Intentionally undefined
};
#endif
