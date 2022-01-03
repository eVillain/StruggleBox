#include "SceneManager.h"
#include "Scene.h"
#include "Log.h"
#include <assert.h>

#define DEBUGSCENES 0

SceneManager::SceneManager()
{
	Log::Info("[SceneManager] constructor, instance at %p", this);
}

SceneManager::~SceneManager()
{
	Log::Info("[SceneManager] destructor, instance at %p", this);
}

void SceneManager::initialize()
{
}

void SceneManager::terminate()
{
    // Drop all active scenes
    while (!_stack.empty()) {
        // Retrieve the currently active scene
        Scene* aScene = _stack.back();

        // Pop the currently active scene off the stack
        _stack.pop_back();

        // Pause the currently active scene
        aScene->Pause();

        // De-initialize the scene
        aScene->Release();
    }

    // Delete all our dropped scenes
    while (!_dead.empty()) {
        // Retrieve the currently active scene
        Scene* aScene = _dead.back();

        // Pop the currently active scene off the stack
        _dead.pop_back();

        // Pause the currently active scene
        aScene->Pause();

        // De-initialize the scene
        aScene->Release();
    }
}

bool SceneManager::IsEmpty()
{
    return _stack.empty();
}

void SceneManager::AddActiveScene(Scene* theScene)
{
    // Check that they didn't provide a bad pointer
    assert(NULL != theScene && "SceneManagerager::AddActiveScene() received a bad pointer");
#if DEBUGSCENES
    printf("SceneManager: adding active scene - %s\n", theScene->GetID().c_str() );
#endif
    // Is there a scene currently running? then Pause it
    if(!_stack.empty()) {
        // Pause the currently running scene since we are changing the
        // currently active scene to the one provided
        _stack.back()->Pause();
    }
    
    // Add the active scene
    _stack.push_back(theScene);
    
    // Initialize the new active scene
    _stack.back()->Initialize();
}

void SceneManager::AddInactiveScene(Scene* theScene)
{
    // Check that they didn't provide a bad pointer
    assert(NULL != theScene && "SceneManagerager::AddInactiveScene() received a bad pointer");
#if DEBUGSCENES
    printf("SceneManager: adding inactive scene - %s\n", theScene->GetID().c_str() );
#endif
    // Add the inactive scene to the bottom of the stack
    _stack.insert(_stack.begin(), theScene);
}

Scene* SceneManager::GetActiveScene()
{
    if ( _stack.empty() ) {
        printf("[SceneManager] WARNING: no active scene to get!");
    }
    return _stack.back();
}

void SceneManager::InactivateActiveScene()
{
#if DEBUGSCENES
    printf("SceneManager: inactivating active scene\n" );
#endif
    // Is there no currently active scene to drop?
    if(!_stack.empty()) {
        // Retrieve the currently active scene
		Scene* aScene = _stack.back();
        
        // Pause the currently active scene
        aScene->Pause();
        
        // Pop the currently active scene off the stack
        _stack.pop_back();
        
        // Move this now inactive scene to the absolute back of our stack
        _stack.insert(_stack.begin(), aScene);
    } else {
        // Quit the application with an error status response
        halt("SceneManager: stack was empty on inactivation");
		return;
    }
    
    // Is there another scene to activate? then call Resume to activate it
    if(!_stack.empty()) {
        // Has this scene ever been initialized?
        if( _stack.back()->IsInitialized() ) {
            // Resume the new active scene
            _stack.back()->Resume();
        }
        else {
            // Initialize the new active scene
            _stack.back()->Initialize();
        }
    }
}

void SceneManager::DropActiveScene()
{
#if DEBUGSCENES
    printf("SceneManager: dropping active scene\n" );
#endif
    // Is there no currently active scene to drop?
    if( !_stack.empty() ) {
        // Retrieve the currently active scene
		Scene* aScene = _stack.back();
        
        // Pause the currently active scene
        aScene->Pause();
        
        // Deinit currently active scene before we pop it off the stack
        // (HandleCleanup() will be called by Scene::DoInit() method if this
        // scene is ever set active again)
        aScene->Release();
        
        // Pop the currently active scene off the stack
        _stack.pop_back();
        
        // Move this now inactive scene to the absolute back of our stack
        _stack.insert(_stack.begin(), aScene);
    } else {
        // Quit the application with an error status response
		halt("SceneManager: stack was empty on scene drop");
        return;
    }
    
    // Is there another scene to activate? then call Resume to activate it
    if( !_stack.empty() ) {
        // Has this scene ever been initialized?
        if( _stack.back()->IsInitialized() ) {
            // Resume the new active scene
            _stack.back()->Resume();
        } else {
            // Initialize the new active scene
            _stack.back()->Initialize();
        }
    }
}

void SceneManager::ResetActiveScene()
{
#if DEBUGSCENES
    printf("SceneManager: resetting active scene\n" );
#endif
    // Is there no currently active scene to reset?
    if(!_stack.empty()) {
        // Retrieve the currently active scene
		Scene* aScene = _stack.back();
                
        // Pause the currently active scene
        aScene->Pause();
        
        // Call the ReInit method to Reset the currently active scene
        aScene->ReInitialize();
        
        // Resume the currently active scene
        aScene->Resume();
    } else {
        // Quit the application with an error status response
		halt("SceneManager: stack was empty on scene reset");
        return;
    }
}

void SceneManager::RemoveActiveScene()
{
#if DEBUGSCENES
    printf("SceneManager: remove active scene\n" );
#endif
    // Is there no currently active scene to drop?
    if( !_stack.empty() ) {
        // Retrieve the currently active scene
		Scene* aScene = _stack.back();
        
        // Pause the currently active scene
        aScene->Pause();
        
        // Deinitialize the currently active scene before we pop it off the stack
        aScene->Release();
        
        // Pop the currently active scene off the stack
        _stack.pop_back();
        
        // Move this scene to our dropped stack
        _dead.push_back(aScene);
    } else {
        // Quit the application with an error status response
		halt("SceneManager: stack was empty on scene removal");
        return;
    }
    
    // Is there another scene to activate? then call Resume to activate it
    if( !_stack.empty() ) {
        // Has this scene ever been initialized?
        if(_stack.back()->IsInitialized()) {
            // Resume the new active scene
            _stack.back()->Resume();
        } else {
            // Initialize the new active scene
            _stack.back()->Initialize();
        }
    }
}

void SceneManager::SetActiveScene(std::string theSceneID) {
#if DEBUGSCENES
    printf("SceneManager: setting active scene - %s\n", theSceneID.c_str() );
#endif
    std::vector<Scene*>::iterator it;
    
    // Find the scene that matches theSceneID
    for( it=_stack.begin(); it != _stack.end(); it++ ) {
        // Does this scene match theSceneID? then activate it as the new
        // currently active scene
        if( (*it)->GetID() == theSceneID ) {
            // Get a pointer to soon to be currently active scene
			Scene* aScene = *it;
            
            // Erase it from the list of previously active scenes
            _stack.erase(it);
            
            // Is there a scene currently running? then Pause it
            if( !_stack.empty() ) {
                // Pause the currently running scene since we are changing the
                // currently active scene to the one specified by theSceneID
                _stack.back()->Pause();
            }
            
            // Add the new active scene
            _stack.push_back(aScene);
            
            // Has this scene ever been initialized?
            if( _stack.back()->IsInitialized() ) {
                // Resume the new active scene
                _stack.back()->Resume();
            } else {
                // Initialize the new active scene
                _stack.back()->Initialize();
            }
            
            // Exit our find loop
            break;
        } // if((*it)->GetID() == theSceneID)
    } // for(it=_stack.begin(); it < _stack.end(); it++)
}

void SceneManager::Cleanup(void)
{
    // Remove one of our dead scenes
    if( !_dead.empty() ) {
        // Retrieve the dead scene
		Scene* aScene = _dead.back();
        assert(NULL != aScene && "SceneManagerager::HandleCleanup() invalid dropped scene pointer");
#if DEBUGSCENES
        printf("SceneManager: cleaning up dead scene - %s\n", aScene->GetID().c_str() );
#endif
        
        // Pop the dead scene off the stack
        _dead.pop_back();
        
        // Call the DeInit if it hasn't been called yet
        if( aScene->IsInitialized() ) {
            aScene->Release();
        }
    }
    
    // Make sure we still have an active scene
    if( nullptr == _stack.back() ) {
        // There are no scenes on the stack, exit the program
        halt("SceneManager: stack was empty on cleanup\n");
    }
}

std::string SceneManager::GetPreviousSceneName()
{
    if ( _stack.size() > 1 ) {
        std::string theSceneID = (*(_stack.begin()+_stack.size()-2))->GetID();
        return theSceneID;
    }
    return "";
}

void SceneManager::KillPreviousScene()
{
    if ( _stack.size() > 1 ) {
        // Retrieve the currently active scene
		Scene* aScene = _stack.back();
        // Pop the currently active scene off the stack
        _stack.pop_back();
        // Retrieve the previous scene
		Scene* bScene = _stack.back();
        // Pop the previous scene off the stack
        _stack.pop_back();
        // Pause the currently active scene
        bScene->Pause();
        // Deinitialize the currently active scene before we pop it off the stack
        bScene->Release();
        // Move this scene to our dropped stack
        _dead.push_back(bScene);
        // Put current scene back
        _stack.push_back(aScene);
    }
}

size_t SceneManager::NumScenes(void) {
    return _stack.size();
}

void SceneManager::SetHaltFunction(std::function<void(const std::string)> haltFunc)
{
	_haltFunc = haltFunc;
}

void SceneManager::halt(const std::string& error)
{
	if (_haltFunc)
	{
		_haltFunc(error);
	}
	else
	{
		Log::Error(error.c_str());
	}
}
