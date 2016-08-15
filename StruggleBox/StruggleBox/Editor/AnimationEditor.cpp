#include "AnimationEditor.h"
#include "Renderer.h"
#include "OptionsWindow.h"
#include "PathUtil.h"
#include "TBGUI.h"
#include "Camera.h"
#include "tb_menu_window.h"

AnimationEditor::AnimationEditor(
	std::shared_ptr<TBGUI> gui,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<Options> options,
	std::shared_ptr<Input> input) :
	EditorScene(gui, camera, renderer, options, input)
{
	//_camera->thirdPerson = true;
	// Setup GUI
	_root.SetLayoutDistributionPosition(tb::LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP);
	_root.SetAxis(tb::AXIS_Y);
	_file_menu_source.AddItem(new tb::TBGenericStringItem("New Object", TBIDC("open-new")));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Load Object", TBIDC("open-load")));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Save Object", TBIDC("open-save")));

	_root.AddListener("file-button", [&](const tb::TBWidgetEvent& ev) {
		tb::TBButton *button = tb::TBSafeCast<tb::TBButton>(ev.target);
		tb::TBMenuWindow* filePopup = new tb::TBMenuWindow(button, TBIDC("file-menu"));
		filePopup->Show(&_file_menu_source, tb::TBPopupAlignment());
	});

	_root.AddListener("file-menu", [&](const tb::TBWidgetEvent& ev) {
		if (ev.ref_id == TBIDC("open-new"))
		{

		}
		else if (ev.ref_id == TBIDC("open-load"))
		{
			//FileWindow* window = new FileWindow(_gui->getRoot(), PathUtil::ObjectsPath(), "bwo", Mode_Load);
			//window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
			//	if (file.length() > 0)
			//		LoadObject(file);
			//}));
		}
		else if (ev.ref_id == TBIDC("open-save"))
		{
			//FileWindow* window = new FileWindow(_gui->getRoot(), PathUtil::ObjectsPath(), "bwo", Mode_Save);
			//window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
			//	if (file.length() > 0)
			//		SaveObject(file);
			//}));
		}
	});

	_root.AddListener("options-button", [&](const tb::TBWidgetEvent& ev) {
		new OptionsWindow(&_root, _options);
	});

	_room.buildRoom(128.0f, 32);
	_renderer->setRoomSize(128.0f);
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

void AnimationEditor::Release()
{
	EditorScene::Release();
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
	_cursor.posWorld = _renderer->GetCursor3DPos(_cursor.posScrn);
	EditorScene::Draw();
	auto data = _skeleton.getInstanceData(_skeletonWindow->getCurrentAnimation(), 0.0f);
	drawBones(data);
}

void AnimationEditor::ShowEditor()
{
	std::string path = PathUtil::GUIPath() + "ui_animationeditor.txt";
	_root.LoadResourceFile(path.c_str());

	_skeletonWindow = new SkeletonWindow(_gui->getRoot(), &_skeleton);
	_skeletonWindow->SetPosition(tb::TBPoint(_gui->getRoot()->GetRect().w - _skeletonWindow->GetRect().w, 0));
}

void AnimationEditor::RemoveEditor()
{
	_skeletonWindow->Die();
	_skeletonWindow = nullptr;
}

bool AnimationEditor::OnEvent(
	const std::string & theEvent,
	const float & amount)
{
	if (EditorScene::OnEvent(theEvent, amount))
		return true;
	else if (amount == -1.0f)
	{
		if (theEvent == INPUT_SHOOT) {
			JointID clicked = _skeleton.getNearestJoint(_skeletonWindow->getCurrentAnimation(), _cursor.posWorld, 0.0f);	// TODO: ADD TIME HERE
			if (clicked)
				_skeletonWindow->setCurrentJoint(clicked);
		}
	}
	return false;
}

bool AnimationEditor::OnMouse(const glm::ivec2 & coord)
{
	if (EditorScene::OnMouse(coord))
		return true;
	return false;
}

void AnimationEditor::drawBones(const std::vector<InstanceData>& data)
{
	for (size_t i=0; i < data.size(); i++)
	{
		const InstanceData& joint = data[i];
		CubeInstance boneCube = {
			joint.position.x, joint.position.y, joint.position.z, 0.1,
			joint.rotation.x, joint.rotation.y, joint.rotation.z, joint.rotation.w,
			MaterialData::texOffset(50)
		};
		_renderer->bufferCubes(&boneCube, 1);

		if (i == _skeletonWindow->getCurrentJoint())
		{
			_renderer->DrawBoxOutline(joint.position, glm::vec3(0.13), COLOR_SELECT);
		}
		else
		{
			Color outlineColor = _skeleton.getJoint(i).parent == i ? COLOR_YELLOW : COLOR_GREEN;
			_renderer->DrawBoxOutline(joint.position, glm::vec3(0.11), outlineColor);
		}
		if (_skeleton.getJoint(i).parent != i)
		{
			const InstanceData& parent = data[_skeleton.getJoint(i).parent];

			_renderer->Buffer3DLine(parent.position, joint.position, COLOR_YELLOW, COLOR_GREEN);
		}
	}
}
