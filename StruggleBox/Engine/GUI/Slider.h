//
//  Slider.h
//  DungeonSmith
//
//  Created by The Drudgerist on 29/08/15.
//  Copyright (c) 2015 The Drudgerist. All rights reserved.
//

#ifndef __DungeonSmith__Slider__
#define __DungeonSmith__Slider__

#include "Widget.h"
#include "SliderBehavior.h"
//#include "Label.h"

namespace GUI
{
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
        Slider();
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
}   /* namespace GUI */

#endif /* defined(__DungeonSmith__Slider__) */
