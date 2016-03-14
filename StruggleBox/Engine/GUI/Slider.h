#ifndef SLIDER_H
#define SLIDER_H

#include "Widget.h"
#include "SliderBehavior.h"
#include <memory>

class Locator;
class Label;

class Slider : public Widget
{
    friend class GUI;
public:
    ~Slider();

    // Overrides from Widget
    virtual void Draw(Renderer* renderer);
    // When clicked/pressed
    virtual void OnInteract(const bool interact,
                            const glm::ivec2& coord);
    virtual void OnDrag(const glm::ivec2& coord);
    
    virtual void setVisibility(const bool visible);
    // Attach a behavior to make the button do something when pressed
    void SetBehavior( ISliderBehavior* behavior ) { _behavior = behavior; };
    void setLabel(const std::string text);

protected:
    Slider(Locator& locator);
    
private:
    ISliderBehavior* _behavior;
    double _sliderValue;    // Unit value between 0.0 and 1.0
    int _sliderWidth;
    int _sliderPadding;
    bool _draggingSlider;
    std::string _name;
    std::shared_ptr<Label> _label;
    
    void CheckSliderPress(const glm::ivec2& coord);
};

#endif /* SLIDER_H */
