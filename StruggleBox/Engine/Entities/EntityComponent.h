#ifndef BWO_COMPONENT_H
#define BWO_COMPONENT_H
//
//  EntityComponent.h
//  Defines a base class for entity components
//  Each component should handle one set of data
//  TODO: Add list of component types
//
//  Created by Ville-Veikko Urrila on 4/17/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#include <string>
#include "Entity.h"

class EntityComponent {
private:
public:
    std::string m_family;
    const int m_ownerID;
    EntityComponent( const int ownerID ) : m_ownerID(ownerID) {};
    virtual ~EntityComponent() {};
    virtual void Update( double delta ) = 0;
    
    const std::string GetFamily() const { return m_family; };
};

#endif
