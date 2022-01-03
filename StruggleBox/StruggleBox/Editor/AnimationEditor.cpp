#include "AnimationEditor.h"

#include "Allocator.h"
#include "SkeletonWindow.h"
#include "Renderer.h"
#include "PathUtil.h"
#include "Camera.h"

AnimationEditor::AnimationEditor(
	Camera& camera,
	Allocator& allocator,
	Renderer& renderer,
	Options& options,
	Input& input,
	StatTracker& statTracker) :
	EditorScene(camera, allocator, renderer, options, input, statTracker)
{
	m_camera.thirdPerson = true;

	m_room.buildRoom(32.f, 32);
	m_renderer.setRoomSize(32.0f);
}

AnimationEditor::~AnimationEditor()
{
}

void AnimationEditor::Initialize()
{
	EditorScene::Initialize();
	ShowEditor();
}

void AnimationEditor::ReInitialize()
{
	EditorScene::ReInitialize();
}

void AnimationEditor::Pause()
{
	EditorScene::Pause();
	RemoveEditor();
}

void AnimationEditor::Resume()
{
	EditorScene::Resume();
	ShowEditor();
}

void AnimationEditor::Update(const double delta)
{
	EditorScene::Update(delta);
}

void AnimationEditor::Draw()
{
	EditorScene::Draw();

	//_cursor.posWorld = _renderer.GetCursor3DPos(_cursor.posScrn);
	//auto data = _skeleton.getInstanceData(_skeletonWindow->getCurrentAnimation(), 0.0f);
	//drawBones(data);
}

void AnimationEditor::ShowEditor()
{
	if (!m_skeletonWindow)
	{
		m_skeletonWindow = m_gui.createCustomNode<SkeletonWindow>(m_gui, m_skeleton);
		m_skeletonWindow->setPositionX(m_renderer.getWindowSize().x - m_skeletonWindow->getContentSize().x);
		m_gui.getRoot().addChild(m_skeletonWindow);
	}
}

void AnimationEditor::RemoveEditor()
{
	if (m_skeletonWindow)
	{
		m_gui.destroyNodeAndChildren(m_skeletonWindow);
		m_skeletonWindow = nullptr;
	}
}

bool AnimationEditor::OnEvent(const InputEvent event, const float amount)
{
	if (EditorScene::OnEvent(event, amount))
	{
		return true;
	}
	//else if (amount == -1.0f)
	//{
	//	if (theEvent == INPUT_SHOOT) {
	//		JointID clicked = _skeleton.getNearestJoint(_skeletonWindow->getCurrentAnimation(), _cursor.posWorld, 0.0f);	// TODO: ADD TIME HERE
	//		if (clicked)
	//			_skeletonWindow->setCurrentJoint(clicked);
	//	}
	//}
	return false;
}

bool AnimationEditor::OnMouse(const glm::ivec2 & coord)
{
	if (EditorScene::OnMouse(coord))
		return true;
	return false;
}

void AnimationEditor::drawBones(const std::vector<InstanceTransformData>& data)
{
	for (size_t i=0; i < data.size(); i++)
	{
		const InstanceTransformData& joint = data[i];
		CubeInstance boneCube = {
			joint.position.x, joint.position.y, joint.position.z, 0.1,
			joint.rotation.x, joint.rotation.y, joint.rotation.z, joint.rotation.w,
			MaterialData::texOffsetX(50), MaterialData::texOffsetY(50),
		};
		m_renderer.bufferCubes(&boneCube, 1);

		//if (i == _skeletonWindow->getCurrentJoint())
		//{
		//	drawBoxOutline(joint.position, glm::vec3(0.13), COLOR_SELECT);
		//}
		//else
		//{
		//	Color outlineColor = _skeleton.getJoint(i).parent == i ? COLOR_YELLOW : COLOR_GREEN;
		//	drawBoxOutline(joint.position, glm::vec3(0.11), outlineColor);
		//}
		//if (_skeleton.getJoint(i).parent != i)
		//{
		//	const InstanceData& parent = data[_skeleton.getJoint(i).parent];

		//	_renderer.Buffer3DLine(parent.position, joint.position, COLOR_YELLOW, COLOR_GREEN);
		//}
	}
}
