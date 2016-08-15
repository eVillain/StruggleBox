#ifndef OPTIONS_WINDOW_H
#define OPTIONS_WINDOW_H

#include "Window.h"
#include <memory>

class Options;

class OptionsWindow : public Window
{
public:
	OptionsWindow(
		tb::TBWidget *root,
		std::shared_ptr<Options> options);
	virtual bool OnEvent(const tb::TBWidgetEvent &ev);

private:
	std::shared_ptr<Options> _options;
};

#endif // !OPTIONS_WINDOW_H