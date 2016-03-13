#ifndef SLIDER_BEHAVIOR_H
#define SLIDER_BEHAVIOR_H

#include <sstream>

class ISliderBehavior
{
public:
    virtual ~ISliderBehavior() {};
    // Takes a unit value from 0.0 to 1.0, which correspond to min and max
    virtual void SetValue(const double value) = 0;
    virtual const double GetValue() = 0;
    virtual const std::string GetValueString() = 0;
};

template <typename UnknownType>
class SliderBehavior : public ISliderBehavior
{
public:
    SliderBehavior(UnknownType& data,
                   const UnknownType min = 0,
                   const UnknownType max = 1) :
    _data(&data),
    _min(min),
    _max(max)
    { }
    
    void SetValue(const double value)
    {
        if (_data)
        {
            UnknownType range = _max - _min;
            UnknownType newValue = _min + (range*value);
            *_data = newValue;
        }
    }
    
    const double GetValue()
    {
        UnknownType range = _max - _min;
        return double((*_data-_min)/range);
    }
    
    const std::string GetValueString()
    {
        if (!_data) return "";
        std::ostringstream buff;
        buff.precision(3);
        buff<<*_data;
        return buff.str();
    }
private:
    UnknownType* _data;                    // Pointer to the data
    const UnknownType _min;
    const UnknownType _max;
};

template <typename UnknownType, class UnknownClass>
class SliderBehaviorCallback : public ISliderBehavior
{
public:
    SliderBehaviorCallback(UnknownType& data,
                           UnknownClass* objectPtr,
                           void( UnknownClass::*func )( UnknownType& ),
                           const UnknownType min = 0,
                           const UnknownType max = 1) :
    _data(&data),
    _min(min),
    _max(max)
    { }
    
    void SetValue(const UnknownType& value)
    {
        if (_data)
        {
            UnknownType range = _max - _min;
            UnknownType newValue = _min + (range*value);
            *_data = newValue;
            
            if ( object && function ) {
                (*object.*function)( *_data );
            }
        }
    }
    const double GetValue()
    {
        UnknownType range = _max - _min;
        return double((_data-_min)/range);
    }
    const std::string GetValueString()
    {
        std::ostringstream buff;
        buff.precision(3);
        buff<<GetValue();
        return buff.str();
    }
private:
    UnknownType* _data;                         // Pointer to the data
    const UnknownType _min;
    const UnknownType _max;
    
    void ( UnknownClass::*function )( ... );    // Pointer to a member function
    UnknownClass* object;                       // Pointer to an object instance
};

#endif /* SLIDER_BEHAVIOR_H */
