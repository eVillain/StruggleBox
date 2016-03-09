#include <iostream>
#include "HyperVisor.h"
#include "Game/MainMenu.h"
#include "SceneManager.h"

int main(int argc, const char * argv[])
{
#ifdef WIN32
	// Remove command prompt console
	// FreeConsole();
#endif

	HyperVisor hv;
	hv.Initialize("StruggleBox", argc, argv);

    SceneManager* sceneMan = hv.GetLocator().Get<SceneManager>();
	MainMenu * mainMenu = new MainMenu(hv.GetLocator());
    sceneMan->AddActiveScene(mainMenu);

    hv.Run();
    hv.Terminate();
    
    return 0;
}

