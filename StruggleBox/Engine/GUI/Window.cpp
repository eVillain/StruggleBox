#include "Window.h"
#include "tb_node_tree.h"
#include "tb_core.h"
#include "tb_widgets_reader.h"

Window::Window(tb::TBWidget * root)
{
	root->AddChild(this);
}

bool Window::LoadResourceFile(const char * filename)
{
	// We could do g_widgets_reader->LoadFile(this, filename) but we want
	// some extra data we store under "WindowInfo", so read into node tree.
	tb::TBNode node;
	if (!node.ReadFile(filename))
		return false;
	LoadResource(node);
	return true;
}

void Window::LoadResourceData(const char * data)
{
	// We could do g_widgets_reader->LoadData(this, filename) but we want
	// some extra data we store under "WindowInfo", so read into node tree.
	tb::TBNode node;
	node.ReadData(data);
	LoadResource(node);
}

void Window::LoadResource(tb::TBNode & node)
{
	tb::g_widgets_reader->LoadNodeTree(this, &node);

	// Get title from the WindowInfo section (or use "" if not specified)
	SetText(node.GetValueString("WindowInfo>title", ""));

	const tb::TBRect parent_rect(0, 0, GetParent()->GetRect().w, GetParent()->GetRect().h);
	const tb::TBDimensionConverter *dc = tb::g_tb_skin->GetDimensionConverter();
	tb::TBRect window_rect = GetResizeToFitContentRect();

	// Use specified size or adapt to the preferred content size.
	tb::TBNode *tmp = node.GetNode("WindowInfo>size");
	if (tmp && tmp->GetValue().GetArrayLength() == 2)
	{
		window_rect.w = dc->GetPxFromString(tmp->GetValue().GetArray()->GetValue(0)->GetString(), window_rect.w);
		window_rect.h = dc->GetPxFromString(tmp->GetValue().GetArray()->GetValue(1)->GetString(), window_rect.h);
	}

	// Use the specified position or center in parent.
	tmp = node.GetNode("WindowInfo>position");
	if (tmp && tmp->GetValue().GetArrayLength() == 2)
	{
		window_rect.x = dc->GetPxFromString(tmp->GetValue().GetArray()->GetValue(0)->GetString(), window_rect.x);
		window_rect.y = dc->GetPxFromString(tmp->GetValue().GetArray()->GetValue(1)->GetString(), window_rect.y);
	}
	else
		window_rect = window_rect.CenterIn(parent_rect);

	// Make sure the window is inside the parent, and not larger.
	window_rect = window_rect.MoveIn(parent_rect).Clip(parent_rect);

	SetRect(window_rect);

	// Ensure we have focus - now that we've filled the window with possible focusable
	// widgets. EnsureFocus was automatically called when the window was activated (by
	// adding the window to the root), but then we had nothing to focus.
	// Alternatively, we could add the window after setting it up properly.
	EnsureFocus();
}

bool Window::OnEvent(const tb::TBWidgetEvent & ev)
{
	if (ev.type == tb::EVENT_TYPE_KEY_DOWN && ev.special_key == tb::TB_KEY_ESC)
	{
		// We could call Die() to fade away and die, but click the close button instead.
		// That way the window has a chance of intercepting the close and f.ex ask if it really should be closed.
		tb::TBWidgetEvent click_ev(tb::EVENT_TYPE_CLICK);
		m_close_button.InvokeEvent(click_ev);
		return true;
	}
	return TBWindow::OnEvent(ev);
}
