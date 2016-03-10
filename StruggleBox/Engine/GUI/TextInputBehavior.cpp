#include "TextInputBehavior.h"

TextInputBehaviorLambda::TextInputBehaviorLambda(std::function<void(const std::string& inputText)> func) :
function(func)
{ }

void TextInputBehaviorLambda::Trigger(const std::string& inputText)
{
    if ( function )
    {
        function(inputText);
    }
}

