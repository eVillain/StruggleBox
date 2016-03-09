#ifndef LOCATOR_H
#define LOCATOR_H

#include "TypeMagic.h"
#include "Container.h"
#include <memory>
#include <map>

class Locator
{
public:
    Locator(){};
    virtual ~Locator(){};
    
    template <class InstanceClass>
    void MapInstance(InstanceClass* instance)
    {
        int instanceMagic = TypeMagic::MagicNumberFor<InstanceClass>();
        _injections[instanceMagic] = new Container(instance);
    }
    
    template <class InstanceClass, class InterfaceClass>
    void MapInstanceToInterface(InstanceClass* instance)
    {
        int interfaceMagic = TypeMagic::MagicNumberFor<InterfaceClass>();
        _injections[interfaceMagic] = new Container(instance);
    }
    
    template <class InstanceClass>
    void UnMap()
    {
        int instanceMagic = TypeMagic::MagicNumberFor<InstanceClass>();
        if (_injections.find(instanceMagic) != _injections.end())
            _injections.erase(instanceMagic);
    }
    
    template <class InterfaceClass>
    bool Satisfies() const
    {
        int interfaceMagic = TypeMagic::MagicNumberFor<InterfaceClass>();
        if (_injections.find(interfaceMagic) != _injections.end())
            return true;
        return false;
    }
    
    template <class InterfaceClass>
    InterfaceClass* Get()
    {
        int interfaceMagic = TypeMagic::MagicNumberFor<InterfaceClass>();
        if (_injections.find(interfaceMagic) != _injections.end())
            return _injections[interfaceMagic]->UnsafeCast<InterfaceClass>();
        return nullptr;
    }

private:
    std::map<int, Container*> _injections;
};

#endif /* LOCATOR_H */
