#ifndef BUTTON_BEHAVIOR_H
#define BUTTON_BEHAVIOR_H

#include <functional>

class ButtonBehavior
{
public:
    virtual ~ButtonBehavior(){};
    virtual void Trigger() = 0;
};

class ButtonBehaviorLambda : public ButtonBehavior
{
public:
    ButtonBehaviorLambda( std::function<void()> func );
    void Trigger();
private:
    std::function<void()> function;
};

class ButtonBehaviorToggle : public ButtonBehavior
{
public:
    ButtonBehaviorToggle( bool* clientData );
    void Trigger();
private:
    bool* toggleData;
};

template <class UnknownClass>
class ButtonBehaviorMember {
public:
    ButtonBehaviorMember(UnknownClass* objectPtr,
                         void( UnknownClass::*func )( void* ),
                         void* clientData) :
    function(func),
    object(objectPtr),
    callBackData(clientData)
    { }
    void Trigger()
    {
        if ( object && function ) {
            (*object.*function)( callBackData );
        }
    }
private:
    void ( UnknownClass::*function )( void* );  // Pointer to a member function
    UnknownClass* object;                       // Pointer to an object instance
    void* callBackData;                         // Data pointer to pass with callback
};

template <typename UnknownType>
class ButtonBehaviorVariable {
public:
    ButtonBehaviorVariable(UnknownType* objectP,
                           UnknownType clientData) :
    object(objectP),
    callBackData(clientData)
    { }
    void Trigger()
    {
        if ( object ) {
            UnknownType* obj = object;
            *obj = callBackData;
        }
    }
private:
    UnknownType* object;                    // Pointer to the object owner
    UnknownType callBackData;
};

#endif /* BUTTON_BEHAVIOR_H */
