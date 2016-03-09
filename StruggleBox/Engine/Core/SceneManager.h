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
    Scene& GetActiveScene(void);
    void InactivateActiveScene(void);
    void DropActiveScene(void);
    void ResetActiveScene(void);
    void RemoveActiveScene(void);
    void SetActiveScene(std::string theSceneID);
    void Cleanup(void);
    bool IsEmpty(void);
    std::string GetPreviousSceneName(void);
    void KillPreviousScene(void);
    size_t NumScenes(void);
private:
    Locator& _locator;
    // Stack to store the current and previously active scenes
    std::vector<Scene*>   mStack;
    // Stack to store the dead scenes until they properly cleaned up
    std::vector<Scene*>   mDead;
    
    SceneManager(const SceneManager&);              // Intentionally undefined
    SceneManager& operator=(const SceneManager&);   // Intentionally undefined
};
#endif
