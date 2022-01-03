//#include "MeshWindow.h"
//#include "PathUtil.h"
//#include "Log.h"
//#include "StringUtil.h"
//#include "MathUtils.h"
//#include "InstancedMesh.h"
//#include "tb_editfield.h"
//#include "tb_inline_select.h"
//#include <string>
//
//MeshWindow::MeshWindow(
//	tb::TBWidget* root,
//	std::shared_ptr<InstancedMesh> mesh) :
//	Window(root),
//	_mesh(mesh),
//	_currentID(0)
//{
//	Log::Debug("[MeshWindow] constructor, instance at %p", this);
//	std::string resourcePath = PathUtil::GUIPath() + "ui_meshwindow.txt";
//	LoadResourceFile(resourcePath.c_str());
//
//	m_close_button.SetState(tb::WIDGET_STATE_DISABLED, true);
//	refreshInstances();
//}
//
//MeshWindow::~MeshWindow()
//{
//}
//
//bool MeshWindow::OnEvent(const tb::TBWidgetEvent & ev)
//{
//
//	if (ev.type == tb::EVENT_TYPE_CLICK)
//	{
//		if (ev.target->GetID() == TBIDC("button-mesh-clear"))
//		{
//			_mesh->resize(0);
//		}
//		else if (ev.target->GetID() == TBIDC("button-instance-select"))
//		{
//			_currentID = ev.target->data.GetInt();
//			refresh();
//		}
//		else if (ev.target->GetID() == TBIDC("button-instance-create"))
//		{
//			_currentID = _mesh->addInstance();
//			refresh(); 
//		}
//		else if (ev.target->GetID() == TBIDC("button-instance-remove"))
//		{
//			_mesh->removeInstance(_currentID);
//			if (_mesh->getInstanceCount() == 0)
//			{
//				_currentID = 0;
//			}
//			else
//			{
//				_currentID = _mesh->getAllIDs()[0];
//			}
//			refresh();
//		}
//		return true;
//	}
//	else if (ev.type == tb::EVENT_TYPE_CHANGED)
//	{
//		if (ev.target->GetID() == TBIDC("instance-rotation-x"))
//		{
//			const glm::quat rotation = _mesh->getRotation(_currentID);
//			glm::vec3 euler = glm::eulerAngles(rotation);
//			euler.x = ev.target->GetValueDouble() * float(M_PI / 180.0);
//			glm::quat newRotation = glm::quat(euler);
//			_mesh->setRotation(newRotation, _currentID);
//		}
//		if (ev.target->GetID() == TBIDC("instance-rotation-y"))
//		{
//			const glm::quat rotation = _mesh->getRotation(_currentID);
//			glm::vec3 euler = glm::eulerAngles(rotation);
//			euler.y = ev.target->GetValueDouble() * float(M_PI / 180.0);
//			glm::quat newRotation = glm::quat(euler);
//			_mesh->setRotation(newRotation, _currentID);
//		}
//		if (ev.target->GetID() == TBIDC("instance-rotation-z"))
//		{
//			const glm::quat rotation = _mesh->getRotation(_currentID);
//			glm::vec3 euler = glm::eulerAngles(rotation);
//			euler.z = ev.target->GetValueDouble() * float(M_PI / 180.0);
//			glm::quat newRotation = glm::quat(euler);
//			_mesh->setRotation(newRotation, _currentID);
//		}
//	}
//	else if (ev.type == tb::EVENT_TYPE_KEY_DOWN &&
//		ev.special_key == tb::TB_KEY_ENTER)
//	{
//		if (ev.target->GetID() == TBIDC("instance-position-x"))
//		{
//			glm::vec3 pos = _mesh->getPosition(_currentID);
//			pos.x = std::stof(std::string(ev.target->GetText()));
// 			_mesh->setPosition(pos, _currentID);
//		}
//		else if (ev.target->GetID() == TBIDC("instance-position-y"))
//		{
//			glm::vec3 pos = _mesh->getPosition(_currentID);
//			pos.y = std::stof(std::string(ev.target->GetText()));
//			_mesh->setPosition(pos, _currentID);
//		}
//		else if (ev.target->GetID() == TBIDC("instance-position-z"))
//		{
//			glm::vec3 pos = _mesh->getPosition(_currentID);
//			pos.z = std::stof(std::string(ev.target->GetText()));
//			_mesh->setPosition(pos, _currentID);
//		}
//		else if (ev.target->GetID() == TBIDC("instance-scale-x"))
//		{
//			glm::vec3 scale = _mesh->getScale(_currentID);
//			scale.x = std::stof(std::string(ev.target->GetText()));
//			_mesh->setScale(scale, _currentID);
//		}
//		else if (ev.target->GetID() == TBIDC("instance-scale-y"))
//		{
//			glm::vec3 scale = _mesh->getScale(_currentID);
//			scale.y = std::stof(std::string(ev.target->GetText()));
//			_mesh->setScale(scale, _currentID);
//		}
//		else if (ev.target->GetID() == TBIDC("instance-scale-z"))
//		{
//			glm::vec3 scale = _mesh->getScale(_currentID);
//			scale.z = std::stof(std::string(ev.target->GetText()));
//			_mesh->setScale(scale, _currentID);
//		}
//	}
//	return Window::OnEvent(ev);
//}
//
//void MeshWindow::SetCurrentID(const int newID)
//{
//	_currentID = newID;
//	refresh();
//}
//
//void MeshWindow::refresh()
//{
//	refreshInstances();
//}
//
//void MeshWindow::refreshInstances()
//{
//	tb::TBTextField* nameField = GetWidgetByIDAndType<tb::TBTextField>(TBIDC("instance-current"));
//	nameField->SetText(StringUtil::IntToString(_currentID).c_str());
//
//	tb::TBEditField* posXField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("instance-position-x"));
//	posXField->SetText(StringUtil::FloatToString(_mesh->getPosition(_currentID).x).c_str());
//	tb::TBEditField* posYField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("instance-position-y"));
//	posYField->SetText(StringUtil::FloatToString(_mesh->getPosition(_currentID).y).c_str());
//	tb::TBEditField* posZField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("instance-position-z"));
//	posZField->SetText(StringUtil::FloatToString(_mesh->getPosition(_currentID).z).c_str());
//
//	const glm::quat rotation = _mesh->getRotation(_currentID);
//	glm::vec3 euler = glm::eulerAngles(rotation);
//	glm::vec3 degrees = euler * float(180.0f / M_PI);
//	tb::TBInlineSelect* rotXField = GetWidgetByIDAndType<tb::TBInlineSelect>(TBIDC("instance-rotation-x"));
//	rotXField->SetText(StringUtil::FloatToString(degrees.x).c_str());
//	tb::TBInlineSelect* rotYField = GetWidgetByIDAndType<tb::TBInlineSelect>(TBIDC("instance-rotation-y"));
//	rotYField->SetText(StringUtil::FloatToString(degrees.y).c_str());
//	tb::TBInlineSelect* rotZField = GetWidgetByIDAndType<tb::TBInlineSelect>(TBIDC("instance-rotation-z"));
//	rotZField->SetText(StringUtil::FloatToString(degrees.z).c_str());
//
//	tb::TBEditField* scaleXField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("instance-scale-x"));
//	scaleXField->SetText(StringUtil::FloatToString(_mesh->getScale(_currentID).x).c_str());
//	tb::TBEditField* scaleYField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("instance-scale-y"));
//	scaleYField->SetText(StringUtil::FloatToString(_mesh->getScale(_currentID).y).c_str());
//	tb::TBEditField* scaleZField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("instance-scale-z"));
//	scaleZField->SetText(StringUtil::FloatToString(_mesh->getScale(_currentID).z).c_str());
//
//	tb::TBLayout* node = GetWidgetByIDAndType<tb::TBLayout>(TBIDC("instances-list"));
//	tb::TBWidget* child = node->GetFirstChild();
//	while (child != nullptr)
//	{
//		node->RemoveChild(child);
//		child = node->GetFirstChild();
//	}
//	
//	auto allIDs = _mesh->getAllIDs();
//	for (size_t i = 0; i < allIDs.size(); i++)
//	{
//		tb::TBButton* materialButton = new tb::TBButton();
//		materialButton->SetSkinBg(TBIDC("TBButton.flat"));
//		materialButton->SetText(StringUtil::IntToString(allIDs[i]).c_str());
//		materialButton->SetID(TBIDC("button-instance-select"));
//		materialButton->data = tb::TBValue(int(allIDs[i]));
//		materialButton->SetGravity(tb::WIDGET_GRAVITY_LEFT_RIGHT);
//		node->AddChild(materialButton);
//	}
//}
