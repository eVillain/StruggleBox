#include "FileWindow.h"
#include "PathUtil.h"
#include "FileUtil.h"
#include "Log.h"
#include "tb_editfield.h"

FileWindow::FileWindow(
	tb::TBWidget * root,
	const std::string & path,
	const std::string & fileType,
	FileWindowMode mode) :
	Window(root),
	_path(path),
	_fileType(fileType),
	_mode(mode),
	_callback(nullptr)
{
	std::string resourcePath = PathUtil::GUIPath() + "ui_filewindow.txt";
	LoadResourceFile(resourcePath.c_str());

	if (!FileUtil::DoesFolderExist(_path)) {
		_path = FileUtil::GetPath();
	}
	refreshHeader();
	refreshFooter();
	refreshList();
}

FileWindow::~FileWindow()
{
	if (_callback)
		delete _callback;
}

bool FileWindow::OnEvent(const tb::TBWidgetEvent & ev)
{
	if (ev.type == tb::EVENT_TYPE_CLICK)
	{
		if (ev.target->GetID() == TBIDC("button-folder-select"))
		{
			std::string fileName = ev.target->GetText();
			if (fileName == "..") {
				std::string folderUp = FileUtil::GetContainingFolder(_path);
				if (folderUp.length() != 0 &&
					folderUp.length() < _path.length())
				{
					_path = folderUp;
					refreshList();
				}
			}
			else
			{
			_path = _path + ev.target->GetText().CStr() + "/";
			refreshHeader();
			refreshList();
			}
			return true;
		}
		else if (ev.target->GetID() == TBIDC("button-file-select"))
		{
			_fileName = ev.target->GetText();
			refreshFooter();
			return true;
		}
		else if (ev.target->GetID() == TBIDC("button-ok"))
		{
			if (_fileName.length() == 0) return true;
			if (_mode == Mode_Load && !FileUtil::DoesFileExist(_path, _fileName)) return true;
			if (_callback) _callback->Trigger(_path + _fileName);
			Close();
			return true;
		}
		else if (ev.target->GetID() == TBIDC("button-cancel"))
		{
			if (_callback) _callback->Trigger("");
			Close();
			return true;
		}
	}
	else if (ev.target->GetID() == TBIDC("file-name") &&
			ev.type == tb::EVENT_TYPE_KEY_DOWN &&
			ev.special_key == tb::TB_KEY_ENTER)
	{
		tb::TBEditField* footer = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("file-name"));
		_fileName = footer->GetText();
		if (_callback) _callback->Trigger(_path + _fileName);
		return true;
	}
	return Window::OnEvent(ev);
}

void FileWindow::SetCallback(GenericCallback<std::string>* callback)
{
	if (_callback) delete _callback;
	_callback = callback;
}

void FileWindow::refreshList()
{
	tb::TBLayout* node = GetWidgetByIDAndType<tb::TBLayout>(TBIDC("file-list"));
	tb::TBWidget* child = node->GetFirstChild();
	while (child != nullptr)
	{
		node->RemoveChild(child);
		child = node->GetFirstChild();
	}

	std::vector<std::string> folders;
	FileUtil::GetSubfolders(_path, folders);
	for (std::string folderName : folders)
	{
		if (folderName == ".") continue;
		tb::TBButton* fileButton = new tb::TBButton();
		fileButton->SetSkinBg(TBIDC("TBButton.flat"));
		fileButton->SetText(folderName.c_str());
		fileButton->SetID(TBIDC("button-folder-select"));
		fileButton->SetGravity(tb::WIDGET_GRAVITY_LEFT_RIGHT);
		node->AddChild(fileButton);
	}

	std::vector<std::string> files;
	if (_fileType.length() > 0)
	{
		FileUtil::GetFilesOfType(_path, _fileType, files);
	}
	else
	{
		FileUtil::GetAllFiles(_path, files);
	}

	for (std::string fileName : files)
	{
		tb::TBButton* fileButton = new tb::TBButton();
		fileButton->SetSkinBg(TBIDC("TBButton.flat"));
		fileButton->SetText(fileName.c_str());
		fileButton->SetID(TBIDC("button-file-select"));
		fileButton->SetGravity(tb::WIDGET_GRAVITY_LEFT_RIGHT);
		node->AddChild(fileButton);
	}
}

void FileWindow::refreshHeader()
{
	tb::TBTextField* header = GetWidgetByIDAndType<tb::TBTextField>(TBIDC("file-path"));
	header->SetText(_path.c_str());
}

void FileWindow::refreshFooter()
{
	tb::TBEditField* footer = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("file-name"));
	if (_fileName.length() == 0)
		footer->SetPlaceholderText(("*." + _fileType).c_str());
	else
		footer->SetText(_fileName.c_str());
}
