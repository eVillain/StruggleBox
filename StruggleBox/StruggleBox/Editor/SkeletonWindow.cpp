#include "SkeletonWindow.h"

#include "GUI.h"
#include "LayoutNode.h"
#include "ValueEditNode.h"

#include "PathUtil.h"
#include "Log.h"
#include "StringUtil.h"
#include "MathUtils.h"
#include "MaterialData.h"
#include "VoxelData.h"
#include <string>

const glm::vec2 SkeletonWindow::WINDOW_SIZE = glm::vec2(260.f, 600.f);

SkeletonWindow::SkeletonWindow(GUI& gui, Skeleton& skeleton)
	: WindowNode(gui, WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, "Skeleton")
	, _skeleton(skeleton)
	, m_layoutNode(nullptr)
	, m_timeEditNode(nullptr)
{
	_currentJoint = ROOT_JOINT_ID;
	_currentAnimation = _skeleton._animations.begin()->first;
	_currentFrame = 0;

	const glm::vec2 SMALL_VALUE_SIZE = glm::vec2(m_layoutNode->getContentSize().x - 4.f, GUI::DEFAULT_SLIDER_HEIGHT);

	m_layoutNode = m_gui.createCustomNode<LayoutNode>();
	m_layoutNode->setLayoutType(LayoutType::Column);
	m_layoutNode->setContentSize(m_contentNode->getContentSize());
	m_contentNode->addChild(m_layoutNode);

	m_timeEditNode = m_gui.createCustomNode<ValueEditNode<float>>(m_gui);
	m_timeEditNode->setContentSize(SMALL_VALUE_SIZE);
	m_timeEditNode->setValue("Time: ", _currentTime, 0.f, 100.f);
	m_layoutNode->addChild(m_timeEditNode);

	refresh();
}

SkeletonWindow::~SkeletonWindow()
{
}
//
//bool SkeletonWindow::OnEvent(const tb::TBWidgetEvent & ev)
//{
//	if (ev.type == tb::EVENT_TYPE_CLICK)
//	{
//		if (ev.target->GetID() == TBIDC("button-bone-select"))
//		{
//			_currentJoint = ev.target->data.GetInt();
//			refresh();
//		}
//		if (ev.target->GetID() == TBIDC("button-bone-create"))
//		{
//			_skeleton->addJoint(_currentJoint);
//			refresh();
//		}
//		else if (ev.target->GetID() == TBIDC("button-bone-remove"))
//		{
//
//		}
//		else if (ev.target->GetID() == TBIDC("button-bone-parent"))
//		{
//			if (_skeleton->_joints[_currentJoint].parent &&
//				_skeleton->_joints[_currentJoint].parent != _currentJoint)
//			{
//				_currentJoint = _skeleton->_joints[_currentJoint].parent;
//				refresh();
//			}
//		}
//		else if (ev.target->GetID() == TBIDC("button-add-frame"))
//		{
//			_skeleton->_animations[_currentAnimation][_currentJoint].push_back(KeyFrame());
//		}
//		else if (ev.target->GetID() == TBIDC("button-next-frame"))
//		{
//			if (_currentFrame == _skeleton->_animations[_currentAnimation][_currentJoint].size() - 1)
//				_currentFrame = 0;
//			else
//				_currentFrame = _currentFrame + 1;
//			refresh();
//		}
//		else if (ev.target->GetID() == TBIDC("button-prev-frame"))
//		{
//			if (_currentFrame == 0)
//				_currentFrame = _skeleton->_animations[_currentAnimation][_currentJoint].size() - 1;
//			else
//				_currentFrame = _currentFrame - 1;
//			refresh();
//		}
//		else if (ev.target->GetID() == TBIDC("button-add-animation"))
//		{
//			_skeleton->addAnimation("New Animation");
//		}
//		return true;
//	}
//	else if (ev.type == tb::EVENT_TYPE_CHANGED)
//	{
//		KeyFrame& frame = _skeleton->getKeyFrame(_currentJoint, _currentAnimation, _currentFrame);
//
//		if (ev.target->GetID() == TBIDC("bone-rotation-x"))
//		{
//			const glm::quat rotation = frame.data.rotation;
//			glm::vec3 euler = glm::eulerAngles(rotation);
//			euler.x = ev.target->GetValueDouble() * float(M_PI / 180.0);
//			frame.data.rotation = glm::quat(euler);
//		}
//		if (ev.target->GetID() == TBIDC("bone-rotation-y"))
//		{
//			const glm::quat rotation = frame.data.rotation;
//			glm::vec3 euler = glm::eulerAngles(rotation);
//			euler.y = ev.target->GetValueDouble() * float(M_PI / 180.0);
//			frame.data.rotation = glm::quat(euler);
//		}
//		if (ev.target->GetID() == TBIDC("bone-rotation-z"))
//		{
//			const glm::quat rotation = frame.data.rotation;
//			glm::vec3 euler = glm::eulerAngles(rotation);
//			euler.z = ev.target->GetValueDouble() * float(M_PI / 180.0);
//			frame.data.rotation = glm::quat(euler);
//		}
//		if (ev.target->GetID() == TBIDC("slider-timeline"))
//		{
//			_currentTime = ev.target->GetValueDouble();
//		}
//	}
//	else if (ev.type == tb::EVENT_TYPE_KEY_DOWN &&
//		ev.special_key == tb::TB_KEY_ENTER)
//	{
//		if (ev.target->GetText().Length() == 0) return true;
//		KeyFrame& frame = _skeleton->getKeyFrame(_currentJoint, _currentAnimation, _currentFrame);
//
//		if (ev.target->GetID() == TBIDC("bone-position-x"))
//		{
//			glm::vec3 pos = frame.data.position;
//			pos.x = std::stof(std::string(ev.target->GetText()));
//			frame.data.position = pos;
//		}
//		else if (ev.target->GetID() == TBIDC("bone-position-y"))
//		{
//			glm::vec3 pos = frame.data.position;
//			pos.y = std::stof(std::string(ev.target->GetText()));
//			frame.data.position = pos;
//		}
//		else if (ev.target->GetID() == TBIDC("bone-position-z"))
//		{
//			glm::vec3 pos = frame.data.position;
//			pos.z = std::stof(std::string(ev.target->GetText()));
//			frame.data.position = pos;
//		}
//		else if (ev.target->GetID() == TBIDC("bone-scale-x"))
//		{
//			glm::vec3 scale = frame.data.scale;
//			scale.x = std::stof(std::string(ev.target->GetText()));
//			frame.data.scale = scale;
//		}
//		else if (ev.target->GetID() == TBIDC("bone-scale-y"))
//		{
//			glm::vec3 scale = frame.data.scale;
//			scale.y = std::stof(std::string(ev.target->GetText()));
//			frame.data.scale = scale;
//		}
//		else if (ev.target->GetID() == TBIDC("bone-scale-z"))
//		{
//			glm::vec3 scale = frame.data.scale;
//			scale.z = std::stof(std::string(ev.target->GetText()));
//			frame.data.scale = scale;
//		}
//		else if (ev.target->GetID() == TBIDC("bone-name"))
//		{
//			_skeleton->_joints[_currentJoint].name = ev.target->GetText();
//			refresh();
//		}
//		else if (ev.target->GetID() == TBIDC("bone-name"))
//		{
//			_skeleton->_joints[_currentJoint].name = ev.target->GetText();
//			refresh();
//		}
//	}
//	return Window::OnEvent(ev);
//}


void SkeletonWindow::refresh()
{
	KeyFrame& frame = _skeleton.getKeyFrame(_currentJoint, _currentAnimation, _currentFrame);
	const Joint& joint = _skeleton.getJoint(_currentJoint);

	m_layoutNode->refresh();

	//tb::TBEditField* nameField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("bone-name"));
	//if (_currentJoint == ROOT_JOINT_ID)
	//	nameField->SetText("ROOT");
	//else
	//	nameField->SetText(joint.name.c_str());

	//tb::TBInlineSelect* rotXField = GetWidgetByIDAndType<tb::TBInlineSelect>(TBIDC("bone-rotation-x"));
	//tb::TBInlineSelect* rotYField = GetWidgetByIDAndType<tb::TBInlineSelect>(TBIDC("bone-rotation-y"));
	//tb::TBInlineSelect* rotZField = GetWidgetByIDAndType<tb::TBInlineSelect>(TBIDC("bone-rotation-z"));
	//glm::vec3 rotation = glm::eulerAngles(frame.data.rotation) * float(180.0 / M_PI);
	//rotXField->SetValue(rotation.x);
	//rotYField->SetValue(rotation.y);
	//rotZField->SetValue(rotation.z);
	//tb::TBEditField* posXField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("bone-position-x"));
	//tb::TBEditField* posYField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("bone-position-y"));
	//tb::TBEditField* posZField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("bone-position-z"));
	//posXField->SetText(StringUtil::FloatToString(frame.data.position.x).c_str());
	//posYField->SetText(StringUtil::FloatToString(frame.data.position.y).c_str());
	//posZField->SetText(StringUtil::FloatToString(frame.data.position.z).c_str());

	//tb::TBEditField* scaleXField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("bone-scale-x"));
	//tb::TBEditField* scaleYField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("bone-scale-y"));
	//tb::TBEditField* scaleZField = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("bone-scale-z"));
	//scaleXField->SetText(StringUtil::FloatToString(frame.data.scale.x).c_str());
	//scaleYField->SetText(StringUtil::FloatToString(frame.data.scale.y).c_str());
	//scaleZField->SetText(StringUtil::FloatToString(frame.data.scale.z).c_str());

	//tb::TBLayout* jointList = GetWidgetByIDAndType<tb::TBLayout>(TBIDC("joints-list"));
	//tb::TBWidget* jointChild = jointList->GetFirstChild();
	//while (jointChild != nullptr)
	//{
	//	jointList->RemoveChild(jointChild);
	//	jointChild = jointList->GetFirstChild();
	//}

	//refreshJointList(jointList, ROOT_JOINT_ID, 0);

	//tb::TBLayout* frameList = GetWidgetByIDAndType<tb::TBLayout>(TBIDC("frames-list"));
	//tb::TBWidget* frameChild = frameList->GetFirstChild();
	//while (frameChild != nullptr)
	//{
	//	frameList->RemoveChild(frameChild);
	//	frameChild = frameList->GetFirstChild();
	//}

	//auto& frames = _skeleton->_animations[_currentAnimation][_currentJoint];
	//for (size_t i = 0; i < frames.size(); i++)
	//{
	//	tb::TBButton* frameButton = new tb::TBButton();
	//	frameButton->SetSkinBg(TBIDC("TBButton.flat"));
	//	frameButton->SetText(("KeyFrame " + StringUtil::IntToString(i)).c_str());
	//	frameButton->SetID(TBIDC("button-frame-select"));
	//	frameButton->data = tb::TBValue(int(i));
	//	frameButton->SetGravity(tb::WIDGET_GRAVITY_LEFT_RIGHT);
	//	frameList->AddChild(frameButton);
	//}

	//tb::TBLayout* animationList = GetWidgetByIDAndType<tb::TBLayout>(TBIDC("animations-list"));
	//tb::TBWidget* animationChild = animationList->GetFirstChild();
	//while (animationChild != nullptr)
	//{
	//	animationList->RemoveChild(animationChild);
	//	animationChild = animationList->GetFirstChild();
	//}

	//for (auto& animation : _skeleton->_animations)
	//{
	//	tb::TBButton* frameButton = new tb::TBButton();
	//	frameButton->SetSkinBg(TBIDC("TBButton.flat"));
	//	frameButton->SetText(animation.first.c_str());
	//	frameButton->SetID(TBIDC("button-animation-select"));
	//	frameButton->data = tb::TBValue(animation.first.c_str());
	//	frameButton->SetGravity(tb::WIDGET_GRAVITY_LEFT_RIGHT);
	//	animationList->AddChild(frameButton);
	//}
}
//
//void SkeletonWindow::refreshJointList(
//	tb::TBLayout * node,
//	JointID bone,
//	int depth)
//{
//	tb::TBButton* boneButton = new tb::TBButton();
//	boneButton->SetSkinBg(TBIDC("TBButton.flat"));
//	if (bone == ROOT_JOINT_ID)
//		boneButton->SetText("ROOT");
//	else
//		boneButton->SetText((std::string(depth, '-') + _skeleton->_joints[bone].name).c_str());
//
//	boneButton->SetID(TBIDC("button-bone-select"));
//	boneButton->data = tb::TBValue(bone);
//	boneButton->SetGravity(tb::WIDGET_GRAVITY_LEFT_RIGHT);
//
//	node->AddChild(boneButton);
//
//	if (_skeleton->_children.find(bone) != _skeleton->_children.end())
//	{
//		for (size_t i = 0; i < _skeleton->_children[bone].size(); i++)
//		{
//			refreshJointList(node, _skeleton->_children[bone][i], depth + 1);
//		}
//	}
//}
