#ifndef FILE_WINDOW_H
#define FILE_WINDOW_H

#include "Window.h"
#include "GenericCallback.h"
#include <string>

enum FileWindowMode
{
	Mode_Load,
	Mode_Save
};
class FileWindow : public Window
{
public:
	FileWindow(
		tb::TBWidget* root,
		const std::string& path,
		const std::string& fileType = "",
		FileWindowMode mode = Mode_Load);
	~FileWindow();

	bool OnEvent(const tb::TBWidgetEvent & ev);
	
	void SetCallback(GenericCallback<std::string> * callback);

	const std::string GetPath() { return _path; }
	const std::string GetFile() { return _fileName; }

private:
	std::string _path;
	const std::string _fileType;
	const FileWindowMode _mode;
	std::string _fileName;
	GenericCallback<std::string> *_callback;

	void refreshList();
	void refreshHeader();
	void refreshFooter();
};

#endif // !FILE_WINDOW_H