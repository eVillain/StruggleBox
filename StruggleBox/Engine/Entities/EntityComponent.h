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
		const EntityID ownerID,
		const std::string family ) :
		_ownerID(ownerID),
		_family(family)
	{};
    virtual ~EntityComponent() {};
    virtual void update(const double delta) = 0;
    
	const EntityID getOwnerID() const { return _ownerID; };
    const std::string& getFamily() const { return _family; };

protected:
	const EntityID _ownerID;
    std::string _family;
};

#endif /* ENTITY_COMPONENT_H */
