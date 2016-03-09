#ifndef Container_h
#define Container_h

#include "TypeMagic.h"
#include <memory>

class Container
{
public:
    template <typename T_>
    Container(const T_ & t) :
    _value(new ContainerValue<T_>(t)){}
    
    // And now for the tricky bit ¯\_(ツ)_/¯
    template <typename T_>
    T_ * UnsafeCast() const
    {
        return (T_*)_value->Raw();
    }
        
    template <typename T_>
    bool IsSameType()
    {
        return TypeMagic::MagicNumberFor<T_>() == _value->magicNumber;
    }
    
    const int GetMagicNumber() const
    {
        return _value->magicNumber;
    }
    
private:
    struct ContainerValueBase
    {
        int magicNumber;
        
        ContainerValueBase(const int m) :
        magicNumber(m){}
        
        virtual ~ContainerValueBase(){}
        
        virtual void* Raw() { return nullptr; }
    };
    
    template <typename T_>
    struct ContainerValue :
    ContainerValueBase
    {
        T_ value;
        
        ContainerValue(const T_ & v) :
        ContainerValueBase(TypeMagic::MagicNumberFor<T_>()),
        value(v)
        {
        }
        
        void* Raw() { return (void*)value; }
    };
    
    std::shared_ptr<ContainerValueBase> _value;
};

#endif /* Container_h */
