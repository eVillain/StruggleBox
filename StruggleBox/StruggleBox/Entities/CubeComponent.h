//
//  CubeComponent.h
//  Bloxelizer
//
//  Created by The Drudgerist on 8/29/13.
//
//

#ifndef BWO_CUBE_COMPONENT_H
#define BWO_CUBE_COMPONENT_H

#include "EntityComponent.h"
class Cubeject;
class EntityManager;

class CubeComponent : public EntityComponent {
    EntityManager* m_manager;
    Cubeject* m_object;
    int m_instanceID;
public:
    CubeComponent( const int ownerID, EntityManager* manager, const std::string& objectName );
    virtual ~CubeComponent();
    void LoadObject();
    void UnloadObject();
    virtual void Update( double delta );
    
};

#endif
