#ifndef NGN_ATTRIBUTE_H
#define NGN_ATTRIBUTE_H

//
//  Attribute.h
//  Ingenium data Attribute class
//  These are a form of type-erasure, no run-time types needed
//  Instead the magic_number can be used to check data type
//
//  Created by Ville-Veikko Urrila on 4/19/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#include <memory>
#include "GFXHelpers.h"

// Sanity check class, should never get called unless something is broken
class SomethingIsSomethingElse
{
    
};

class Attribute
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
    struct AttributeValueBase
    {
        int magic_number;
        
        AttributeValueBase(const int m) :
        magic_number(m)
        {
        }
        
        virtual ~AttributeValueBase()
        {
        }
    };
    
    template <typename T_>
    struct AttributeValue :
    AttributeValueBase
    {
        T_ value;
        
        AttributeValue(const T_ & v) :
        AttributeValueBase(magic_number_for<T_>()),
        value(v)
        {
        }
    };
    
    std::shared_ptr<AttributeValueBase> _value;
    
public:
    template <typename T_>
    Attribute(const T_ & t) :
    _value(new AttributeValue<T_>(t))
    {
    }

    template <typename T_>
    T_ & as() const
    {
        if (magic_number_for<T_>() != _value->magic_number)
            throw "[Attribute] magic number mismatch, wrong data type";;
        return std::static_pointer_cast< AttributeValue<T_> >(_value)->value;
    }
    const int GetMagicNumber() const { return _value->magic_number; };
    template <typename T_>
    bool IsType() {
        if (magic_number_for<T_>() == _value->magic_number) {
            return true;
        }
        return false;
    }
    const std::string GetValueString()
    {
        if (magic_number_for<bool>() == _value->magic_number) {
            return boolToString(std::static_pointer_cast< AttributeValue<bool> >(_value)->value);
        } else if (magic_number_for<int>() == _value->magic_number) {
            return intToString(std::static_pointer_cast< AttributeValue<int> >(_value)->value);
        } else if (magic_number_for<float>() == _value->magic_number) {
            return floatToString(std::static_pointer_cast< AttributeValue<float> >(_value)->value);
        } else if (magic_number_for<double>() == _value->magic_number) {
            return doubleToString(std::static_pointer_cast< AttributeValue<double> >(_value)->value);
        } else if (magic_number_for<std::string>() == _value->magic_number) {
            return std::string(std::static_pointer_cast< AttributeValue<std::string> >(_value)->value);
        }
        return "Unknown Attribute type";
    };

};

#endif
