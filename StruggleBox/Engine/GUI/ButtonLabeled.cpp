#include "ButtonLabeled.h"

ButtonLabeled::ButtonLabeled() :
_label(nullptr)
{ }

void ButtonLabeled::Draw(Renderer *renderer)
{
    Button::Draw(renderer);
    if (_label == nullptr) return;
    
    if (_focus)
    {
        _label->setColor(COLOR_UI_TEXT_HIGHLIGHT);
    } else {
        if (_pressed) {
            _label->setColor(COLOR_UI_TEXT_ACTIVE);
            _label->getTransform().SetPosition(_transform.GetPosition()+glm::vec3(-8,-8,0));
        } else {
            _label->setColor(COLOR_UI_TEXT);
        }
    }
    _label->getTransform().SetPosition(_transform.GetPosition());
}