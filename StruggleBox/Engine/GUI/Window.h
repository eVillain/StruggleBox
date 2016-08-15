#ifndef WINDOW_H
#define WINDOW_H

#include "tb_window.h"

class Window : public tb::TBWindow
{
public:
	Window(tb::TBWidget *root);
	bool LoadResourceFile(const char *filename);
	void LoadResourceData(const char *data);
	void LoadResource(tb::TBNode &node);

	virtual bool OnEvent(const tb::TBWidgetEvent &ev);
};

#endif // !WINDOW_BASE_H