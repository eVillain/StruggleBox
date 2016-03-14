#ifndef MENU_H
#define MENU_H

#include "Widget.h"

class Locator;
class Label;

class Menu : public Widget
{
public:
    Menu(Locator& locator);
    ~Menu();
    
    virtual void Draw(Renderer* renderer);
    virtual void OnDrag(const glm::ivec2& coord);
    
    virtual void OnInteract(const bool interact,
                            const glm::ivec2& coord);

    void addWidget(std::shared_ptr<Widget> widget,
                   const std::string& subMenu = "");
    void createSubMenu(const std::string& name);
    
    void setName(const std::string text);
    const std::string getName();
private:
    std::vector<std::shared_ptr<Widget>> _widgets;
    std::vector<std::shared_ptr<Menu>> _subMenus;
    
    std::shared_ptr<Label> _label;
    int _contentHeight;
    bool _dragging;
    glm::ivec2 _dragPosition;
    
    void refresh();
};

#endif /* MENU_H */
