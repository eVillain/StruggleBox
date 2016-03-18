#ifndef VALUE_DISAPLY_H
#define VALUE_DISAPLY_H

#include "Widget.h"
#include <memory>
#include <sstream>

class Label;

class IValueContainer
{
public:
    virtual ~IValueContainer() {};
    virtual const std::string GetValueString() = 0;
};

template <typename UnknownType>
class ValueContainer : public IValueContainer
{
public:
    ValueContainer(UnknownType& data) :
    _data(data)
    { }
    
    const std::string GetValueString()
    {
        std::ostringstream buff;
        buff.precision(3);
        buff<<_data;
        return buff.str();
    }
    
private:
    UnknownType& _data;
};


class ValueDisplay : public Widget
{
    friend class GUI;
public:
    ~ValueDisplay();
    
    void Update(const double deltaTime);
    
    void setVisibility(const bool visible);
    
    template <typename UnknownType>
    void setValue(UnknownType& value)
    {
        if (_value) { delete _value; }
        auto container = new ValueContainer<UnknownType>(value);
        _value = container;
    }
    
    void setName(const std::string name);
    std::shared_ptr<Label> getLabel() { return _label; }

protected:
    ValueDisplay(Locator& locator);
    
private:
    IValueContainer* _value;
    std::shared_ptr<Label> _label;
    std::string _name;
};

#endif /* VALUE_DISAPLY_H */
