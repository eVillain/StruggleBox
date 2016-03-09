#ifndef ENTITY_COMPONENT_H
#define ENTITY_COMPONENT_H

#include <string>
#include "Entity.h"

//  Defines a base class for entity components
//  Each component should handle one set of data
class EntityComponent
{
public:
    EntityComponent( const int ownerID ) : m_ownerID(ownerID) {};
    virtual ~EntityComponent() {};
    virtual void Update( double delta ) = 0;
    
    const std::string GetFamily() const { return m_family; };
    const int GetOwnerID() const { return m_ownerID; };
protected:
    std::string m_family;
    const int m_ownerID;
};

#endif /* ENTITY_COMPONENT_H */
