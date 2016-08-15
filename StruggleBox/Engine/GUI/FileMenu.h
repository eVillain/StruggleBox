#ifndef FILE_MENU_H
#define FILE_MENU_H

#include "tb_menu_window.h"

class FileMenu : public tb::TBMenuWindow
{
public:
	FileMenu(tb::TBWidget* root);
	~FileMenu();

private:
	tb::TBGenericStringItemSource _file_menu_source;
};

#endif // !FILE_MENU_H

