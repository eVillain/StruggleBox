#ifndef TB_GUI_H
#define TB_GUI_H

#include "tb_widgets.h"
#include "SDL2/SDL.h"
#include <memory>

/// Wrapper for TurboBadger GUI
/// Contains the root widget
class TBGUI
{
public:
	TBGUI();
	~TBGUI();

	bool initialize();
	void terminate();
	void update();
	void draw();

	bool HandleSDLEvent(SDL_Event & event);

	void onResized(int width, int height);

	tb::TBWidget *getRoot() { return &_root; }

private:
	tb::TBWidget _root;
	std::unique_ptr<tb::TBRenderer> _renderer;

	bool InvokeKey(unsigned int key, tb::SPECIAL_KEY special_key,
		tb::MODIFIER_KEYS modifierkeys, bool down);
};

#endif // !TB_GUI_H