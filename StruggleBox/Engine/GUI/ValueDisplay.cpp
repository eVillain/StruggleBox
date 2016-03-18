#include "ValueDisplay.h"
#include "Locator.h"
#include "Text.h"
#include "GUI.h"

ValueDisplay::ValueDisplay(Locator& locator) :
Widget(locator),
_value(nullptr),
_label(nullptr)
{
}

ValueDisplay::~ValueDisplay()
{
    if (_value)
    {
        delete _value;
        _value = nullptr;
    }
    
    if (_label)
    {
        _locator.Get<Text>()->DestroyLabel(_label);
        _label = nullptr;
    }
}

void ValueDisplay::Update(const double deltaTime)
{
    if (_value)
    {
        std::string valueText;
        if (_name.length()) {
            valueText += _name + ": ";
        }
        valueText += _value->GetValueString();

        if (!_label)
        {
            _label = _locator.Get<Text>()->CreateLabel(valueText);
        }
        else
        {
            _label->setText(valueText);
        }
    }
    if (_transform.isDirty())
    {
        _label->getTransform().SetPosition(_transform.GetPosition());
        _transform.unflagDirty();
    }
}

void ValueDisplay::setName(const std::string name)
{
    _name = name;
}


void ValueDisplay::setVisibility(const bool visible)
{
    _visible = visible;
    if (_label)
    {
        _label->setVisible(_visible);
    }
}
