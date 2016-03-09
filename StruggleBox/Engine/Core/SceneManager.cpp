#include "SceneManager.h"
#include "Locator.h"
#include "Scene.h"
#include "HyperVisor.h"
#include <assert.h>

#define DEBUGSCENES 0

SceneManager::SceneManager(Locator& locator) :
_locator(locator)
{ }

SceneManager::~SceneManager()
{
    // Drop all active scenes
    while( !mStack.empty() ) {
        // Retrieve the currently active scene
        Scene* aScene = mStack.back();
        
        // Pop the currently active scene off the stack
        mStack.pop_back();
        
        // Pause the currently active scene
        aScene->Pause();
        
        // De-initialize the scene
        aScene->Release();
        
        // Just delete the scene now
        delete aScene;
        
        // Don't keep pointers around we don't need
        aScene = NULL;
    }
    
    // Delete all our dropped scenes
    while( !mDead.empty() ) {
        // Retrieve the currently active scene
        Scene* aScene = mDead.back();
        
        // Pop the currently active scene off the stack
        mDead.pop_back();
        
        // Pause the currently active scene
        aScene->Pause();
        
        // De-initialize the scene
        aScene->Release();
                
        // Just delete the scene now
        delete aScene;
        
        // Don't keep pointers around we don't need
        aScene = NULL;
    }
}

bool SceneManager::IsEmpty()
{
    return mStack.empty();
}

void SceneManager::AddActiveScene(Scene* theScene)
{
    // Check that they didn't provide a bad pointer
    assert(NULL != theScene && "SceneManagerager::AddActiveScene() received a bad pointer");
#if DEBUGSCENES
    printf("SceneManager: adding active scene - %s\n", theScene->GetID().c_str() );
#endif
    // Is there a scene currently running? then Pause it
    if(!mStack.empty()) {
        // Pause the currently running scene since we are changing the
        // currently active scene to the one provided
        mStack.back()->Pause();
    }
    
    // Add the active scene
    mStack.push_back(theScene);
    
    // Initialize the new active scene
    mStack.back()->Initialize();
}

void SceneManager::AddInactiveScene(Scene* theScene)
{
    // Check that they didn't provide a bad pointer
    assert(NULL != theScene && "SceneManagerager::AddInactiveScene() received a bad pointer");
#if DEBUGSCENES
    printf("SceneManager: adding inactive scene - %s\n", theScene->GetID().c_str() );
#endif
    // Add the inactive scene to the bottom of the stack
    mStack.insert(mStack.begin(), theScene);
}

Scene& SceneManager::GetActiveScene()
{
    if ( mStack.empty() ) {
        printf("[SceneManager] WARNING: no active scene to get!");
    }
    return *mStack.back();
}

void SceneManager::InactivateActiveScene()
{
#if DEBUGSCENES
    printf("SceneManager: inactivating active scene\n" );
#endif
    // Is there no currently active scene to drop?
    if(!mStack.empty()) {
        // Retrieve the currently active scene
        Scene* aScene = mStack.back();
        
        // Pause the currently active scene
        aScene->Pause();
        
        // Pop the currently active scene off the stack
        mStack.pop_back();
        
        // Move this now inactive scene to the absolute back of our stack
        mStack.insert(mStack.begin(), aScene);
        
        // Don't keep pointers around we don't need anymore
        aScene = NULL;
    } else {
        // Quit the application with an error status response
        if(_locator.Satisfies<HyperVisor>()) {
            printf("SceneManager: stack was empty on inactivation\n");
            _locator.Get<HyperVisor>()->Stop();
        }
        return;
    }
    
    // Is there another scene to activate? then call Resume to activate it
    if(!mStack.empty()) {
        // Has this scene ever been initialized?
        if( mStack.back()->IsInitialized() ) {
            // Resume the new active scene
            mStack.back()->Resume();
        }
        else {
            // Initialize the new active scene
            mStack.back()->Initialize();
        }
    }
}

void SceneManager::DropActiveScene()
{
#if DEBUGSCENES
    printf("SceneManager: dropping active scene\n" );
#endif
    // Is there no currently active scene to drop?
    if( !mStack.empty() ) {
        // Retrieve the currently active scene
        Scene* aScene = mStack.back();
        
        // Pause the currently active scene
        aScene->Pause();
        
        // Deinit currently active scene before we pop it off the stack
        // (HandleCleanup() will be called by Scene::DoInit() method if this
        // scene is ever set active again)
        aScene->Release();
        
        // Pop the currently active scene off the stack
        mStack.pop_back();
        
        // Move this now inactive scene to the absolute back of our stack
        mStack.insert(mStack.begin(), aScene);
        
        // Don't keep pointers around we don't need anymore
        aScene = NULL;
    } else {
        // Quit the application with an error status response
        if(_locator.Satisfies<HyperVisor>()) {
            printf("SceneManager: stack was empty on scene drop\n");
            _locator.Get<HyperVisor>()->Stop();
        }
        return;
    }
    
    // Is there another scene to activate? then call Resume to activate it
    if( !mStack.empty() ) {
        // Has this scene ever been initialized?
        if( mStack.back()->IsInitialized() ) {
            // Resume the new active scene
            mStack.back()->Resume();
        } else {
            // Initialize the new active scene
            mStack.back()->Initialize();
        }
    }
}

void SceneManager::ResetActiveScene()
{
#if DEBUGSCENES
    printf("SceneManager: resetting active scene\n" );
#endif
    // Is there no currently active scene to reset?
    if(!mStack.empty()) {
        // Retrieve the currently active scene
        Scene* aScene = mStack.back();
                
        // Pause the currently active scene
        aScene->Pause();
        
        // Call the ReInit method to Reset the currently active scene
        aScene->ReInitialize();
        
        // Resume the currently active scene
        aScene->Resume();
        
        // Don't keep pointers around we don't need anymore
        aScene = NULL;
    } else {
        // Quit the application with an error status response
        if(_locator.Satisfies<HyperVisor>()) {
            printf("SceneManager: stack was empty on scene reset\n");
            _locator.Get<HyperVisor>()->Stop();
        }
        return;
    }
}

void SceneManager::RemoveActiveScene()
{
#if DEBUGSCENES
    printf("SceneManager: remove active scene\n" );
#endif
    // Is there no currently active scene to drop?
    if( !mStack.empty() ) {
        // Retrieve the currently active scene
        Scene* aScene = mStack.back();
        
        // Pause the currently active scene
        aScene->Pause();
        
        // Deinitialize the currently active scene before we pop it off the stack
        aScene->Release();
        
        // Pop the currently active scene off the stack
        mStack.pop_back();
        
        // Move this scene to our dropped stack
        mDead.push_back(aScene);
        
        // Don't keep pointers around we don't need anymore
        aScene = NULL;
    } else {
        // Quit the application with an error status response
        if(_locator.Satisfies<HyperVisor>()) {
            printf("SceneManager: stack was empty on scene removal\n");
            _locator.Get<HyperVisor>()->Stop();
        }
        return;
    }
    
    // Is there another scene to activate? then call Resume to activate it
    if( !mStack.empty() ) {
        // Has this scene ever been initialized?
        if(mStack.back()->IsInitialized()) {
            // Resume the new active scene
            mStack.back()->Resume();
        } else {
            // Initialize the new active scene
            mStack.back()->Initialize();
        }
    }
}

void SceneManager::SetActiveScene(std::string theSceneID) {
#if DEBUGSCENES
    printf("SceneManager: setting active scene - %s\n", theSceneID.c_str() );
#endif
    std::vector<Scene*>::iterator it;
    
    // Find the scene that matches theSceneID
    for( it=mStack.begin(); it != mStack.end(); it++ ) {
        // Does this scene match theSceneID? then activate it as the new
        // currently active scene
        if( (*it)->GetID() == theSceneID ) {
            // Get a pointer to soon to be currently active scene
            Scene* aScene = *it;
            
            // Erase it from the list of previously active scenes
            mStack.erase(it);
            
            // Is there a scene currently running? then Pause it
            if( !mStack.empty() ) {
                // Pause the currently running scene since we are changing the
                // currently active scene to the one specified by theSceneID
                mStack.back()->Pause();
            }
            
            // Add the new active scene
            mStack.push_back(aScene);
            
            // Don't keep pointers we don't need around
            aScene = NULL;
            
            // Has this scene ever been initialized?
            if( mStack.back()->IsInitialized() ) {
                // Resume the new active scene
                mStack.back()->Resume();
            } else {
                // Initialize the new active scene
                mStack.back()->Initialize();
            }
            
            // Exit our find loop
            break;
        } // if((*it)->GetID() == theSceneID)
    } // for(it=mStack.begin(); it < mStack.end(); it++)
}

void SceneManager::Cleanup(void)
{
    // Remove one of our dead scenes
    if( !mDead.empty() ) {
        // Retrieve the dead scene
        Scene* aScene = mDead.back();
        assert(NULL != aScene && "SceneManagerager::HandleCleanup() invalid dropped scene pointer");
#if DEBUGSCENES
        printf("SceneManager: cleaning up dead scene - %s\n", aScene->GetID().c_str() );
#endif
        
        // Pop the dead scene off the stack
        mDead.pop_back();
        
        // Call the DeInit if it hasn't been called yet
        if( aScene->IsInitialized() ) {
            aScene->Release();
        }
        
        // Just delete the scene now
        delete aScene;
        
        // Don't keep pointers around we don't need
        aScene = NULL;
    }
    
    // Make sure we still have an active scene
    if( NULL == mStack.back() ) {
        // There are no scenes on the stack, exit the program
        if(_locator.Satisfies<HyperVisor>()) {
            printf("SceneManager: stack was empty on cleanup\n");
            _locator.Get<HyperVisor>()->Stop();
        }
    }
}
std::string SceneManager::GetPreviousSceneName(void) {
    if ( mStack.size() > 1 ) {
        std::string theSceneID = (*(mStack.begin()+mStack.size()-2))->GetID();
        return theSceneID;
    }
    return "";
}
void SceneManager::KillPreviousScene(void) {
    if ( mStack.size() > 1 ) {
        // Retrieve the currently active scene
        Scene* aScene = mStack.back();
        // Pop the currently active scene off the stack
        mStack.pop_back();
        // Retrieve the previous scene
        Scene* bScene = mStack.back();
        // Pop the previous scene off the stack
        mStack.pop_back();
        // Pause the currently active scene
        bScene->Pause();
        // Deinitialize the currently active scene before we pop it off the stack
        bScene->Release();
        // Move this scene to our dropped stack
        mDead.push_back(bScene);
        // Put current scene back
        mStack.push_back(aScene);
    }
}

size_t SceneManager::NumScenes(void) {
    return mStack.size();
}
