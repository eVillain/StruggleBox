#include "AnimationEditor.h"

#include "Allocator.h"
#include "SkeletonWindow.h"
#include "VoxelRenderer.h"
#include "PathUtil.h"
#include "Camera3D.h"
#include "ButtonNode.h"

AnimationEditor::AnimationEditor(
	Allocator& allocator,
	VoxelRenderer& renderer,
	RenderCore& renderCore,
	OSWindow& osWindow,
	Options& options,
	Input& input,
	StatTracker& statTracker)
	: EditorScene(allocator, renderer, renderCore, osWindow, options, input, statTracker)
	, m_skeletonWindow(nullptr)
	, m_voxels(renderer, allocator)
	//, m_mesh(nullptr)
{
	//m_mesh = CUSTOM_NEW(InstancedTriangleMesh, m_allocator)(VertexDataType::InstancedTextureMeshVerts, m_renderer, m_allocator);
	//_instanceID = _mesh->addInstance(glm::vec3(0, 2, 0));

	m_renderer.getDefaultCamera().setThirdPerson(true);

	m_room.buildRoom(32.f, 32);
	//m_renderer.getPlugin3D().setRoomSize(32.0f);
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

	m_voxels.draw();

	//_cursor.posWorld = _renderer.GetCursor3DPos(_cursor.posScrn);
	auto data = m_skeleton.getInstanceData(m_skeletonWindow->getCurrentAnimation(), 0.0f);
	drawBones(data);
}

void AnimationEditor::onFileLoad(const std::string& file)
{
	if (isLoadingMesh())
	{
		m_skeletonWindow->getLoadMeshButton()->setToggled(false);
	}
}

void AnimationEditor::onFileSave(const std::string& file)
{

}

void AnimationEditor::ShowEditor()
{
	if (!m_skeletonWindow)
	{
		m_skeletonWindow = m_gui.createCustomNode<SkeletonWindow>(m_gui, m_skeleton);
		//m_skeletonWindow->setPositionX(m_renderer.getWindowSize().x - m_skeletonWindow->getContentSize().x);
		m_gui.getRoot().addChild(m_skeletonWindow);

		m_skeletonWindow->getLoadMeshButton()->setCallback([this](bool state) {
			if (state)
			{
				const std::string path = FileUtil::GetPath() + "Data/Objects/";
				showFileWindow(path, "*.bwo", true);
			}
		});
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

void AnimationEditor::drawBones(const std::vector<InstanceTransformData3D>& data)
{
	for (size_t i=0; i < data.size(); i++)
	{
		const InstanceTransformData3D& joint = data[i];
		//CubeInstance boneCube = {
		//	joint.position.x, joint.position.y, joint.position.z, 0.1,
		//	joint.rotation.x, joint.rotation.y, joint.rotation.z, joint.rotation.w,
		//	MaterialData::texOffsetX(50), MaterialData::texOffsetY(50),
		//};
		//m_renderer.getPlugin3D().bufferCubes(&boneCube, 1);

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

bool AnimationEditor::isLoadingMesh() const
{
	return m_skeletonWindow->getLoadMeshButton()->isToggled();
}
