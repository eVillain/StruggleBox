#include "Menu.h"
#include "Locator.h"
#include "Text.h"
#include "GUI.h"
#include "Renderer.h"
#include "Button.h"

Menu::Menu(Locator& locator) :
Widget(locator),
_parent(nullptr),
_contentHeight(4),
_label(nullptr),
_minimizeButton(nullptr),
_draggable(true),
_dragging(false),
_minimizeable(true),
_minimized(false)
{
    setMinimizeable(true);
}

Menu::~Menu()
{
    GUI* gui = _locator.Get<GUI>();

    for (auto subMenu : _subMenus)
    {
        gui->DestroyWidget(subMenu.second);
//        subMenu.second = nullptr;
    }
    _subMenus.clear();
    
    for (auto widget : _widgets)
    {
        gui->DestroyWidget(widget);
        printf("Menu removed widget, use count %lu \n", widget.use_count());
//        widget = nullptr;
    }
    _widgets.clear();
    
    if (_label)
    {
        _locator.Get<Text>()->DestroyLabel(_label);
        _label = nullptr;
    }
    
    if (_minimizeButton)
    {
        _locator.Get<GUI>()->DestroyWidget(_minimizeButton);
        _minimizeButton = nullptr;
    }
}

void Menu::Draw(Renderer* renderer)
{
    if ( !_visible ) return;
    
    Widget::Draw(renderer);
    
    glm::ivec2 drawPos = glm::ivec2(_transform.GetPosition().x-(_size.x*0.5),
                                    _transform.GetPosition().y-(_size.y*0.5)-(_contentHeight));
    float z = _transform.GetPosition().z;

    // Box for content under menu bar
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(drawPos.x, drawPos.y+1),
                           glm::vec2(drawPos.x, drawPos.y+_contentHeight-1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           z);   // Left
    renderer->Buffer2DLine(glm::vec2(drawPos.x+_size.x-1, drawPos.y+_contentHeight-1),
                           glm::vec2(drawPos.x+_size.x-1, drawPos.y+1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           z);   // Right
    renderer->Buffer2DLine(glm::vec2(drawPos.x, drawPos.y),
                           glm::vec2(drawPos.x+_size.x-2, drawPos.y),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           z);   // Top
    renderer->Buffer2DLine(glm::vec2(drawPos.x, drawPos.y+_contentHeight-1),
                           glm::vec2(drawPos.x+_size.x-2, drawPos.y+_contentHeight-1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           z);   // Bottom
    
    renderer->DrawGradientY(Rect2D(drawPos.x, drawPos.y+1, _size.x-1, _contentHeight-1),
                            COLOR_UI_GRADIENT_TOP,
                            COLOR_UI_GRADIENT_BOTTOM,
                            z);
    // Inside border
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderer->Draw2DRect(Rect2D(drawPos.x+1, drawPos.y+1, _size.x-2, _contentHeight-2),
                         COLOR_UI_BORDER_INNER,
                         COLOR_NONE,
                         z);
    renderer->Render2DLines();
    
    if (_label)
    {
        if (_focus)
        {
            _label->setColor(COLOR_UI_TEXT_HIGHLIGHT);
        } else {
            
            _label->setColor(COLOR_UI_TEXT);
        }
    }
}

void Menu::Update(const double deltaTime)
{
    if (_transform.isDirty())
    {
        refresh();
        _transform.unflagDirty();
    }
}

void Menu::OnDrag(const glm::ivec2& coord)
{
    if (!_draggable) return;
    
    glm::ivec2 move = coord - _dragPosition;
    glm::vec3 origPos = _transform.GetPosition();
    _transform.SetPositionX(origPos.x + move.x);
    _transform.SetPositionY(origPos.y + move.y);
    _dragPosition = coord;
    refresh();
}

void Menu::OnInteract(const bool interact,
                      const glm::ivec2& coord)
{
    if (!_draggable) return;
    _dragging = interact;
    if (_dragging) { _dragPosition = coord; }
    else { _dragPosition = glm::ivec2(); }
}

void Menu::addWidget(std::shared_ptr<Widget> widget,
                     const std::string& subMenuName)
{
    if (subMenuName.length() == 0) {
        _widgets.push_back(widget);
        widget->setVisibility(!_minimized);
        if (_parent) _parent->refresh();
        else refresh();
    } else {
        if (_subMenus.find(subMenuName) == _subMenus.end()) {
            createSubMenu(subMenuName);
        }
        _subMenus[subMenuName]->addWidget(widget);
    }
}

void Menu::createSubMenu(const std::string& name)
{
    auto subMenu = _locator.Get<GUI>()->CreateWidget<Menu>();
    subMenu->setName(name);
    subMenu->minimize();
    _subMenus[name] = subMenu;
    subMenu->_parent = this;
    subMenu->_draggable = false;
    if (_parent) _parent->refresh();
    else refresh();
}

void Menu::refresh()
{
    if (_minimizeButton)
    {
        glm::vec3 minimizeBtnPos = glm::vec3(_transform.GetPosition());
        minimizeBtnPos.x += (_size.x/2) - 12;
        minimizeBtnPos.z += 1;
        _minimizeButton->GetTransform().SetPosition(minimizeBtnPos);
        _minimizeButton->setSize(glm::ivec2(_size.y-6));
    }
    if (_label)
    {
        _label->getTransform().SetPosition(_transform.GetPosition());
        _label->setFontSize(_size.y - 8);
    }
    glm::vec3 widgetPos = _transform.GetPosition();
    widgetPos.y -= 2;
    widgetPos.z += 1;
    _contentHeight = 4;
    
    for (auto subMenu : _subMenus)
    {
        subMenu.second->setSize(glm::ivec2(_size.x-4, _size.y-2));
        widgetPos.y -= subMenu.second->getSize().y;
        subMenu.second->GetTransform().SetPosition(widgetPos);
        subMenu.second->refresh();
        widgetPos.y -= subMenu.second->getContentHeight();
        if (!_minimized) _contentHeight += subMenu.second->getSize().y + subMenu.second->getContentHeight();
    }
    
    for (auto widget : _widgets)
    {
        widget->setSize(glm::ivec2(_size.x-4, _size.y-2));
        widgetPos.y -= widget->getSize().y;
        widget->GetTransform().SetPosition(widgetPos);
        if (!_minimized) _contentHeight += widget->getSize().y;
    }
}

void Menu::minimize()
{
    if (!_minimizeable) return;
    
    _minimized = !_minimized;
    
    for (auto subMenu : _subMenus)
    {
        subMenu.second->setVisibility(!_minimized);
    }
    
    for (auto widget : _widgets)
    {
        widget->setVisibility(!_minimized);
    }
    
    if (_minimized)
    {
        _minimizeButton->setLabel("v");
    } else {
        _minimizeButton->setLabel("^");
    }
    refresh();
    if (_parent) _parent->refresh();
}

void Menu::setMinimizeable(const bool minimizeable)
{
    _minimizeable = minimizeable;
    if (_minimizeable &&
        _minimizeButton == nullptr)
    {
        _minimizeButton = _locator.Get<GUI>()->CreateWidget<Button>();
        _minimizeButton->setLabel("^");
        _minimizeButton->SetBehavior(new ButtonBehaviorLambda([=](){
            minimize();
        }));
    } else if (!_minimizeable &&
               _minimizeButton != nullptr)
    {
        _locator.Get<GUI>()->DestroyWidget(_minimizeButton);
        _minimizeButton = nullptr;
    }
}

void Menu::setVisibility(const bool visible)
{
    _visible = visible;
    
    _label->setVisible(_visible);
    
    if (_minimizeButton)
    {
        _minimizeButton->setVisibility(_visible);
    }
    
    bool contentVisibility = _visible && !_minimized;
    
    for (auto subMenu : _subMenus)
    {
        subMenu.second->setVisibility(contentVisibility);
    }
    
    for (auto widget : _widgets)
    {
        widget->setVisibility(contentVisibility);
    }
}

void Menu::setName(const std::string text)
{
    if (_label) {
        _label->setText(text);
    } else {
        _label = _locator.Get<Text>()->CreateLabel(text);
    }
}

const std::string Menu::getName()
{
    return _label->getText();
}
