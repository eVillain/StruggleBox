#ifndef HYPERVISOR_H
#define HYPERVISOR_H

#include "CoreTypes.h"
#include "Options.h"
#include "InputListener.h"
#include "AppContext.h"
#include "Locator.h"
#include <memory>
#include <string>

class HyperVisor : public InputEventListener
{
public:
    HyperVisor();
    
    void Initialize(const std::string title,
                    const int argc,
                    const char * arg[]);
    
    void Terminate();
    
    void Run();
    void Stop();

    // TODO: Implement engine restart functionality
    void Restart();

    Locator& GetLocator() { return _locator; };
    
private:
    std::unique_ptr<AppContext> _context;
    bool _quit;
    bool _initialized;

    Locator _locator;
    
    double startTime;                       // Timestamp for the engine startup
    double lastFrameTime, lastUpdateTime;   // Temporary timestamps
    int frames;                             // Number of rendered frames (TODO: MOVE TO RENDERER)
    
    std::unique_ptr<Options> _options;      // Engine and game options interface, just an autosaving dictionary

    TextManager* m_textMan;                 // Text renderer, should become part of renderer
    
    Input* m_inputMan;                      // User input manager
        
    Camera* m_camera;                       // Camera system
    
    void initThreadPool();
    void initCommandProcessor();
    void initAppContext(const std::string& title);
    void initOptions();
    void initRenderer();
    void initConsole();
    
    
    void LoadEditor();
    void LoadGame();
    
    bool OnEvent(const std::string& event,
                 const float& amount);
};

#endif /* HYPERVISOR_H */
