#ifndef SLIDER_H
#define SLIDER_H

#include "Widget.h"
#include "SliderBehavior.h"

class Locator;

class Slider : public Widget
{
public:
    // Overrides from Widget
    virtual void SetPosition(const glm::ivec2& position);
    virtual void SetVisible(const bool visible);
    virtual void Draw(Renderer* renderer);
    virtual const void Update();
    // When clicked/pressed
    virtual void OnInteract( const bool interact, const glm::ivec2& coord );
    virtual void OnDrag( const glm::ivec2& coord );
    // Attach a behavior to make the button do something when pressed
    void SetBehavior( ISliderBehavior* behavior ) { _behavior = behavior; };
    
protected:
    Slider(Locator& locator);
    ~Slider();
    
private:
    ISliderBehavior* _behavior;
    double _sliderValue;    // Unit value between 0.0 and 1.0
    int _sliderWidth;
    int _sliderPadding;
    bool _draggingSlider;
    std::string _name;
    //        std::unique_ptr<Text::Label> _label;
    
    void CheckSliderPress(const glm::ivec2& coord);
};

#endif /* SLIDER_H */
