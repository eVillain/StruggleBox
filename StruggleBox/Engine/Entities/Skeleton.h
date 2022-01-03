#ifndef SKELETON_H
#define SKELETON_H

#include "GFXDefines.h"
#include <string>
#include <vector>
#include <map>

struct KeyFrame
{
	double time;
	InstanceTransformData data;
};

typedef uint16_t JointID;
const JointID ROOT_JOINT_ID = 0;

struct Joint
{
	std::string name;
	JointID parent;
	bool inheritScale;
	bool inheritRotation;
};

struct searchResult
{
	JointID current;
	float distance;
};

class Skeleton
{
	friend class SkeletonWindow;
public:
	Skeleton();

	Joint& getRoot() { return _joints[0]; }

	JointID addJoint(const JointID parentID = ROOT_JOINT_ID);

	const Joint& getJoint(const JointID jointID) { return _joints[jointID]; }

	JointID getNearestJoint(
		const std::string& animation,
		const glm::vec3& position,
		const float time);

	std::vector<InstanceTransformData> getInstanceData(
		const std::string& animation,
		const float time,
		const glm::vec3& position = glm::vec3(),
		const glm::quat& rotation = glm::quat(),
		const glm::vec3& scale = glm::vec3(1));

	void addAnimation(const std::string animation);

	KeyFrame& getKeyFrame(
		const JointID joint,
		const std::string& animation,
		const uint16_t frame);

private:
	std::vector<Joint> _joints;
	typedef std::vector<KeyFrame> AnimationStream;
	typedef std::map<JointID, AnimationStream> SkeletalAnimation;
	std::map<std::string, SkeletalAnimation> _animations;
	std::map<JointID, std::vector<JointID>> _children;

	void getInstanceData(
		std::vector<InstanceTransformData> instances,
		const std::string& animation,
		const float time,
		JointID currentJoint,
		const glm::vec3 parentWorldPosition,
		const glm::quat parentWorldRotation,
		const glm::vec3 parentWorldScale);

	InstanceTransformData getFrameAtTime(AnimationStream& stream, const float time);

	void getNearestJoint(
		const std::string& animation,
		const glm::vec3& position,
		const float time,
		JointID currentJoint,
		searchResult& result,
		const glm::vec3 parentWorldPosition,
		const glm::quat parentWorldRotation,
		const glm::vec3 parentWorldScale);
};

#endif
