#include "HyperVisor.h"
#include "Game/MainMenu.h"
#include "Injector.h"
#include <iostream>
#include <memory>

int main(int argc, const char * argv[])
{
#ifdef WIN32
	// Remove command prompt console
	// FreeConsole();
#endif

	HyperVisor hv;
	hv.Initialize("StruggleBox", argc, argv);

    //SceneManager* sceneMan = hv.GetLocator().Get<SceneManager>();
	//MainMenu * mainMenu = new MainMenu(hv.GetLocator());
    //sceneMan->AddActiveScene(mainMenu);

	auto mainMenu = hv.getInjector()->instantiateUnmapped<MainMenu,
	Injector, Renderer, Particles, Text, SceneManager, TBGUI, Options>();
    hv.Run(mainMenu);
    hv.Terminate();
    
    return 0;
}

