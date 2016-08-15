#include "LayoutLambda.h"
#include "tb_node_tree.h"
#include "tb_core.h"
#include "tb_widgets_reader.h"

LayoutLambda::LayoutLambda()
{
}

LayoutLambda::~LayoutLambda()
{
}

bool LayoutLambda::LoadResourceFile(const char * filename)
{
	// We could do g_widgets_reader->LoadFile(this, filename) but we want
	// some extra data we store under "WindowInfo", so read into node tree.
	tb::TBNode node;
	if (!node.ReadFile(filename))
		return false;
	tb::g_widgets_reader->LoadNodeTree(this, &node);
	return true;
}

bool LayoutLambda::OnEvent(const tb::TBWidgetEvent & ev)
{
	if (ev.type == tb::EVENT_TYPE_CLICK &&
		_bindings.find(ev.target->GetID()) != _bindings.end())
	{
		_bindings[ev.target->GetID()](ev);
		return true;
	}
	return tb::TBLayout::OnEvent(ev);
}

void LayoutLambda::AddListener(
	std::string widgetID,
	WidgetEventFunction func)
{
	tb::TBID id = TBIDC(widgetID.c_str());
	_bindings[id] = func;
}
