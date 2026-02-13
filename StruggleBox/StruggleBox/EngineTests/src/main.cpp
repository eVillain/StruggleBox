#include "Allocator.h"
#include "Injector.h"
#include "EngineCore.h"
#include "RenderTestScene.h"

#include "TestsMenu.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "Renderer3DDeferred.h"
#include "VoxelRenderer.h"
#include "ProxyAllocator.h"
#include "RenderCore.h"
#include "Input.h"
#include "OSWindow.h"
#include "StatTracker.h"
#include "Options.h"

int main(int argc, char* argv[])
{
	const size_t HEAP_SIZE = 512 * 1024 * 1024;
	unsigned char* heap = (unsigned char*)malloc(HEAP_SIZE); // This is the heap memory for the entire application

	EngineCore& core = EngineCore::create(argc, const_cast<const char**>(argv), heap);
	core.setTitle("Engine Tests");
	core.initialize();
	
	Renderer2D* renderer2D = CUSTOM_NEW(Renderer2D, core.getRendererAllocator())(core.getGlobalInjector().getInstance<RenderCore>(), core.getRendererAllocator());
	core.getGlobalInjector().mapInstance<Renderer2D>(*renderer2D);
	renderer2D->initialize();

	Renderer3D* renderer3D = CUSTOM_NEW(Renderer3D, core.getRendererAllocator())(core.getGlobalInjector().getInstance<RenderCore>(), core.getRendererAllocator());
	core.getGlobalInjector().mapInstance<Renderer3D>(*renderer3D);
	renderer3D->initialize();

	//Renderer3DDeferred* renderer3DDeferred = CUSTOM_NEW(Renderer3DDeferred, core.getRendererAllocator())(core.getGlobalInjector().getInstance<RenderCore>(),
	//	core.getRendererAllocator(), core.getGlobalInjector().getInstance<Options>());
	//core.getGlobalInjector().mapInstance<Renderer3DDeferred>(*renderer3DDeferred);
	//renderer3DDeferred->initialize();

	ProxyAllocator& rendererAllocator = core.getRendererAllocator();
	VoxelRenderer* voxelRenderer = CUSTOM_NEW(VoxelRenderer, rendererAllocator)(core.getGlobalInjector().getInstance<RenderCore>(), rendererAllocator, core.getGlobalInjector().getInstance<Options>());
	core.getGlobalInjector().mapInstance<VoxelRenderer>(*voxelRenderer);
	core.getGlobalInjector().mapInterfaceToType<Renderer3DDeferred, VoxelRenderer>();
	voxelRenderer->initialize();

	TestsMenu& testsMenu = core.getGlobalInjector().instantiateUnmapped<TestsMenu, Injector, Allocator, Renderer2D, Input, OSWindow, Options, StatTracker>();
	core.runScene(testsMenu);
	CUSTOM_DELETE(&testsMenu, core.getGlobalAllocator());

	voxelRenderer->terminate();
	CUSTOM_DELETE(voxelRenderer, core.getRendererAllocator());
	//renderer3DDeferred->terminate();
	//CUSTOM_DELETE(renderer3DDeferred, core.getRendererAllocator());
	renderer3D->terminate();
	CUSTOM_DELETE(renderer3D, core.getRendererAllocator());
	renderer2D->terminate();
	CUSTOM_DELETE(renderer2D, core.getRendererAllocator());

	core.getGlobalInjector().unmap<VoxelRenderer>();
	core.getGlobalInjector().unmap<Renderer3DDeferred>();
	core.getGlobalInjector().unmap<Renderer3D>();
	core.getGlobalInjector().unmap<Renderer2D>();

	int result = core.terminate();
	EngineCore::destroy(core);

	free(heap);

	return result;
}