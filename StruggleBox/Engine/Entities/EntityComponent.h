#ifndef ENTITY_COMPONENT_H
#define ENTITY_COMPONENT_H

#include <string>
#include "Entity.h"

//  Defines a base class for entity components
//  Each component should handle one set of data
class EntityComponent
{
public:
    EntityComponent(
		const int ownerID,
		const std::string family ) :
		_ownerID(ownerID),
		_family(family)
	{};
    virtual ~EntityComponent() {};
    virtual void update(const double delta) = 0;
    
	const int getOwnerID() const { return _ownerID; };
    const std::string getFamily() const { return _family; };

protected:
	const int _ownerID;
    std::string _family;
};

#endif /* ENTITY_COMPONENT_H */
