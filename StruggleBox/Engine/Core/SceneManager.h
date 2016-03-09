#ifndef NGN_SCENE_MANAGER_H
#define NGN_SCENE_MANAGER_H
//
//  SceneManager.h
//  NeverEngine
//
//  Created by Ville-Veikko Urrila on 8/10/12.
//  Copyright (c) 2012 The Drudgerist. All rights reserved.
//
#include <vector>
#include <string>
#include "CoreTypes.h"

class SceneManager {
public:
    SceneManager( HyperVisor* hv );
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
    // Pointer to the HyperVisor class
    HyperVisor*                 hyperVisor;
    // Stack to store the current and previously active scenes
    std::vector<Scene*>   mStack;
    // Stack to store the dead scenes until they properly cleaned up
    std::vector<Scene*>   mDead;
    
    SceneManager(const SceneManager&);              // Intentionally undefined
    SceneManager& operator=(const SceneManager&);   // Intentionally undefined
};
#endif
