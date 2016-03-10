#include "ButtonBehavior.h"

ButtonBehaviorLambda::ButtonBehaviorLambda( std::function<void()> func) :
function(func)
{ }

void ButtonBehaviorLambda::Trigger()
{
    if ( function )
    {
        function();
    }
}

ButtonBehaviorToggle::ButtonBehaviorToggle( bool* clientData ) :
toggleData(clientData)
{ }

void ButtonBehaviorToggle::Trigger()
{
    if ( toggleData )
    {
        *toggleData = !(*toggleData );
    }
}
