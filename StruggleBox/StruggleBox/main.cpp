#include "CoreDefines.h"
#include "Game/MainMenu.h"
#include "Renderer.h"
#include "Options.h"
#include "Injector.h"
#include "SceneManager.h"

#include "EngineCore.h"
#include "FreeListAllocator.h"

int main(int argc, const char* argv[])
{
	 unsigned char* heap = (unsigned char*)malloc(HEAP_SIZE); // This is the heap memory for the entire application

	 EngineCore& core = EngineCore::create(argc, argv, heap);
	 core.setTitle("StruggleBox");
	 core.initialize();
	 MainMenu& mainMenu = core.getGlobalInjector().instantiateUnmapped<MainMenu, Injector, Allocator, Renderer, SceneManager, Options>();
	 core.runScene(mainMenu);
	 int result = core.terminate();

	 CUSTOM_DELETE(&mainMenu, core.getGlobalAllocator());
	 free(heap);

	 return result;
}

