#pragma once

#include "WindowNode.h"
#include <functional>
#include <string>

class ButtonNode;
class LabelNode;
class ScrollerNode;

class FileWindow : public WindowNode
{
public:
	enum Mode
	{
		Load,
		Save
	};

	FileWindow(
		const GUI& gui,
		const std::string& path,
		const std::string& fileType,
		const FileWindow::Mode& mode);
	~FileWindow();
	
	void setCallback(const std::function<void(const std::string&)>& callback) { m_callback = callback; }
	void setCloseCallback(const std::function<void()>& callback) { m_closeCallback = callback; }

	const std::string getPath() { return m_path; }
	const std::string getFile() { return m_fileName; }

private:
	static const glm::vec2 WINDOW_SIZE;
	static const float HEADER_HEIGHT;
	static const float FOOTER_HEIGHT;

	const std::string m_fileType;
	const FileWindow::Mode m_mode;
	std::string m_path;
	std::string m_fileName;
	std::function<void(const std::string&)> m_callback;
	std::function<void()> m_closeCallback;

	LabelNode* m_headerLabel;
	LabelNode* m_footerLabel;
	ScrollerNode* m_scrollerNode;
	ButtonNode* m_closeButton;
	ButtonNode* m_confirmButton;

	void refreshList();
	void refreshHeader();
	void refreshFooter();
	void onFolderSelected(const std::string& folderName);
	void onFileSelected(const std::string& fileName);

	void onCloseButton(bool);
	void onConfirmButton(bool);

	static const std::string getTitle(const FileWindow::Mode mode);
};
