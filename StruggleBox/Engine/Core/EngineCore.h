#pragma once

#include <string>

class Allocator;
class ProxyAllocator;
class Injector;
class Scene;

class EngineCore
{
public:
	static EngineCore& create(const int argc,
							  const char* arg[],
							  unsigned char* heap);
	~EngineCore();

	void initialize();
	void runScene(Scene& scene);
	int terminate();

	void setTitle(const std::string& title) { m_title = title; }

	Injector& getGlobalInjector() const { return m_coreInjector; }
	Allocator& getGlobalAllocator() const { return m_coreAllocator; }

private:
	Injector& m_coreInjector;
	Allocator& m_coreAllocator;
	ProxyAllocator& m_rendererAllocator;

	bool m_quit;
	double m_startTime;                       // Timestamp for the engine startup
	double m_lastFrameTime, m_lastUpdateTime; // Temporary frame timestamps
	std::string m_title;

	friend class Injector;
	EngineCore(Injector& injector, Allocator& allocator);

	void initCommandProcessor();
	void initApplicationContext();
	void initThreadPool();
	void initRenderer();
	void initInjectorDependencies();

	void run();
	void stop();

	void pollEvents();
};

