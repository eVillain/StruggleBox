#ifndef GUI_SCENE_H
#define GUI_SCENE_H

#include "Scene.h"
#include "LayoutLambda.h"
#include <memory>

class TBGUI;

class GUIScene : public Scene
{
public:
	GUIScene(
		const std::string sceneID,
		std::shared_ptr<TBGUI> gui);
	~GUIScene();

	virtual void Initialize();
	virtual void ReInitialize();
	virtual void Pause();
	virtual void Resume();

protected:
	std::shared_ptr<TBGUI> _gui;

	LayoutLambda _root;
};

#endif // !GUI_SCENE_H