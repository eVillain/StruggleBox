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
#include "RenderCore.h"
#include "SceneManager.h"
#include "Scene.h"
#include "StatTracker.h"

constexpr bool USE_STD_OUTPUT = true;

EngineCore& EngineCore::create(const int argc, const char* arg[], unsigned char* heap)
{
	Timer::StartRunTime();
	Console::Initialize();

	FreeListAllocator* coreAllocator = new (heap)FreeListAllocator(HEAP_SIZE - sizeof(FreeListAllocator), heap + sizeof(FreeListAllocator));
	if (USE_STD_OUTPUT)
	{
		LogOutputSTD* outSTD = CUSTOM_NEW(LogOutputSTD, (*coreAllocator))();
		Log::AttachOutput(outSTD);
	}
	else
	{
		LogOutputConsole* outConsole = CUSTOM_NEW(LogOutputConsole, (*coreAllocator))();
		Log::AttachOutput(outConsole);
#ifdef WIN32
		FreeConsole(); // Remove command prompt console - its useless if we never output to it :)
#endif
	}

	//Log::Info("EngineCore Created core allocator at %p", allocator);

	Injector& injector = *(CUSTOM_NEW(Injector, (*coreAllocator))(*coreAllocator));
	injector.mapInstance<Injector>(injector);
	injector.mapInstance<Allocator>(*coreAllocator);

	injector.mapSingleton<EngineCore, Injector, Allocator>();
	EngineCore& core = injector.getInstance<EngineCore>();

	return core;
}

void EngineCore::destroy(EngineCore& engineCore)
{
	Injector& injector = engineCore.m_coreInjector;
	Allocator& allocator = engineCore.m_coreAllocator;
	Allocator& rendererAllocator = engineCore.m_rendererAllocator;

	const auto& logOutputs = Log::GetOutputs();
	if (USE_STD_OUTPUT)
	{
		for (LogOutput* output : logOutputs)
		{
			if (dynamic_cast<LogOutputSTD*>(output))
			{
				Log::DetachOutput(output);
				CUSTOM_DELETE(output, allocator);
				break;
			}
		}
	}
	else
	{
		for (LogOutput* output : logOutputs)
		{
			if (dynamic_cast<LogOutputConsole*>(output))
			{
				Log::DetachOutput(output);
				CUSTOM_DELETE(output, allocator);
				break;
			}
		}
	}


	injector.unmap<Injector>();
	injector.unmap<Allocator>();
	injector.unmap<EngineCore>();

	CUSTOM_DELETE(&rendererAllocator, allocator);
	CUSTOM_DELETE(&injector, allocator);
	//delete (&allocator);
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
	//Log::Debug("EngineCore shutting down...");
	CommandProcessor::Terminate();

	SceneManager& sceneManager = m_coreInjector.getInstance<SceneManager>();
	sceneManager.terminate();

	Input& input = m_coreInjector.getInstance<Input>();
	input.Terminate();

	ThreadPool& threadPool = m_coreInjector.getInstance<ThreadPool>();
	CUSTOM_DELETE(&threadPool, m_coreAllocator);

	m_coreInjector.unmap<SceneManager>();
	m_coreInjector.unmap<StatTracker>();
	m_coreInjector.unmap<ThreadPool>();
	m_coreInjector.unmap<Input>();
	m_coreInjector.unmap<OSWindow>();
	m_coreInjector.unmap<AppContext>();
	m_coreInjector.unmap<Options>();

	RenderCore& renderCore = m_coreInjector.getInstance<RenderCore>();
	renderCore.terminate();
	CUSTOM_DELETE(&renderCore, m_rendererAllocator);
	m_coreInjector.unmap<RenderCore>();

	allocator::deleteProxyAllocator(m_rendererAllocator, m_coreAllocator);
	// Output console to log file
	//Console::SaveLog();

	return 0;
}

EngineCore::EngineCore(Injector& injector, Allocator& coreAllocator)
	: m_coreInjector(injector)
	, m_coreAllocator(coreAllocator)
	, m_rendererAllocator(*allocator::newProxyAllocator(coreAllocator))
	, m_quit(false)
	, m_startTime(Timer::Seconds())
	, m_lastFrameTime(0.0)
	, m_lastUpdateTime(0.0)
{
	//Log::Debug("EngineCore constructed at %p", this);
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
		//ThreadPool& threadPool = *(CUSTOM_NEW(ThreadPool, m_coreAllocator)(0));
		//m_coreInjector.mapInstance<ThreadPool>(threadPool);
		options.getOption<bool>("h_multiThreading") = false;
		return;
	}

	ThreadPool& threadPool = *(CUSTOM_NEW(ThreadPool, m_coreAllocator)(availableThreads));
	m_coreInjector.mapInstance<ThreadPool>(threadPool);
}

void EngineCore::initRenderer()
{
	ThreadPool& threadPool = m_coreInjector.getInstance<ThreadPool>();
	RenderCore& renderCore = *(CUSTOM_NEW(RenderCore, m_rendererAllocator)(m_rendererAllocator, threadPool));
	m_coreInjector.mapInstance<RenderCore>(renderCore);
}

void EngineCore::initInjectorDependencies()
{
	m_coreInjector.mapSingleton<SceneManager>();
	m_coreInjector.mapSingleton<StatTracker, Options>();
}

void EngineCore::run()
{
	SceneManager& sceneManager = m_coreInjector.getInstance<SceneManager>();
	StatTracker& statTracker = m_coreInjector.getInstance<StatTracker>();
	statTracker.setValueBuffered("Frame Time", 1000);
	ThreadPool& threadPool = m_coreInjector.getInstance<ThreadPool>();
	OSWindow& window = m_coreInjector.getInstance<OSWindow>();
	RenderCore& renderCore = m_coreInjector.getInstance<RenderCore>();
	m_lastFrameTime = Timer::Seconds();

	while (!m_quit)
	{
		const double timeNow = Timer::Seconds();
		const double deltaTime = timeNow - m_lastFrameTime;
		m_lastFrameTime = timeNow;

		pollEvents();

		CommandProcessor::Update(deltaTime);

		renderCore.beginFrame();

		if (!sceneManager.IsEmpty())
		{
			Scene* currentScene = sceneManager.GetActiveScene();
			currentScene->Update(deltaTime);
			currentScene->Draw();
		}

		statTracker.trackFloatValue((float)deltaTime * 1000.f, "Frame Time");
		statTracker.trackIntValue((int32_t)(1.f / (float)deltaTime), "FPS");
		statTracker.trackIntValue((int32_t)threadPool.numJobs(), "Threadpool Jobs");

		renderCore.endFrame();
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
		if (eventData.type == SDL_EVENT_QUIT)
		{
			stop();
			break;
		}

		Input& input = m_coreInjector.getInstance<Input>();
		input.ProcessInput(eventData);
	}
}
