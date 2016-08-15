#ifndef GENERIC_CALLBACK_H
#define GENERIC_CALLBACK_H

#include <functional>

class Callback
{
public:
	virtual ~Callback() {};
};

template<typename T>
class GenericCallback : Callback
{
public:
    virtual ~GenericCallback() {};
    virtual void Trigger(const T& data) = 0;
};

template<typename T>
class CallbackLambda : public GenericCallback<T>
{
public:
	CallbackLambda(std::function<void(const T&)> function) :
		_function(function)
	{ }
    
    void Trigger(const T& data)
	{
		if (_function)
		{
			_function(data);
		}
	}
private:
    std::function<void(const T&)> _function;
};

template <typename T, class UnknownClass>
class CallbackMember : public GenericCallback<T>
{
public:
    CallbackMember(UnknownClass* objectPtr,
                   void(UnknownClass::*func)(const T&)) :
    function(func),
    object(objectPtr)
    { }
    
    void Trigger(const T& data)
    {
        if (object && function)
		{
            (*object.*function)(data);
        }
    }
private:
    /// Pointer to a member function
    void (UnknownClass::*function)(const T&);
    /// Pointer to an object instance
    UnknownClass* object;
};

#endif
