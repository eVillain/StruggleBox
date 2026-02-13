#include "Allocator.h"
#include "Injector.h"
#include "EngineCore.h"

#include "Renderer2DDeferred.h"
#include "RenderCore.h"
#include "ProxyAllocator.h"
#include "MainMenu.h"
#include "Input.h"
#include "OSWindow.h"
#include "StatTracker.h"
#include "Options.h"

int main(int argc, char* argv[])
{
	const size_t HEAP_SIZE = 128 * 1024 * 1024;
	unsigned char* heap = (unsigned char*)malloc(HEAP_SIZE); // This is the heap memory for the entire application

	EngineCore& core = EngineCore::create(argc, const_cast<const char**>(argv), heap);
	core.setTitle("Engine Tests");
	core.initialize();

	RenderCore& renderCore = core.getGlobalInjector().getInstance<RenderCore>();
	Options& options = core.getGlobalInjector().getInstance<Options>();
	Renderer2DDeferred* renderer2D = CUSTOM_NEW(Renderer2DDeferred, core.getRendererAllocator())(renderCore, core.getRendererAllocator(), options);
	core.getGlobalInjector().mapInstance<Renderer2DDeferred>(*renderer2D);
	renderer2D->initialize();

	MainMenu& mainMenu = core.getGlobalInjector().instantiateUnmapped<MainMenu, Injector, Allocator, RenderCore, Input, OSWindow, Options, StatTracker>();
	core.runScene(mainMenu);
	CUSTOM_DELETE(&mainMenu, core.getGlobalAllocator());

	renderer2D->terminate();
	CUSTOM_DELETE(renderer2D, core.getRendererAllocator());
	core.getGlobalInjector().unmap<Renderer2DDeferred>();

	int result = core.terminate();
	EngineCore::destroy(core);

	free(heap);

	return result;
}