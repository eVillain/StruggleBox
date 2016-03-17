#include "ListBehavior.h"

ListBehaviorLambda::ListBehaviorLambda(std::function<void(const std::string&)> func) :
function(func)
{ }

void ListBehaviorLambda::Trigger(const std::string& data)
{
    if ( function )
    {
        function(data);
    }
}
