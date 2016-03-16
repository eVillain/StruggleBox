#include "FileMenu.h"
#include "Locator.h"
#include "Text.h"

FileMenu::FileMenu(Locator& locator) :
Widget(locator),
_label(nullptr)
{
    
}

FileMenu::~FileMenu()
{
    
}

void FileMenu::Draw(Renderer* renderer)
{
    Widget::Draw(renderer);
    
}

void Update(const double deltaTime)
{
    
}

void FileMenu::setFocus(const bool focus)
{
    _focus = focus;
}

void FileMenu::setActive(const bool active)
{
    _active = active;
}

void FileMenu::setVisibility(const bool visible)
{
    _visible = visible;
}

void FileMenu::OnInteract(const bool interact,
                          const glm::ivec2& coord)
{
    
}


void FileMenu::setName(const std::string text)
{
    if (_label) {
        _label->setText(text);
    } else {
        _label = _locator.Get<Text>()->CreateLabel(text);
    }
    printf("[FileMenu] set label %s use count %lu \n",
           _label->getText().c_str(), _label.use_count());
}

