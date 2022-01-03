#pragma once

#include "WindowNode.h"
#include "ValueEditNode.h"
#include "Skeleton.h"
#include <memory>

class LayoutNode;

class SkeletonWindow : public WindowNode
{
public:
	SkeletonWindow(
		GUI& gui,
		Skeleton& skeleton);
	~SkeletonWindow();

	//bool OnEvent(const tb::TBWidgetEvent & ev);

	void refresh();

	//void refreshJointList(
	//	tb::TBLayout* node,
	//	JointID bone,
	//	int depth);

	void setCurrentJoint(JointID joint) { _currentJoint = joint; refresh(); }
	JointID getCurrentJoint() { return _currentJoint; }
	void setCurrentAnimation(const std::string animation) { _currentAnimation = animation; refresh(); }
	const std::string getCurrentAnimation() { return _currentAnimation; }
	void setCurrentFrame(uint16_t frame) { _currentFrame = frame; refresh(); }
	uint16_t getCurrentFrame() { return _currentFrame; }
	void setCurrentTime(float time) { _currentTime = time; refresh(); }
	float getCurrentTime() { return _currentTime; }
private:

	static const glm::vec2 WINDOW_SIZE;

	Skeleton& _skeleton;
	std::string _currentAnimation;
	JointID _currentJoint;
	uint16_t _currentFrame;
	float _currentTime;

	LayoutNode* m_layoutNode;
	ValueEditNode<float>* m_timeEditNode;
};

