#ifndef BUTTON_H
#define BUTTON_H

#include "Widget.h"
#include "ButtonBehavior.h"

class Button : public Widget
{
    friend class GUI;
public:
    ~Button();
    // Overrides from Widget
    virtual void Draw(Renderer* renderer);
    virtual const void Update();
    // When clicked/pressed
    virtual void OnInteract( const bool interact, const glm::ivec2& coord );
    
    // Attach a behavior to make the button do something when pressed
    void SetBehavior(ButtonBehavior* behavior ) { _behavior = behavior; };
    
protected:
    Button();
private:
    bool _pressed;
    ButtonBehavior* _behavior;
};

#endif /* BUTTON_H */
