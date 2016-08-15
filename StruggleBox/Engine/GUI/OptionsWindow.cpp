#include "OptionsWindow.h"
#include "Options.h"
#include "PathUtil.h"
#include "StringUtil.h"
#include "Log.h"
#include "tb_tab_container.h"
#include "tb_inline_select.h"

OptionsWindow::OptionsWindow(
	tb::TBWidget * root,
	std::shared_ptr<Options> options) :
	Window(root),
	_options(options)
{
	std::string path = PathUtil::GUIPath() + "ui_optionswindow.txt";

	LoadResourceFile(path.c_str());

	// Get all the options and add them in to our menu
	std::map<const std::string, Attribute*>& allOptions = options->getAllOptions();
	std::map<const std::string, Attribute*>::iterator it;
	for (it = allOptions.begin(); it != allOptions.end(); it++) {
		std::string category = "tab-general";
		if (it->first.substr(0, 2) == "a_") { category = "tab-audio"; }
		else if (it->first.substr(0, 2) == "d_") { category = "tab-debug"; }
		else if (it->first.substr(0, 2) == "e_") { category = "tab-editor"; }
		else if (it->first.substr(0, 2) == "i_") { category = "tab-input"; }
		else if (it->first.substr(0, 2) == "r_") { category = "tab-renderer"; }
		tb::TBLayout* node = GetWidgetByIDAndType<tb::TBLayout>(TBIDC(category.c_str()));
		if (!node)
		{
			Log::Error("[OptionsWindow] No % category node!", category.c_str());
			return;
		}
		tb::TBClickLabel* label = new tb::TBClickLabel();
		label->SetText(it->first.c_str());
		if (it->second->IsType<bool>())
		{
			tb::TBRadioButton* button = new tb::TBRadioButton();
			button->SetValue(it->second->as<bool>());
			button->SetID(TBIDC(it->first.c_str()));
			label->GetContentRoot()->AddChild(button);
		}
		else if (it->second->IsType<int>())
		{
			tb::TBInlineSelect* select = new tb::TBInlineSelect();
			select->SetValue(it->second->as<int>());
			select->SetID(TBIDC(it->first.c_str()));
			label->GetContentRoot()->AddChild(select);
		}
		else if (it->second->IsType<float>())
		{
			tb::TBEditField* editField = new tb::TBEditField();
			editField->SetText(StringUtil::FloatToString(it->second->as<float>()).c_str());
			editField->SetID(TBIDC(it->first.c_str()));
			label->GetContentRoot()->AddChild(editField);
		}
		else if (it->second->IsType<std::string>())
		{
			tb::TBEditField* editField = new tb::TBEditField();
			editField->SetText(it->second->as<std::string>().c_str());
			editField->SetID(TBIDC(it->first.c_str()));
			label->GetContentRoot()->AddChild(editField);
		}
		node->AddChild(label);
	}
}

bool OptionsWindow::OnEvent(const tb::TBWidgetEvent & ev)
{
	if (ev.type == tb::EVENT_TYPE_CLICK)
	{
		// Check all the options for the corresponding one
		std::map<const std::string, Attribute*>& allOptions = _options->getAllOptions();
		std::map<const std::string, Attribute*>::iterator it;
		for (it = allOptions.begin(); it != allOptions.end(); it++)
		{
			if (ev.target->GetID() == TBIDC(it->first.c_str()))
			{
				if (it->second->IsType<bool>())
				{
					tb::TBRadioButton *button = GetWidgetByIDAndType<tb::TBRadioButton>(TBIDC(it->first.c_str()));
					if (!button) return false;
					it->second->as<bool>() = button->GetValue();
				}
				else if (it->second->IsType<int>())
				{
					tb::TBInlineSelect *select = GetWidgetByIDAndType<tb::TBInlineSelect>(TBIDC(it->first.c_str()));
					if (!select) return false;
					it->second->as<int>() = select->GetValue();
				}
				else if (it->second->IsType<float>())
				{
					tb::TBEditField *select = GetWidgetByIDAndType<tb::TBEditField>(TBIDC(it->first.c_str()));
					if (!select) return false;
					float value = std::stof(std::string(select->GetText()));
					it->second->as<float>() = value;
				}
				else if (it->second->IsType<std::string>())
				{
					tb::TBEditField *editField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC(it->first.c_str()));
					if (!editField) return false;
					it->second->as<std::string>() = editField->GetText();
				}
				return true;
			}
		}
	}
	return Window::OnEvent(ev);
}
