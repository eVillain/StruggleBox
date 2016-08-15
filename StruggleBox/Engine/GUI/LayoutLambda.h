#ifndef LAYOUT_LAMBDA_H
#define LAYOUT_LAMBDA_H

#include "tb_layout.h"
#include <map>
#include <string>
#include <functional>

class LayoutLambda : public tb::TBLayout
{
public:
	LayoutLambda();
	~LayoutLambda();

	bool LoadResourceFile(const char *filename);

	virtual bool OnEvent(const tb::TBWidgetEvent &ev);
	typedef std::function<void(const tb::TBWidgetEvent &ev)> WidgetEventFunction;

	void AddListener(std::string widgetID, WidgetEventFunction func);
private:
	std::map<tb::TBID, WidgetEventFunction> _bindings;
};

#endif // !LAYOUT_LAMBDA_H