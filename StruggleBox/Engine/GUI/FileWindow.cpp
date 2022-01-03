#include "FileWindow.h"
 
#include "GUI.h"
#include "PathUtil.h"
#include "FileUtil.h"
#include "Log.h"
#include "LabelNode.h"
#include "ScrollerNode.h"
#include "ButtonNode.h"

const glm::vec2 FileWindow::WINDOW_SIZE = glm::vec2(400.f, 600.f);
const float FileWindow::HEADER_HEIGHT = 20.f;
const float FileWindow::FOOTER_HEIGHT = 60.f;

FileWindow::FileWindow(
	const GUI& gui,
	const std::string& path,
	const std::string& fileType,
	const FileWindow::Mode& mode)
	: WindowNode(gui, WINDOW_SIZE, GUI::WINDOW_HEADER, GUI::WINDOW_CONTENT, getTitle(mode))
	, m_path(path)
	, m_fileType(fileType)
	, m_mode(mode)
	, m_callback(nullptr)
	, m_headerLabel(gui.createLabelNode("", GUI::FONT_DEFAULT, 12))
	, m_footerLabel(gui.createLabelNode("", GUI::FONT_DEFAULT, 12))
	, m_scrollerNode(gui.createScrollerNode(GUI::RECT_DEFAULT))
	, m_closeButton(gui.createRectButton(glm::vec2(24.f, 24.f), "X"))
	, m_confirmButton(gui.createRectButton(glm::vec2(100.f, 24.f), "Confirm"))
{
	m_headerLabel->setAnchorPoint(glm::vec2(0.f, 0.5f));
	m_footerLabel->setAnchorPoint(glm::vec2(0.f, 0.5f));
	m_closeButton->setAnchorPoint(glm::vec2(1.f, 0.0f));
	m_confirmButton->setAnchorPoint(glm::vec2(1.f, 0.0f));
	m_closeButton->setCallback(std::bind(&FileWindow::onCloseButton, this, std::placeholders::_1));
	m_confirmButton->setCallback(std::bind(&FileWindow::onConfirmButton, this, std::placeholders::_1));

	addChild(m_headerLabel);
	addChild(m_footerLabel);
	addChild(m_scrollerNode);
	addChild(m_closeButton);
	addChild(m_confirmButton);

	if (!FileUtil::DoesFolderExist(m_path))
	{
		m_path = FileUtil::GetPath();
	}
	refreshHeader();
	refreshFooter();
	refreshList();
}

FileWindow::~FileWindow()
{
}

//bool FileWindow::OnEvent(const tb::TBWidgetEvent & ev)
//{
//	if (ev.type == tb::EVENT_TYPE_CLICK)
//	{
//		if (ev.target->GetID() == TBIDC("button-folder-select"))
//		{
//			std::string fileName = ev.target->GetText();
//			if (fileName == "..") {
//				std::string folderUp = FileUtil::GetContainingFolder(_path);
//				if (folderUp.length() != 0 &&
//					folderUp.length() < _path.length())
//				{
//					_path = folderUp;
//					refreshList();
//				}
//			}
//			else
//			{
//			_path = _path + ev.target->GetText().CStr() + "/";
//			refreshHeader();
//			refreshList();
//			}
//			return true;
//		}
//		else if (ev.target->GetID() == TBIDC("button-file-select"))
//		{
//			_fileName = ev.target->GetText();
//			refreshFooter();
//			return true;
//		}
//		else if (ev.target->GetID() == TBIDC("button-ok"))
//		{
//			if (_fileName.length() == 0) return true;
//			if (_mode == Mode_Load && !FileUtil::DoesFileExist(_path, _fileName)) return true;
//			if (_callback) _callback->Trigger(_path + _fileName);
//			Close();
//			return true;
//		}
//		else if (ev.target->GetID() == TBIDC("button-cancel"))
//		{
//			if (_callback) _callback->Trigger("");
//			Close();
//			return true;
//		}
//	}
//	else if (ev.target->GetID() == TBIDC("file-name") &&
//			ev.type == tb::EVENT_TYPE_KEY_DOWN &&
//			ev.special_key == tb::TB_KEY_ENTER)
//	{
//		tb::TBEditField* footer = GetWidgetByIDAndType<tb::TBEditField>(TBIDC("file-name"));
//		_fileName = footer->GetText();
//		if (_callback) _callback->Trigger(_path + _fileName);
//		return true;
//	}
//	return Window::OnEvent(ev);
//}
//

void FileWindow::refreshList()
{
	const glm::vec2 FILE_NODE_SIZE = glm::vec2(m_contentNode->getContentSize().x - 2, 20.f);
	m_scrollerNode->setContentSize(m_contentNode->getContentSize() - glm::vec2(4.f, HEADER_HEIGHT + FOOTER_HEIGHT));
	m_scrollerNode->setPosition(glm::vec3(2.f, FOOTER_HEIGHT, 1.f));
	std::vector<Node*> nodes;

	std::vector<std::string> folders;
	FileUtil::GetSubfolders(m_path, folders);
	for (const std::string& folderName : folders)
	{
		if (folderName == ".") continue;

		ButtonNode* fileButton = m_gui.createRectButton(FILE_NODE_SIZE, folderName);
		LabelNode* buttonLabel = dynamic_cast<LabelNode*>(fileButton->getChildren().at(0));
		if (buttonLabel)
		{
			buttonLabel->setAnchorPoint(glm::vec2(0.f, 0.5f));
		}
		fileButton->setCallback([this, folderName](bool) {
			onFolderSelected(folderName);
			});
		nodes.push_back(fileButton);
	}

	std::vector<std::string> files;
	if (m_fileType.length() > 0)
	{
		FileUtil::GetFilesOfType(m_path, m_fileType, files);
	}
	else
	{
		FileUtil::GetAllFiles(m_path, files);
	}

	for (const std::string& fileName : files)
	{
		ButtonNode* fileButton = m_gui.createRectButton(FILE_NODE_SIZE, fileName);
		LabelNode* buttonLabel = dynamic_cast<LabelNode*>(fileButton->getChildren().at(0));
		if (buttonLabel)
		{
			buttonLabel->setAnchorPoint(glm::vec2(0.f, 0.5f));
		}
		fileButton->setCallback([this, fileName](bool) {
			onFileSelected(fileName);
			});
		nodes.push_back(fileButton);
	}

	m_scrollerNode->setContent(nodes, ScrollStrategy::KEEP_OFFSET);
}

void FileWindow::refreshHeader()
{
	if (!m_headerLabel)
	{
		return;
	}
	m_headerLabel->setText(m_path);
	m_headerLabel->setPosition(glm::vec3(2.f, m_contentNode->getContentSize().y - (HEADER_HEIGHT * 0.5f), 1.f));
}

void FileWindow::refreshFooter()
{
	if (!m_footerLabel)
	{
		return;
	}

	if (m_fileName.empty())
	{ 
		m_footerLabel->setText("*." + m_fileType);
	}
	else
	{
		m_footerLabel->setText(m_fileName);
	}
	m_footerLabel->setPosition(glm::vec3(2.f, 30.f, 1.f));
	m_closeButton->setPosition(glm::vec3(m_contentSize.x - 2.f, 2.f, 1.f));
	m_confirmButton->setPosition(glm::vec3(m_contentSize.x - 34.f, 2.f, 1.f));
}

void FileWindow::onFolderSelected(const std::string& folderName)
{
	if (folderName == "..") {
		std::string folderUp = FileUtil::GetContainingFolder(m_path);
		if (folderUp.length() != 0 &&
			folderUp.length() < m_path.length())
		{
			m_path = folderUp;
			refreshList();
		}
	}
	else
	{
		m_path = m_path + folderName + "/";
		refreshHeader();
		refreshList();
	}
}

void FileWindow::onFileSelected(const std::string& fileName)
{
	m_fileName = fileName;
	refreshFooter();
}

void FileWindow::onCloseButton(bool)
{
	if (m_closeCallback)
	{
		m_closeCallback();
	}

	m_parent->removeChild(this);
	m_gui.destroyNodeAndChildren(this);
}

void FileWindow::onConfirmButton(bool)
{
	if (m_callback)
	{
		m_callback(m_fileName);
	}
}

const std::string FileWindow::getTitle(const FileWindow::Mode mode)
{
	if (mode == FileWindow::Mode::Load)
	{
		return "Load File";
	}
	return "Save File";
}
