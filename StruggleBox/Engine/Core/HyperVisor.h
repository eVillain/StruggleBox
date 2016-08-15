#ifndef HYPERVISOR_H
#define HYPERVISOR_H

class Injector;
class AppContext;
class Options;
class Input;

class Scene;

#include "InputListener.h"
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
    
    void Run(std::shared_ptr<Scene> scene);
    void Stop();

    // TODO: Implement engine restart functionality
    void Restart();
    
	// Should only be used to initialize a scene
	std::shared_ptr<Injector> getInjector() {
		return _injector;
	};
private:
	unsigned char* _heap;					// Heap memory for allocators
	std::shared_ptr<Injector> _injector;	// Dependency injector
    std::shared_ptr<AppContext> _context;	// Main application context
	std::shared_ptr<Options> _options;      // Engine and game options interface, just an autosaving dictionary
	std::shared_ptr<Input> _input;          // User input manager

    bool _quit;
    bool _initialized;    
    double startTime;                       // Timestamp for the engine startup
    double lastFrameTime, lastUpdateTime;   // Temporary timestamps
    int frames;                             // Number of rendered frames (TODO: MOVE TO RENDERER)

    bool OnEvent(const std::string& event,
                 const float& amount);
};

#endif /* HYPERVISOR_H */
