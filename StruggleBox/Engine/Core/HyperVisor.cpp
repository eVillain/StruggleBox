#include "HyperVisor.h"
#include "Injector.h"
#include "AppContext.h"
#include "Options.h"
#include "Input.h"

#include "Camera.h"
#include "LightSystem3D.h"
#include "SceneManager.h"
#include "Particles.h"
#include "Physics.h"
#include "EntityManager.h"
#include "ShaderManager.h"
#include "RendererGLProg.h"
#include "TBGUI.h"
#include "Text.h"
#include "StatTracker.h"
#include "DebugDraw.h"

#include "FileUtil.h"
#include "Timer.h"
#include "ThreadPool.h"
#include "Log.h"
#include "LogOutputSTD.h"

#include "CommandProcessor.h"
#include "Console.h"
#include "ConsoleDisplay.h"

#include "Scene.h"
#include "Input.h"
#include "TextureManager.h"
#include "Color.h"

#include <iostream>
#include <math.h>
#include <memory>

HyperVisor::HyperVisor() :
	_injector(std::make_shared<Injector>()),
	_quit(false),
	_initialized(false),
	_context(nullptr),
	startTime(0.0),
	lastFrameTime(0.0),
	lastUpdateTime(0.0),
	frames(0),
	_heap(nullptr)
{
    Timer::StartRunTime();
}

void HyperVisor::Initialize(const std::string title,
                            const int argc,
                            const char * arg[])
{
	// Logging
    Log::Debug("HyperVisor initializing engine...");

	// Command processor
	CommandProcessor::Initialize();
	CommandProcessor::AddCommand("quit", Command<>([=]() { Stop(); }));
	CommandProcessor::AddCommand("exit", Command<>([=]() { Stop(); }));
	CommandProcessor::AddCommand("stats", Command<>([&]() { _injector->getInstance<StatTracker>()->ToggleStats(); }));
	//CommandProcessor::AddCommand("console", Command<>([&]() { Console::ToggleVisibility(); }));

	// Add mappings to dependency injector - ordering is magic, do not touch
	_injector->mapInstance(_injector);
	_injector->mapInstance(std::shared_ptr<HyperVisor>(this));
	_injector->mapSingleton<AppContext>();
	_injector->mapSingleton<Options>();

	// Allocate a heap to work in
	//_heap = new unsigned char[HEAPSIZE];
    
	// Options
	_options = _injector->getInstance<Options>();
	_options->getOption<std::string>("version") = "alpha 0.0.0";

	// Thread pool
	const int hwThreads = std::thread::hardware_concurrency();
	if (hwThreads > 1) {
		_options->getOption<bool>("h_multiThreading") = true;
		_injector->mapInstance(std::make_shared<ThreadPool>(hwThreads - 1));
	}

	int renderWidth = _options->getOption<int>("r_resolutionX");
	int renderHeight = _options->getOption<int>("r_resolutionY");
	bool renderFullScreen = _options->getOption<bool>("r_fullScreen");

	// Application context
	_context = _injector->getInstance<AppContext>();
	_context->InitApp(title + " - " + _options->getOption<std::string>("version"),
		renderWidth,
		renderHeight,
		renderFullScreen);

	_injector->mapInstance<OSWindow>(std::shared_ptr<OSWindow>(_context->GetWindow()));
	_injector->mapSingleton<Input,
		OSWindow>();
	_injector->mapSingleton<Camera>();

	_injector->mapSingleton<SceneManager>();
	_injector->mapSingleton<Physics>();
	_injector->mapSingleton<EntityManager,
	Injector>();
	_injector->mapSingleton<ShaderManager>();
	_injector->mapSingleton<LightSystem3D,
		ShaderManager>();
	_injector->mapSingletonOf<Renderer, RendererGLProg,
		Options, Camera, LightSystem3D, ShaderManager>();

	_injector->mapSingleton<TBGUI>();

	_injector->mapSingleton<Text,
		Renderer>();
	_injector->mapSingleton<Particles,
		Renderer>();
	_injector->mapSingleton<StatTracker,
		Options, Text>();
	_injector->mapSingleton<DebugDraw,
		Renderer, ShaderManager>();
	//_injector->mapSingleton<ConsoleDisplay,
	//GUIDraw, Text, GUI, Options>();

	// Stats tracker
    //std::shared_ptr<StatTracker> _stats = _injector->getInstance<StatTracker>();
    
    // Try to load renderer, exit on fail
	std::shared_ptr<Renderer> renderer = _injector->getInstance<Renderer>();
	std::shared_ptr<Text> text = _injector->getInstance<Text>();
	std::shared_ptr<LightSystem3D> lightSys3D = _injector->getInstance<LightSystem3D>();

	text->Initialize();

	// Camera
	std::shared_ptr<Camera> _camera = _injector->getInstance<Camera>();

	// Console
	Console::Initialize();
	Console::AddVar(_camera->focalDepth, "focalDepth");
	Console::AddVar(_camera->focalLength, "focalLength");
	Console::AddVar(_camera->fStop, "fStop");
	Console::AddVar(_camera->exposure, "exposure");

    // Input
    _input = _injector->getInstance<Input>();
	_input->Initialize();
	_input->RegisterEventObserver(this);
    
	std::shared_ptr<OSWindow> window = _injector->getInstance<OSWindow>();

	std::shared_ptr<TBGUI> tbgui = _injector->getInstance<TBGUI>();
	tbgui->initialize();
	tbgui->onResized(window->GetWidth(), window->GetHeight());

    // Engine is finally ready so we'll pass our command-line arguments to
    // the CommandProcessor where they will be parsed and processed.
    CommandProcessor::Buffer(argc, arg);
    
    _initialized = true;
    
    // Set "running" flag
    _quit = false;
}


void HyperVisor::Run(std::shared_ptr<Scene> scene)
{
    // Main loop
    startTime = Timer::Seconds();
    lastFrameTime = startTime;
    frames = 0;

	std::shared_ptr<Camera> camera = _injector->getInstance<Camera>();
	std::shared_ptr<Renderer> renderer = _injector->getInstance<Renderer>();
	std::shared_ptr<DebugDraw> debugDraw = _injector->getInstance<DebugDraw>();
	std::shared_ptr<SceneManager> sceneManager = _injector->getInstance<SceneManager>();
	std::shared_ptr<Text> text = _injector->getInstance<Text>();
	std::shared_ptr<LightSystem3D> lightSys3D = _injector->getInstance<LightSystem3D>();
	std::shared_ptr<StatTracker> stats = _injector->getInstance<StatTracker>();
	std::shared_ptr<ThreadPool> threadPool = _injector->getInstance<ThreadPool>();
//	std::shared_ptr<ConsoleDisplay> consoleDisplay = _injector->getInstance<ConsoleDisplay>();
	std::shared_ptr<Particles> particles = _injector->getInstance<Particles>();
	std::shared_ptr<TBGUI> tbgui = _injector->getInstance<TBGUI>();

	sceneManager->AddActiveScene(scene);

    while( !_quit )
    {
        // Get frame time
        double timeNow = Timer::Seconds();
        double deltaTime = timeNow - lastFrameTime;
        lastFrameTime = timeNow;

		//Event handler
		SDL_Event eventData;

		/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
		/* more events on the event queue, our while loop will exit when */
		/* that occurs.                                                  */
		while (SDL_PollEvent(&eventData))
		{
			if (eventData.type == SDL_QUIT)
			{
				Stop(); 
				break;
			}

			if (!_input->ProcessInput(eventData))
			{
				tbgui->HandleSDLEvent(eventData);
			}
		}

                
        // Update camera before beginning rendering
        camera->Update( deltaTime );
        
		tbgui->update();
		
        // Setup rendering for new frame
        renderer->BeginDraw();

        if (!sceneManager->IsEmpty())
        {
            std::shared_ptr<Scene> currentScene = sceneManager->GetActiveScene();
            currentScene->Update(deltaTime);
            
            currentScene->Draw();
        }
		particles->Draw();
		renderer->flush();

		// Debug draw
		debugDraw->flush();

        // Stats
        if (stats->IsVisible() ) {
            stats->SetRFrameDelta(deltaTime);
            stats->SetRFPS(1.0/deltaTime);
            stats->SetTNumJobs((int)threadPool->NumJobs());
            stats->UpdateStats(renderer.get());
        }
        
        // Render console on top of everything
		//consoleDisplay->Draw();
        
        // Render any text in UI
        text->Draw();

		// Render UI widgets
		tbgui->draw();
        
        renderer->EndDraw();
        
        _context->GetWindow()->SwapBuffers();
        
        // Process commands
        CommandProcessor::Update(deltaTime);

        // Increase frame count
        frames ++;
    }
}

void HyperVisor::Stop()
{
    _quit = true;
}


void HyperVisor::Restart()
{
    // TODO: This should reset every system and restart the engine
	Log::Debug("[HyperVisor] Restart called, not implemented yet!");
}

void HyperVisor::Terminate()
{
    Log::Debug("[HyperVisor] shutting down...");
    
	_injector->getInstance<TBGUI>()->terminate();
	_injector->getInstance<SceneManager>()->RemoveActiveScene();

    // Measure running time
    double timeNow = Timer::Seconds();
    lastFrameTime = Timer::Seconds() - startTime;
    double totalTime = timeNow - startTime;
    
    // Output console to log file
    Console::SaveLog();
    
    // Display profiling information
    float avgFPS = (float)(frames / totalTime);
    std::string funLevel;
    if ( avgFPS > 60 ) { funLevel = "Furiously crunched"; }
    else if ( avgFPS > 58 ) { funLevel = "Easily delivered"; }
    else if ( avgFPS > 30 ) { funLevel = "Struggled through"; }
    else if ( avgFPS > 15 ) { funLevel = "Barely managed"; }
    else { funLevel = "Somehow scraped up"; }
	Log::Debug( "[HyperVisor] %s %d frames in %.2f seconds = %.1f FPS (average)", funLevel.c_str(), frames, totalTime,
           avgFPS );
}

// Low level events - we will try to make sure that we can always access the
// console or at least back to the main menu so we can quit :)
bool HyperVisor::OnEvent(const std::string& event,
                         const float& amount)
{
    if ( amount != -1 ) return false; // We only care for key/button release events here
    
    if (event == INPUT_CONSOLE)
    {
		if (!_injector->hasMapping<ConsoleDisplay>())
			return false;
		std::shared_ptr<ConsoleDisplay> consoleDisplay = _injector->getInstance<ConsoleDisplay>();
		if (!consoleDisplay->isVisible())
		{
			consoleDisplay->ToggleVisibility();
		}
        return true;
    }
    else if (event == INPUT_BACK)
    {
		//std::shared_ptr<ConsoleDisplay> consoleDisplay = _injector->getInstance<ConsoleDisplay>();

  //      if ( consoleDisplay->isVisible() )
  //      {
		//	consoleDisplay->ToggleVisibility();
		//}
  //      else
  //      {
			_options->getOption<bool>("r_grabCursor") = false; // Bring the cursor back in case it was hidden
			SDL_ShowCursor(true);
			// Show main menu
			std::shared_ptr<SceneManager> sceneManager = _injector->getInstance<SceneManager>();
			std::string prevState = sceneManager->GetPreviousSceneName();
			if (!prevState.empty())
			{
				sceneManager->SetActiveScene(prevState);
			}
			else
			{
				Stop();
			}
		//}
        return true;
    }
    return false;
}
