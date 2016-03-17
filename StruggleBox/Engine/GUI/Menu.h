#ifndef MENU_H
#define MENU_H

#include "Widget.h"
#include <map>

class Locator;
class Label;
class Button;

class Menu : public Widget
{
public:
    Menu(Locator& locator);
    ~Menu();
    
    void Draw(Renderer* renderer);
    void Update(const double deltaTime);
    void OnDrag(const glm::ivec2& coord);
    
    void OnInteract(const bool interact,
                    const glm::ivec2& coord);
    void setVisibility(const bool visible);

    void addWidget(std::shared_ptr<Widget> widget,
                   const std::string& subMenu = "");
    void createSubMenu(const std::string& name);
    
    void minimize();

    void setMinimizeable(const bool minimizeable);
    void setName(const std::string text);
    const std::string getName();
    const int getContentHeight() { return _contentHeight; }
    
private:
    Menu* _parent;
    std::vector<std::shared_ptr<Widget>> _widgets;
    std::map<std::string, std::shared_ptr<Menu>> _subMenus;
    std::shared_ptr<Label> _label;
    std::shared_ptr<Button> _minimizeButton;
    
    int _contentHeight;
    bool _minimizeable;
    bool _minimized;
    
    bool _dragging;
    bool _draggable;
    
    glm::ivec2 _dragPosition;
    
    void refresh();
};

#endif /* MENU_H */
