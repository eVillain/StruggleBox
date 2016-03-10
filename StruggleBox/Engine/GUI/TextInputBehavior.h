#ifndef GUI_TEXT_INPUT_BEHAVIOR_H
#define GUI_TEXT_INPUT_BEHAVIOR_H

#include <functional>
#include <string>

class TextInputBehavior
{
public:
    virtual ~TextInputBehavior(){};
    virtual void Trigger(const std::string& inputText) = 0;
};

class TextInputBehaviorLambda : public TextInputBehavior
{
public:
    TextInputBehaviorLambda( std::function<void(const std::string&)> func );
    void Trigger(const std::string& inputText);
private:
    std::function<void(const std::string&)> function;
};

template <class UnknownClass>
class TextInputBehaviorCallback : public TextInputBehavior {
public:
    TextInputBehaviorCallback( UnknownClass* objectPtr = NULL,
                              void( UnknownClass::*func )( std::string& ) = NULL ) :
    object(objectPtr),
    function(func)
    { }
    void Trigger(const std::string& inputText)
    {
        if ( object && function ) {
            (*object.*function)( inputText );
        }
    }
private:
    // Text input callback attributes
    void ( UnknownClass::*function )( const std::string& );
    UnknownClass* object;
};

class TextInputBehaviorStaticCallback : public TextInputBehavior {
public:
    TextInputBehaviorStaticCallback(void( *func )( const std::string& ) = NULL ) :
    function(func)
    { }
    void Trigger(const std::string& inputText)
    {
        if ( function ) {
            (function)( inputText );
        }
    }
private:
    void ( *function )( const std::string& );
};

#endif /* GUI_TEXT_INPUT_BEHAVIOR_H */
