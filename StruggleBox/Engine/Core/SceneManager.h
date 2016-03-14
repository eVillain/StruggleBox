#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <vector>
#include <string>

class Locator;
class Scene;

class SceneManager
{
public:
    SceneManager(Locator& locator);
    virtual ~SceneManager();

    void AddActiveScene(Scene* theScene);
    void AddInactiveScene(Scene* theScene);
    Scene& GetActiveScene();
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
private:
    Locator& _locator;
    // Stack to store the current and previously active scenes
    std::vector<Scene*>   _stack;
    // Stack to store the dead scenes until they properly cleaned up
    std::vector<Scene*>   _dead;
    
    SceneManager(const SceneManager&);              // Intentionally undefined
    SceneManager& operator=(const SceneManager&);   // Intentionally undefined
};
#endif
