#include "FileMenu.h"

FileMenu::FileMenu(tb::TBWidget * root) :
	tb::TBMenuWindow(root, TBIDC("file-menu"))
{
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Load", TBIDC("open-load")));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("-"));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Save", TBIDC("open-save")));
	Show(&_file_menu_source, tb::TBPopupAlignment());
}

FileMenu::~FileMenu()
{
}
