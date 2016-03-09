#ifndef NGN_CVAR_H
#define NGN_CVAR_H
#include "GFXHelpers.h"
#include <string>
#include <memory>

class CVarIsWrongType
{
};

class ConsoleVar
{
private:
    static int next_magic_number()
    {
        static int magic(0);
        return magic++;
    }
public:
    template <typename T_>
    static int magic_number_for()
    {
        static int result(next_magic_number());
        return result;
    }
private:
    struct ConsoleVarValueBase
    {
        int magic_number;
        
        ConsoleVarValueBase(const int m) :
        magic_number(m)
        {
        }
        
        virtual ~ConsoleVarValueBase()
        {
        }
    };
    
    template <typename T_>
    struct ConsoleVarValue :
    ConsoleVarValueBase
    {
        T_& value;
        ConsoleVarValue(T_ & v) :
        ConsoleVarValueBase(magic_number_for<T_>()),
        value(v)
        {
        }
    };
    
    std::shared_ptr<ConsoleVarValueBase> _value;
    
public:
    template <typename T_> ConsoleVar(T_ & t) :
    _value(new ConsoleVarValue<T_>(t))
    {
    }
    
    template <typename T_> T_ & as() const
    {
        if (magic_number_for<T_>() != _value->magic_number)
            throw CVarIsWrongType();
        return std::static_pointer_cast< ConsoleVarValue<T_> >(_value)->value;
    }
    
    int GetMagicNumber() { return _value->magic_number; };
    
    template <typename T_> bool IsType()
    {
        if (magic_number_for<T_>() == _value->magic_number) {
            return true;
        }
        return false;
    }
    const std::string GetString()
    {
        if (magic_number_for<bool>() == _value->magic_number) {
            return boolToString(std::static_pointer_cast< ConsoleVarValue<bool> >(_value)->value);
        } else if (magic_number_for<int>() == _value->magic_number) {
            return intToString(std::static_pointer_cast< ConsoleVarValue<int> >(_value)->value);
        } else if (magic_number_for<float>() == _value->magic_number) {
            return floatToString(std::static_pointer_cast< ConsoleVarValue<float> >(_value)->value);
        } else if (magic_number_for<double>() == _value->magic_number) {
            return doubleToString(std::static_pointer_cast< ConsoleVarValue<double> >(_value)->value);
        } else if (magic_number_for<std::string>() == _value->magic_number) {
            return std::string(std::static_pointer_cast< ConsoleVarValue<std::string> >(_value)->value);
        }
        return "Unknown CVar type";
    };
};

#endif
