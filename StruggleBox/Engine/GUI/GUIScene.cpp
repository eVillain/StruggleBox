#include "GUIScene.h"
#include "TBGUI.h"

GUIScene::GUIScene(
	const std::string sceneID, 
	std::shared_ptr<TBGUI> gui) :
	Scene(sceneID),
	_gui(gui)
{

}

GUIScene::~GUIScene()
{
}

void GUIScene::Initialize()
{
	_root.SetRect(_gui->getRoot()->GetRect());
	_gui->getRoot()->AddChild(&_root, tb::WIDGET_Z_BOTTOM);
}

void GUIScene::ReInitialize()
{
}

void GUIScene::Pause()
{
	_gui->getRoot()->RemoveChild(&_root);

	tb::TBWidget* child = _root.GetFirstChild();
	while (child != nullptr)
	{
		_root.RemoveChild(child);
		child = _root.GetFirstChild();
	}
}

void GUIScene::Resume()
{
	_gui->getRoot()->AddChild(&_root, tb::WIDGET_Z_BOTTOM);
}
