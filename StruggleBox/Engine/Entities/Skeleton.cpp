#include "Skeleton.h"
#include "StringUtil.h"

Skeleton::Skeleton()
{
	addJoint();
	addAnimation("default");
}

JointID Skeleton::addJoint(const JointID parentID)
{
	if (parentID >= _joints.size() &&
		parentID != ROOT_JOINT_ID)
		return ROOT_JOINT_ID;
	std::string newName = "Joint" + StringUtil::LongToString(_joints.size());
	_joints.push_back({newName, parentID});
	JointID newID = (JointID)(_joints.size() - 1);
	if (parentID != newID) {
		_children[parentID].push_back(newID);
	}
	for (auto& anim : _animations)
	{
		KeyFrame frame = {
			1.0,
			InstanceData()
		};
		std::vector<KeyFrame> frames;
		frames.push_back(frame);
		anim.second[newID] = frames;
	}
	return newID;
}

JointID Skeleton::getNearestJoint(
	const std::string& animation,
	const glm::vec3& position,
	const float time)
{
	searchResult result = {
		ROOT_JOINT_ID, 2.0f
	};
	getNearestJoint(animation, position, time, ROOT_JOINT_ID, result, glm::vec3(), glm::quat(), glm::vec3(1));
	return result.current;
}

std::vector<InstanceTransformData> Skeleton::getInstanceData(
	const std::string& animation,
	const float time,
	const glm::vec3& position,
	const glm::quat& rotation,
	const glm::vec3& scale)
{
	std::vector<InstanceTransformData> instances(_joints.size());
	getInstanceData(instances, animation, time, ROOT_JOINT_ID, position, rotation, scale);
	return instances;
}

void Skeleton::addAnimation(const std::string animation)
{
	if (_animations.find(animation) != _animations.end())
		return;
	for (size_t i = 0; i < _joints.size(); i++)
	{
		KeyFrame frame = {
			1.0,
			InstanceData()
		};
		std::vector<KeyFrame> frames;
		frames.push_back(frame);
		_animations[animation][i] = frames;
	}
}

KeyFrame & Skeleton::getKeyFrame(
	const JointID joint,
	const std::string & animation,
	const uint16_t frame)
{
	return _animations[animation][joint][frame];
}

void Skeleton::getInstanceData(
	std::vector<InstanceTransformData> instances,
	const std::string & animation,
	const float time,
	JointID currentJoint,
	const glm::vec3 parentWorldPosition,
	const glm::quat parentWorldRotation,
	const glm::vec3 parentWorldScale)
{
	Joint& joint = _joints[currentJoint];
	SkeletalAnimation& skeletalAnimation = _animations[animation];
	AnimationStream& stream = skeletalAnimation[currentJoint];

	const InstanceTransformData currentFrame = getFrameAtTime(stream, time);

	glm::vec3 pos = currentFrame.position;
	glm::quat rot = currentFrame.rotation;
	glm::vec3 scale = currentFrame.scale;

	if (joint.parent != currentJoint)
	{
		if (joint.inheritRotation)
		{
			rot = parentWorldRotation * rot;
			pos = parentWorldPosition + (parentWorldRotation * pos);
		}
		else
		{
			pos = parentWorldPosition + (pos);
		}
		if (joint.inheritScale)
		{
			scale = parentWorldScale * scale;
		}
	}
	instances[currentJoint] = { pos, rot, scale };

	for (size_t i = 0; i < _children[currentJoint].size(); i++)
	{
		getInstanceData(instances, animation, time, _children[currentJoint][i], pos, rot, scale);
	}
}

InstanceTransformData Skeleton::getFrameAtTime(
	AnimationStream & stream,
	const float time)
{
	if (stream.size() == 1)
	{
		return stream[0].data;
	}
	float timeCount = 0.0f;
	for (size_t i = 0; i < stream.size(); i++)
	{
		const KeyFrame& frame = stream[i];
		if (timeCount + frame.time > time)
		{
			// time we seek lies between this frame and the next
			size_t nextFrame;
			if (i == stream.size() - 1)
				nextFrame = 0;
			else
				nextFrame = i + 1;

			const InstanceTransformData& prev = frame.data;
			const InstanceTransformData& next = stream[nextFrame].data;

			const float frameTime = time - timeCount;
			const float timeRatio = frameTime / frame.time;

			glm::vec3 pos = glm::mix(prev.position, next.position, timeRatio);
			glm::quat rot = glm::lerp(prev.rotation, next.rotation, timeRatio);
			glm::vec3 scale = glm::mix(prev.scale, next.scale, timeRatio);

			return {pos, rot, scale};
		}
		timeCount += frame.time;
	}
	return { glm::vec3(), glm::quat(), glm::vec3(1) };
}

void Skeleton::getNearestJoint(
	const std::string& animation,
	const glm::vec3& position,
	const float time,
	JointID currentJoint,
	searchResult& result,
	const glm::vec3 parentWorldPosition,
	const glm::quat parentWorldRotation,
	const glm::vec3 parentWorldScale)
{
	Joint& joint = _joints[currentJoint];
	SkeletalAnimation& skeletalAnimation = _animations[animation];
	AnimationStream& stream = skeletalAnimation[currentJoint];

	const InstanceTransformData currentFrame = getFrameAtTime(stream, time);

	glm::vec3 pos = currentFrame.position;
	glm::quat rot = currentFrame.rotation;
	glm::vec3 scale = currentFrame.scale;

	if (joint.parent != currentJoint)
	{
		if (joint.inheritRotation)
		{
			rot = parentWorldRotation * rot;
			pos = parentWorldPosition + (parentWorldRotation * pos);
		}
		else
		{
			pos = parentWorldPosition + (pos);
		}
		if (joint.inheritScale)
		{
			scale = parentWorldScale * scale;
		}
	}

	float distance = glm::distance(pos, position);
	if (distance < result.distance)
	{
		result.current = currentJoint;
		result.distance = distance;
	}

	for (size_t i = 0; i < _children[currentJoint].size(); i++)
	{
		getNearestJoint(animation, position, time, _children[currentJoint][i], result, pos, rot, scale);
	}
}
