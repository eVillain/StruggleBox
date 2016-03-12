#ifndef BUTTON_H
#define BUTTON_H

#include "Widget.h"
#include "ButtonBehavior.h"

class Button : public Widget
{
    friend class GUI;
public:
    ~Button();
    
    virtual void Draw(Renderer* renderer);
    virtual void OnDrag(const glm::ivec2& coord);

    virtual void OnInteract(const bool interact,
                            const glm::ivec2& coord);
    
    /// Attaches a behavior to make the button do something when pressed
    void SetBehavior(ButtonBehavior* behavior) { _behavior = behavior; };
    
protected:
    Button();
    bool _pressed;

private:
    ButtonBehavior* _behavior;
};

#endif /* BUTTON_H */
