#include "EngineCore.h"

// Core
#include "AppContext.h"
#include "CoreDefines.h"
#include "CoreIncludes.h"
#include "ArenaOperators.h"
#include "FreeListAllocator.h"
#include "ProxyAllocator.h"
#include "Injector.h"
#include "Options.h"
#include "Input.h"
#include "ThreadPool.h"
#include "Log.h"
#include "LogOutputConsole.h"
#include "LogOutputSTD.h"
#include "CommandProcessor.h"
#include "Timer.h"

// Engine
#include "Camera.h"
#include "SceneManager.h"
#include "Scene.h"
#include "RendererGLProg.h"
#include "StatTracker.h"
#include "DebugDraw.h"

EngineCore& EngineCore::create(const int argc, const char* arg[], unsigned char* heap)
{
	Timer::StartRunTime();
	Console::Initialize();

	FreeListAllocator* allocator = new (heap)FreeListAllocator(HEAP_SIZE - sizeof(FreeListAllocator), heap + sizeof(FreeListAllocator));

	constexpr bool USE_STD_OUTPUT = true;
	if (USE_STD_OUTPUT)
	{
		LogOutputSTD* outSTD = CUSTOM_NEW(LogOutputSTD, (*allocator))();
		Log::AttachOutput<LogOutputSTD>(outSTD);
	}
	else
	{
		LogOutputConsole* outConsole = CUSTOM_NEW(LogOutputConsole, (*allocator))();
		Log::AttachOutput<LogOutputConsole>(outConsole);
#ifdef WIN32
		FreeConsole(); // Remove command prompt console - its useless if we never output to it :)
#endif
	}

	Log::Info("EngineCore Created core allocator at %p", allocator);

	Injector& injector = *(CUSTOM_NEW(Injector, (*allocator))(*allocator));
	injector.mapInstance<Injector>(injector);
	injector.mapInstance<Allocator>(*allocator);

	injector.mapSingleton<EngineCore, Injector, Allocator>();
	EngineCore& core = injector.getInstance<EngineCore>();

	return core;
}

EngineCore::~EngineCore()
{
	m_coreInjector.unmap<EngineCore>();
	m_coreInjector.clear();

	CUSTOM_DELETE(&m_coreInjector, m_coreAllocator);
	delete (&m_coreAllocator);
}

void EngineCore::initialize()
{
	initCommandProcessor();
	initApplicationContext();
	initThreadPool();
	initRenderer();
	initInjectorDependencies();

	Input& input = m_coreInjector.getInstance<Input>();
	input.Initialize();
}

void EngineCore::runScene(Scene& scene)
{
	SceneManager& sceneManager = m_coreInjector.getInstance<SceneManager>();
	sceneManager.AddActiveScene(&scene);
	run();
}

int EngineCore::terminate()
{
	Log::Debug("EngineCore shutting down...");
	CommandProcessor::Terminate();

	Input& input = m_coreInjector.getInstance<Input>();
	input.Terminate();

	SceneManager& sceneManager = m_coreInjector.getInstance<SceneManager>();
	sceneManager.terminate();
	m_coreInjector.unmap<SceneManager>();

	m_coreInjector.unmap<DebugDraw>();
	m_coreInjector.unmap<StatTracker>();
	m_coreInjector.getInstance<Renderer>().Terminate();
	m_coreInjector.unmap<Renderer>();
	m_coreInjector.unmap<LightSystem3D>();
	m_coreInjector.unmap<Camera>();


	m_coreInjector.unmap<ThreadPool>();

	m_coreInjector.unmap<Input>();
	m_coreInjector.unmap<OSWindow>();
	m_coreInjector.unmap<AppContext>();
	m_coreInjector.unmap<Options>();

	// Output console to log file
	//Console::SaveLog();

	return 0;
}

EngineCore::EngineCore(Injector& injector, Allocator& allocator)
	: m_coreInjector(injector)
	, m_coreAllocator(allocator)
	, m_rendererAllocator(*CUSTOM_NEW(ProxyAllocator, m_coreAllocator)(m_coreAllocator))
	, m_quit(false)
	, m_startTime(Timer::Seconds())
	, m_lastFrameTime(0.0)
	, m_lastUpdateTime(0.0)
{
	Log::Debug("EngineCore constructed at %p", this);
}

void EngineCore::initCommandProcessor()
{
	CommandProcessor::Initialize();
	CommandProcessor::AddCommand("quit", Command<>([this]() { stop(); }));
	CommandProcessor::AddCommand("exit", Command<>([this]() { stop(); }));
	//CommandProcessor::AddCommand("stats", Command<>([&]() { m_globalInjector.getInstance<StatTracker>()->ToggleStats(); }));
}

void EngineCore::initApplicationContext()
{
	m_coreInjector.mapSingleton<Options>();
	Options& options = m_coreInjector.getInstance<Options>();
	options.getOption<std::string>("version") = "alpha 0.0.0";

	const int renderWidth = options.getOption<int>("r_resolutionX");
	const int renderHeight = options.getOption<int>("r_resolutionY");
	const bool renderFullScreen = options.getOption<bool>("r_fullScreen");

	m_coreInjector.mapSingleton<AppContext>();
	AppContext& context = m_coreInjector.getInstance<AppContext>();
	context.InitApp(m_title + " - " + options.getOption<std::string>("version"),
		renderWidth,
		renderHeight,
		renderFullScreen);

	OSWindow& window = *context.GetWindow();
	m_coreInjector.mapInstance<OSWindow>(window);
	m_coreInjector.mapSingleton<Input, OSWindow>();
}

void EngineCore::initThreadPool()
{
	Options& options = m_coreInjector.getInstance<Options>();

	const unsigned int availableThreads = std::thread::hardware_concurrency() - 1;
	const bool useMultipleThreads = options.getOption<bool>("h_multiThreading");
	if (!useMultipleThreads || !availableThreads)
	{
		ThreadPool& threadPool = *(CUSTOM_NEW(ThreadPool, m_coreAllocator)(0));
		m_coreInjector.mapInstance<ThreadPool>(threadPool);
		options.getOption<bool>("h_multiThreading") = false;
		return;
	}

	ThreadPool& threadPool = *(CUSTOM_NEW(ThreadPool, m_coreAllocator)(availableThreads));
	m_coreInjector.mapInstance<ThreadPool>(threadPool);
}

void EngineCore::initRenderer()
{
	Options& options = m_coreInjector.getInstance<Options>();
	// TODO: Get rid of camera from here
	m_coreInjector.mapSingleton<Camera>();
	Camera& camera = m_coreInjector.getInstance<Camera>();
	ThreadPool& threadPool = m_coreInjector.getInstance<ThreadPool>();

	//const size_t RENDERER_HEAP_SIZE = 128 * MEGA;
	//unsigned char* renderHeap = (unsigned char*)m_coreAllocator.allocate(RENDERER_HEAP_SIZE);
	RendererGLProg* renderer = CUSTOM_NEW(RendererGLProg, m_rendererAllocator)(m_rendererAllocator, options, camera, threadPool);
	m_coreInjector.mapInstance<RendererGLProg>(*renderer);
	m_coreInjector.mapInterfaceToType<Renderer, RendererGLProg>();
	renderer->Initialize();
}

void EngineCore::initInjectorDependencies()
{
	m_coreInjector.mapSingleton<SceneManager>();
	m_coreInjector.mapSingleton<StatTracker, Options>();
}

void EngineCore::run()
{
	Camera& camera = m_coreInjector.getInstance<Camera>();
	Renderer& renderer = m_coreInjector.getInstance<Renderer>();
	SceneManager& sceneManager = m_coreInjector.getInstance<SceneManager>();
	//DebugDraw& debugDraw = m_coreInjector.getInstance<DebugDraw>();
	StatTracker& statTracker = m_coreInjector.getInstance<StatTracker>();
	ThreadPool& threadPool = m_coreInjector.getInstance<ThreadPool>();
	OSWindow& window = m_coreInjector.getInstance<OSWindow>();

	while (!m_quit)
	{
		const double timeNow = Timer::Seconds();
		const double deltaTime = timeNow - m_lastFrameTime;
		m_lastFrameTime = timeNow;

		pollEvents();

		CommandProcessor::Update(deltaTime);

		camera.Update(deltaTime);

		renderer.BeginDraw();

		if (!sceneManager.IsEmpty())
		{
			Scene* currentScene = sceneManager.GetActiveScene();
			currentScene->Update(deltaTime);
			currentScene->Draw();
		}
		renderer.flush();
		//debugDraw.flush();

		statTracker.trackFloatValue(deltaTime, "Frame Time");
		statTracker.trackFloatValue(1.f / deltaTime, "FPS");
		statTracker.trackIntValue((int32_t)threadPool.numJobs(), "Threadpool Jobs");
		statTracker.UpdateStats(&renderer);

		renderer.EndDraw();
		window.SwapBuffers();
	}
}

void EngineCore::stop()
{
	m_quit = true;
}

void EngineCore::pollEvents()
{
	/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
	/* more events on the event queue, our while loop will exit when */
	/* that occurs.                                                  */
	SDL_Event eventData;
	while (SDL_PollEvent(&eventData))
	{
		if (eventData.type == SDL_QUIT)
		{
			stop();
			break;
		}

		Input& input = m_coreInjector.getInstance<Input>();
		input.ProcessInput(eventData);
	}
}
