#ifndef BUTTON_H
#define BUTTON_H

#include "Widget.h"
#include "ButtonBehavior.h"
#include <memory>

class Locator;
class Label;

class Button : public Widget
{
    friend class GUI;
public:
    Button(Locator& locator);
    ~Button();
    
    virtual void Draw(Renderer* renderer);
    virtual void OnDrag(const glm::ivec2& coord);

    virtual void OnInteract(const bool interact,
                            const glm::ivec2& coord);
    
    /// Attaches a behavior to make the button do something when pressed
    void SetBehavior(ButtonBehavior* behavior) { _behavior = behavior; }

    void setLabel(const std::string text);
    
    std::shared_ptr<Label> getLabel() { return _label; }
protected:
    Button();
    bool _pressed;

private:
    ButtonBehavior* _behavior;
    std::shared_ptr<Label> _label;
};

#endif /* BUTTON_H */
