#include "HyperVisor.h"
#include "FileUtil.h"
#include "Timer.h"
#include "ThreadPool.h"

#include "SysCore.h"
#include "CommandProcessor.h"
#include "Console.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Input.h"
#include "UIManager.h"
#include "GUI.h"
#include "Options.h"
#include "RendererGLProg.h"
#include "LightSystem2D.h"
#include "LightSystem3D.h"
#include "Camera.h"
#include "StatTracker.h"
#include "TextManager.h"
#include "Text.h"
#include "TextureManager.h"
#include "Particles.h"
#include <iostream>
#include <math.h>

HyperVisor::HyperVisor() :
_quit(false),
_initialized(false),
_context(nullptr),
startTime(0.0),
lastFrameTime(0.0),
lastUpdateTime(0.0),
frames(0)
{
    Timer::StartRunTime();

    _options = nullptr;
    m_inputMan = NULL;
    m_camera = NULL;
}

void HyperVisor::Initialize(const std::string title,
                            const int argc,
                            const char * arg[])
{
    Log::Debug("HyperVisor initializing engine...");
    
    _locator.MapInstance<HyperVisor>(this);
    
    FileUtil::UpdateRelativePath();
    SysCore::SetRelativePath(); // TODO: Get rid of SysCore entirely

    initCommandProcessor();
    initOptions();
	if ( _options->getOption<bool>("h_multiThreading") ) {
        initThreadPool();
    }
    initAppContext(title);
    
    StatTracker* m_statTracker = new StatTracker(_locator);
    _locator.MapInstance<StatTracker>(m_statTracker);
    
    // Try to load renderer, exit on fail
    initRenderer();
    initConsole();
    
    // Init managers and hook pointers
    SceneManager* m_sceneMan = new SceneManager(_locator);
    _locator.MapInstance<SceneManager>(m_sceneMan);
    
    m_inputMan = new Input(_locator);
    _locator.MapInstance<Input>(m_inputMan);
    m_inputMan->Initialize();
    m_inputMan->RegisterEventObserver(this);

    UIManager* m_uiMan = new UIManager();
    m_uiMan->Initialize(_locator);
    _locator.MapInstance<UIManager>(m_uiMan);
    
    GUI* gui = new GUI();
    gui->Initialize(_locator);
    _locator.MapInstance<GUI>(gui);
    
    Particles* particles = new Particles(_locator);
    _locator.MapInstance<Particles>(particles);
    
    // Engine is finally ready so we'll pass our command-line arguments to
    // the CommandProcessor where they will be parsed and processed.
    CommandProcessor::Buffer(argc, arg);
    
    _initialized = true;
    
    // Set "running" flag
    _quit = false;
}


void HyperVisor::Run()
{
    // Main loop
    startTime = SysCore::GetSeconds();
    lastFrameTime = startTime;
    frames = 0;
    Renderer* renderer = _locator.Get<Renderer>();
    SceneManager* sceneManager = _locator.Get<SceneManager>();
    ThreadPool* threadPool = _locator.Get<ThreadPool>();
    UIManager* uiMan = _locator.Get<UIManager>();
    GUI* gui = _locator.Get<GUI>();
    StatTracker* statTracker = _locator.Get<StatTracker>();
    TextManager* textManager = _locator.Get<TextManager>();
    Text* text = _locator.Get<Text>();

    while( !_quit )
    {
        // Get frame time
        double timeNow = SysCore::GetSeconds();
        double deltaTime = timeNow - lastFrameTime;
        lastFrameTime = timeNow;

        // Poll input
        m_inputMan->ProcessInput();
                
        // Update camera before beginning rendering
        m_camera->Update( deltaTime );
        
        gui->Update(deltaTime);

        // Setup rendering for new frame
        renderer->BeginDraw();

        if (!sceneManager->IsEmpty())
        {
            Scene& currentScene = sceneManager->GetActiveScene();
            currentScene.Update(deltaTime);
            
            currentScene.Draw();
            renderer->PostProcess();
        }

        // Render UI widgets
        uiMan->RenderWidgets();
        gui->Draw(renderer);
        
        if (statTracker->IsVisible() ) {
            statTracker->SetRFrameDelta(deltaTime);
            statTracker->SetRFPS(1.0/deltaTime);
            statTracker->SetTNumJobs((int)threadPool->NumJobs());
            statTracker->UpdateStats(renderer);
        }
        
        // Render console on top of everything
        Console::Draw(deltaTime);
        
        // Render any text in UI
        textManager->RenderLabels();
        text->Draw();
        
        renderer->EndDraw();
        
        _context->GetWindow()->SwapBuffers();
        
        // Process commands
        CommandProcessor::Update(deltaTime);
        
        // Check if window was closed
        _quit = _quit || renderer->shouldClose;

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
    printf("[HyperVisor] Restart called, not implemented yet!\n");
}

void HyperVisor::Terminate()
{
    printf("[HyperVisor] shutting down...\n");
    
    // Measure running time
    double timeNow = SysCore::GetSeconds();
    lastFrameTime = SysCore::GetSeconds() - startTime;
    double totalTime = timeNow - startTime;
    
    // Wait for threads to die?
    if (_locator.Satisfies<ThreadPool>()) {
        delete _locator.Get<ThreadPool>();
        _locator.UnMap<ThreadPool>();
    }
    
    // Clean up any scenes left in stack
    if (_locator.Satisfies<SceneManager>()) {
        delete _locator.Get<SceneManager>();
        _locator.UnMap<SceneManager>();
    }
    
    if (Console::isVisible()) Console::ToggleVisibility();
    
    // Clean up any UI elements left behind
    if (_locator.Satisfies<UIManager>()) {
        delete _locator.Get<UIManager>();
        _locator.UnMap<UIManager>();
    }
    if (_locator.Satisfies<GUI>()) {
        delete _locator.Get<GUI>();
        _locator.UnMap<GUI>();
    }
    
    if (_locator.Satisfies<TextManager>()) {
        delete _locator.Get<TextManager>();
        _locator.UnMap<TextManager>();
    }
    if (_locator.Satisfies<Text>()) {
        delete _locator.Get<Text>();
        _locator.UnMap<Text>();
    }

    delete m_camera;
    m_camera = NULL;

    delete m_inputMan;
    m_inputMan = NULL;
    
    if (_locator.Satisfies<Renderer>()) {
        Renderer* renderer = _locator.Get<Renderer>();
        delete renderer;
        _locator.UnMap<Renderer>();
    }
    
    if (_locator.Satisfies<StatTracker>()) {
        delete _locator.Get<StatTracker>();
        _locator.UnMap<StatTracker>();
    }
    
    if (_locator.Satisfies<Particles>()) {
        _locator.UnMap<Particles>();
    }
    
    // Output console to log file
    Console::SaveLog();
    
    // Display profiling information
    float avgFPS = frames / totalTime;
    std::string funLevel;
    if ( avgFPS > 60 ) { funLevel = "Furiously crunched"; }
    else if ( avgFPS > 58 ) { funLevel = "Easily delivered"; }
    else if ( avgFPS > 30 ) { funLevel = "Struggled through"; }
    else if ( avgFPS > 15 ) { funLevel = "Barely managed"; }
    else { funLevel = "Somehow scraped up"; }
    printf( "[HyperVisor] %s %d frames in %.2f seconds = %.1f FPS (average)\n", funLevel.c_str(), frames, totalTime,
           avgFPS );
}



void HyperVisor::initThreadPool()
{
    const int hwThreads = std::thread::hardware_concurrency();
    if (hwThreads < 2) {
        _options->getOption<bool>("h_multiThreading") = false;
    }
    const int numThreads = (hwThreads)-1;
    ThreadPool* threadPool = new ThreadPool(numThreads); // Start new thread pool
    _locator.MapInstance<ThreadPool>(threadPool);
}

void HyperVisor::initCommandProcessor()
{
    CommandProcessor::Initialize();
    
    CommandProcessor::AddCommand("quit", Command<>([=](){ Stop(); }));
    CommandProcessor::AddCommand("exit", Command<>([=](){ Stop(); }));
    
    CommandProcessor::AddCommand("stats", Command<>([&]() {
        _locator.Get<StatTracker>()->ToggleStats();
    }));
    CommandProcessor::AddCommand("console", Command<>([&]() {
        Console::ToggleVisibility();
    }));
}

void HyperVisor::initOptions()
{
    _options = std::make_unique<Options>();
    _locator.MapInstance<Options>(_options.get());
    _options->getOption<std::string>("version") = "alpha 0.0.0";
}

void HyperVisor::initAppContext(const std::string& title)
{
    int renderWidth = _options->getOption<int>("r_resolutionX");
    int renderHeight = _options->getOption<int>("r_resolutionY");
    bool renderFullScreen = _options->getOption<bool>("r_fullScreen");
    
    _context = std::make_unique<AppContext>();
    _context->InitApp(title + " - " + _options->getOption<std::string>("version"),
                      renderWidth,
                      renderHeight,
                      renderFullScreen);
    
    _locator.MapInstance<AppContext>(_context.get());
}

void HyperVisor::initRenderer()
{
    m_camera = new Camera();
    _locator.MapInstance<Camera>(m_camera);
    
    LightSystem3D* m_lightSys3D = new LightSystem3D();
    _locator.MapInstance<LightSystem3D>(m_lightSys3D);
    
    RendererGLProg* renderer = new RendererGLProg();
    _locator.MapInstanceToInterface<RendererGLProg, Renderer>(renderer);
    
    TextManager* m_textMan = new TextManager();
    _locator.MapInstance<TextManager>(m_textMan);
    
    Text* text = new Text();
    _locator.MapInstance<Text>(text);
    
    renderer->Initialize(_locator);
    m_textMan->Initialize(_locator);
    text->Initialize(_locator);
    m_lightSys3D->HookRenderer(renderer);   // TODO: Make this an init function
    
    if (!renderer->initialized)
    {
        printf("[HyperVisor] Critical fail: Renderer not initialized\n");
        exit(EXIT_FAILURE);
    }
}

void HyperVisor::initConsole()
{
    Console::Initialize(_locator);
    
    // Add some console variables
    Console::AddVar(m_camera->focalDepth, "focalDepth");
    Console::AddVar(m_camera->focalLength, "focalLength");
    Console::AddVar(m_camera->fStop, "fStop");
    Console::AddVar(m_camera->exposure, "exposure");
}

// Low level events - we will try to make sure that we can always access the
// console or at least back to the main menu so we can quit :)
bool HyperVisor::OnEvent(const std::string& event,
                         const float& amount)
{
    if ( amount != -1 ) return false; // We only care for key/button release events here
    
    if (event == INPUT_CONSOLE)
    {
        Console::ToggleVisibility();
        return true;
    }
    else if (event == INPUT_BACK)
    {
        if ( Console::isVisible() )
        { Console::ToggleVisibility(); }
        else
        { Stop(); } // For now we stop TODO: change this to pop the previous scene
        return true;
    }
    return false;
}
