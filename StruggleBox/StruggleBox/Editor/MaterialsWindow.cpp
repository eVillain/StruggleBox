#include "MaterialsWindow.h"
#include "PathUtil.h"
#include "Log.h"
#include "StringUtil.h"
#include "MathUtils.h"
#include "tb_editfield.h"
#include <string>

MaterialsWindow::MaterialsWindow(
	tb::TBWidget* root,
	MaterialData* materials) :
	Window(root),
	_materials(materials),
	_currentID(1)
{
	Log::Debug("[MaterialsWindow] constructor, instance at %p", this);
	std::string resourcePath = PathUtil::GUIPath() + "ui_materialwindow.txt";
	LoadResourceFile(resourcePath.c_str());

	refreshMaterials();
	m_close_button.SetState(tb::WIDGET_STATE_DISABLED, true);
}

MaterialsWindow::~MaterialsWindow()
{
}

bool MaterialsWindow::OnEvent(const tb::TBWidgetEvent & ev)
{
	if (ev.type == tb::EVENT_TYPE_CLICK)
	{
		if (ev.target->GetID() == TBIDC("button-set-material"))
		{
			tb::TBEditField* nameField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("material-name"));
			std::string name = nameField->GetText();
			if (name.length() > 0)
			{
				_materials->setName(_currentID, name);
				refreshMaterials();
			}
		}
		else if (ev.target->GetID() == TBIDC("button-material-select"))
		{
			_currentID = ev.target->data.GetInt();
			refreshProperties();
		}
		else if (ev.target->GetID() == TBIDC("toggleSphere"))
		{
			_displaySphere = ev.target->GetValue();
		}
		else if (ev.target->GetID() == TBIDC("toggleArray"))
		{
			_displayArray = ev.target->GetValue();
		}
		else if (ev.target->GetID() == TBIDC("toggleRotate"))
		{
			_rotateMesh = ev.target->GetValue();
		}
		else if (ev.target->GetID() == TBIDC("toggleLightRotate"))
		{
			_rotateLight = ev.target->GetValue();
		}
		else if (ev.target->GetID() == TBIDC("toggleLightColor"))
		{
			_colorLight = ev.target->GetValue();
		}
		return true;
	}
	else if (ev.type == tb::EVENT_TYPE_CHANGED)
	{
		if (ev.target->GetID() == TBIDC("material-ID-slider"))
		{
			_currentID = ev.target->GetValue();
			refreshProperties();
		}
		else if (ev.target->GetID() == TBIDC("red"))
		{
			double value = ev.target->GetValueDouble();
			(*_materials)[_currentID].albedo.r = value;
		}
		else if (ev.target->GetID() == TBIDC("green"))
		{
			double value = ev.target->GetValueDouble();
			(*_materials)[_currentID].albedo.g = value;
		}
		else if (ev.target->GetID() == TBIDC("blue"))
		{
			double value = ev.target->GetValueDouble();
			(*_materials)[_currentID].albedo.b = value;
		}
		else if (ev.target->GetID() == TBIDC("alpha"))
		{
			double value = ev.target->GetValueDouble();
			(*_materials)[_currentID].albedo.a = value;
		}
		else if (ev.target->GetID() == TBIDC("metalness"))
		{
			double value = ev.target->GetValueDouble();
			(*_materials)[_currentID].metalness = value;
		}
		else if (ev.target->GetID() == TBIDC("roughness"))
		{
			double value = ev.target->GetValueDouble();
			(*_materials)[_currentID].roughness = value;
		}
		else if (ev.target->GetID() == TBIDC("noise-amount"))
		{
			double value = ev.target->GetValueDouble();
			(*_materials)[_currentID].noiseAmount = value;
		}
		else if (ev.target->GetID() == TBIDC("noise-scale"))
		{
			double value = ev.target->GetValueDouble();
			MathUtils::Clamp(value, 0.0000000001, 1.0);
			(*_materials)[_currentID].noiseScale = value;
		}
	}
	else if (ev.type == tb::EVENT_TYPE_KEY_DOWN &&
			ev.special_key == tb::TB_KEY_ENTER)
	{
		if (ev.target->GetID() == TBIDC("material-name"))
		{
			std::string name = ev.target->GetText();
			if (name.length() > 0)
			{
				_materials->setName(_currentID, name);
				refreshMaterials();
			}
		}
		else if (ev.target->GetID() == TBIDC("material-ID"))
		{
			int newID = std::stoi(std::string(ev.target->GetText()));
			if (newID > 0 && newID < 256) 
			{
				_currentID = newID;
				refreshProperties();
			}
		}
	}

	return Window::OnEvent(ev);
}

void MaterialsWindow::refresh()
{
	refreshMaterials();
	refreshProperties();
}

void MaterialsWindow::refreshMaterials()
{
	tb::TBLayout* node = GetWidgetByIDAndType<tb::TBLayout>(TBIDC("materials-list"));

	tb::TBWidget* child = node->GetFirstChild();
	while (child != nullptr)
	{
		node->RemoveChild(child);
		child = node->GetFirstChild();
	}

	for (size_t i = 1; i < 256; i++)
	{
		if (_materials->getName(i).length() == 0) continue;
		Log::Debug("[MaterialsWindow] added material with name %s", _materials->getName(i).c_str());

		tb::TBButton* materialButton = new tb::TBButton();
		materialButton->SetSkinBg(TBIDC("TBButton.flat"));
		materialButton->SetText(_materials->getName(i).c_str());
		materialButton->SetID(TBIDC("button-material-select"));
		materialButton->data = tb::TBValue((int)i);
		materialButton->SetGravity(tb::WIDGET_GRAVITY_LEFT_RIGHT);
		node->AddChild(materialButton);
	}
}

void MaterialsWindow::refreshProperties()
{
	tb::TBSlider* idSlider = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("material-ID-slider"));
	idSlider->SetValue(_currentID);

	tb::TBEditField* idField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("material-ID"));
	idField->SetText(StringUtil::IntToString(_currentID).c_str());

	tb::TBEditField* nameField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("material-name"));
	nameField->SetText(_materials->getName(_currentID).c_str());

	tb::TBSlider* sliderR = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("red"));
	tb::TBSlider* sliderG = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("green"));
	tb::TBSlider* sliderB = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("blue"));
	tb::TBSlider* sliderA = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("alpha"));
	tb::TBSlider* sliderM = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("metalness"));
	tb::TBSlider* sliderRg = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("roughness"));
	tb::TBSlider* sliderNa = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("noise-amount"));
	tb::TBSlider* sliderNs = GetWidgetByIDAndType<tb::TBSlider>(TBIDC("noise-scale"));
	sliderR->SetValueDouble((*_materials)[_currentID].albedo.r);
	sliderG->SetValueDouble((*_materials)[_currentID].albedo.g);
	sliderB->SetValueDouble((*_materials)[_currentID].albedo.b);
	sliderA->SetValueDouble((*_materials)[_currentID].albedo.a);
	sliderM->SetValueDouble((*_materials)[_currentID].metalness);
	sliderRg->SetValueDouble((*_materials)[_currentID].roughness);
	sliderNa->SetValueDouble((*_materials)[_currentID].noiseAmount);
	sliderNs->SetValueDouble((*_materials)[_currentID].noiseScale);


}
