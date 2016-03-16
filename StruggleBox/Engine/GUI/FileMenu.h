#ifndef FILE_MENU_H
#define FILE_MENU_H

#include "Widget.h"
#include <memory>

class Label;

class FileMenu : public Widget
{
    friend class GUI;
public:
    ~FileMenu();

    void Draw(Renderer* renderer);
    void Update(const double deltaTime);
    void setFocus(const bool focus);
    void setActive(const bool active);
    void setVisibility(const bool visible);

    void OnInteract(const bool interact,
                    const glm::ivec2& coord);
    
    void setName(const std::string text);
    std::shared_ptr<Label> getLabel() { return _label; }
protected:
    FileMenu(Locator& locator);
private:
    std::shared_ptr<Label> _label;
};

#endif /* FILE_MENU_H */
