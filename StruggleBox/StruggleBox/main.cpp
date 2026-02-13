#include "CoreDefines.h"
#include "Game/MainMenu.h"
#include "Renderer2D.h"
#include "VoxelRenderer.h"
#include "Options.h"
#include "Injector.h"
#include "SceneManager.h"
#include "ThreadPool.h"
#include "ProxyAllocator.h"
#include "EngineCore.h"
#include "RenderCore.h"
#include "StatTracker.h"

int main(int argc, const char* argv[])
{
	 unsigned char* heap = (unsigned char*)malloc(HEAP_SIZE); // This is the heap memory for the entire application

	 EngineCore& core = EngineCore::create(argc, argv, heap);
	 core.setTitle("StruggleBox");
	 core.initialize();

	 Options& options = core.getGlobalInjector().getInstance<Options>();
	 ThreadPool& threadPool = core.getGlobalInjector().getInstance<ThreadPool>();
	 StatTracker& statTracker = core.getGlobalInjector().getInstance<StatTracker>();
	 RenderCore& renderCore = core.getGlobalInjector().getInstance<RenderCore>();

	 Renderer2D* renderer2D = CUSTOM_NEW(Renderer2D, core.getRendererAllocator())(core.getGlobalInjector().getInstance<RenderCore>(), core.getRendererAllocator());
	 core.getGlobalInjector().mapInstance<Renderer2D>(*renderer2D);
	 renderer2D->initialize();

	 VoxelRenderer* renderer3DDeferred = CUSTOM_NEW(VoxelRenderer, core.getRendererAllocator())(core.getGlobalInjector().getInstance<RenderCore>(),
		 core.getRendererAllocator(), core.getGlobalInjector().getInstance<Options>());
	 core.getGlobalInjector().mapInstance<VoxelRenderer>(*renderer3DDeferred);
	 //core.getGlobalInjector().mapInterfaceToType<Renderer3DDeferred, VoxelRenderer>();
	 renderer3DDeferred->initialize();

	 MainMenu& mainMenu = core.getGlobalInjector().instantiateUnmapped<MainMenu, Injector, Allocator, Renderer2D, SceneManager, Options>();
	 core.runScene(mainMenu);
	 CUSTOM_DELETE(&mainMenu, core.getGlobalAllocator());
	 
	 renderer3DDeferred->terminate();
	 CUSTOM_DELETE(renderer3DDeferred, core.getRendererAllocator());
	 renderer2D->terminate();
	 CUSTOM_DELETE(renderer2D, core.getRendererAllocator());
	
	 core.getGlobalInjector().unmap<VoxelRenderer>();
	 core.getGlobalInjector().unmap<Renderer2D>();

	 int result = core.terminate();
	 EngineCore::destroy(core);

	 free(heap);

	 return result;
}

