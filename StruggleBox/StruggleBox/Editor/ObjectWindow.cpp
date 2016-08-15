#include "ObjectWindow.h"
#include "PathUtil.h"
#include "Log.h"
#include "StringUtil.h"
#include "MathUtils.h"
#include "MaterialData.h"
#include "VoxelData.h"
#include "tb_editfield.h"
#include <string>

ObjectWindow::ObjectWindow(
	tb::TBWidget* root,
	std::shared_ptr<VoxelData> voxels,
	MaterialData* materials) :
	Window(root),
	_voxels(voxels),
	_materials(materials),
	_currentID(1),
	_editTool(ObjectTool_None),
	_colorLights(false)
{
	Log::Debug("[ObjectWindow] constructor, instance at %p", this);
	std::string resourcePath = PathUtil::GUIPath() + "ui_voxelobjectwindow.txt";
	LoadResourceFile(resourcePath.c_str());

	m_close_button.SetState(tb::WIDGET_STATE_DISABLED, true);
	refreshMaterials();
}

ObjectWindow::~ObjectWindow()
{
}

bool ObjectWindow::OnEvent(const tb::TBWidgetEvent & ev)
{
	if (ev.type == tb::EVENT_TYPE_CLICK)
	{
		if (ev.target->GetID() == TBIDC("button-material-select"))
		{
			_currentID = ev.target->data.GetInt();
		}
		else if (ev.target->GetID() == TBIDC("button-cursor"))
		{
			_editTool = ObjectTool_None;
		}
		else if (ev.target->GetID() == TBIDC("button-selection"))
		{
			_editTool = ObjectTool_Selection;
		}
		else if (ev.target->GetID() == TBIDC("button-block"))
		{
			_editTool = ObjectTool_Block;
		}
		else if (ev.target->GetID() == TBIDC("button-column"))
		{
			_editTool = ObjectTool_Column;
		}
		else if (ev.target->GetID() == TBIDC("button-clear"))
		{
			_voxels->clear();
		}
		else if (ev.target->GetID() == TBIDC("button-generateTree"))
		{
			_voxels->generateGrass(0);
		}
		else if (ev.target->GetID() == TBIDC("button-rotateYPos"))
		{
			_voxels->rotateY(false);
		}
		else if (ev.target->GetID() == TBIDC("button-rotateYNeg"))
		{
			_voxels->rotateY(true);
		}
		return true;
	}
	else if (ev.type == tb::EVENT_TYPE_CHANGED)
	{
		if (ev.target->GetID() == TBIDC("material-ID-slider"))
		{
			_currentID = ev.target->GetValue();
		}
	}

	return Window::OnEvent(ev);
}

void ObjectWindow::SetCurrentID(const int newID)
{
	_currentID = 0xff & newID;
	refresh();
}

void ObjectWindow::refresh()
{
	refreshMaterials();
}

void ObjectWindow::refreshMaterials()
{
	tb::TBTextField* nameField = GetWidgetByIDAndType<tb::TBTextField>(TBIDC("material-name"));
	std::string name = _materials->getName(_currentID);

	if (_currentID == EMPTY_VOXEL)
		name = "EMPTY VOXEL (0)";
	else if (name.length() == 0)
		name = "unknown (" + StringUtil::IntToString(_currentID) + ")";

	nameField->SetText((name + " (" + StringUtil::IntToString(_currentID) + ")").c_str());

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
		tb::TBButton* materialButton = new tb::TBButton();
		materialButton->SetSkinBg(TBIDC("TBButton.flat"));
		//std::string materialName = 
		materialButton->SetText((StringUtil::IntToString(i) + " - " + _materials->getName(i)).c_str());
		materialButton->SetID(TBIDC("button-material-select"));
		materialButton->data = tb::TBValue((int)i);
		materialButton->SetGravity(tb::WIDGET_GRAVITY_LEFT_RIGHT);
		node->AddChild(materialButton);
	}
}
