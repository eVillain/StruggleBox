#ifndef LIST_BEHAVIOR_H
#define LIST_BEHAVIOR_H

#include <functional>
#include <string>

class ListBehavior
{
public:
    virtual ~ListBehavior(){};
    virtual void Trigger(const std::string& data) = 0;
};

class ListBehaviorLambda : public ListBehavior
{
public:
    ListBehaviorLambda(std::function<void(const std::string&)> function);
    
    void Trigger(const std::string& data);
private:
    std::function<void(const std::string&)> function;
};

template <class UnknownClass>
class ListBehaviorMember : public ListBehavior
{
public:
    ListBehaviorMember(UnknownClass* objectPtr,
                       void(UnknownClass::*func)(const std::string&)) :
    function(func),
    object(objectPtr)
    { }
    
    void Trigger(const std::string& data)
    {
        if ( object && function ) {
            (*object.*function)(data);
        }
    }
private:
    /// Pointer to a member function
    void (UnknownClass::*function)(const std::string&);
    /// Pointer to an object instance
    UnknownClass* object;
};



#endif /* LIST_BEHAVIOR_H */
