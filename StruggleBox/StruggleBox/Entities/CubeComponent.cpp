//
//  CubeComponent.cpp
//  Bloxelizer
//
//  Created by The Drudgerist on 8/29/13.
//
//

#include "CubeComponent.h"
#include "World3D.h"
#include "Cubeject.h"
#include "EntityManager.h"
CubeComponent::CubeComponent( const int ownerID, EntityManager* manager, const std::string& objectName ) :
EntityComponent(ownerID) {
    m_family = "Cube";
    m_manager = manager;
    m_object = NULL;
    m_instanceID = -1;
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    if ( !m_owner->HasAttribute("objectFile") ) {
        if ( objectName.empty() ) {
            printf("CubeComp: new object with no name?\n");
            m_owner->GetAttributeDataPtr<std::string>("objectFile") = "UnknownObject";
        } else {
            printf("CubeComp: new object\n");
            // Save the filename into an attribute
            m_object = m_manager->world->LoadObject(objectName);
            m_owner->GetAttributeDataPtr<std::string>("objectFile") = objectName;
        }
        m_owner->GetAttributeDataPtr<glm::vec3>("scale") = glm::vec3(0.1f);
    }
    LoadObject();
}

CubeComponent::~CubeComponent() {
    printf("Killed cubes for %i\n", m_ownerID);
    UnloadObject();
    m_object = NULL;
    m_instanceID = -1;
}
void CubeComponent::LoadObject() {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    if ( !m_object ) {
        if ( m_owner->HasAttribute("objectFile")) {
            const std::string objFileAttr = m_owner->GetAttributeDataPtr<std::string>("objectFile");
            printf("CubeComp: had object %s\n", objFileAttr.c_str());
            m_object = m_manager->world->LoadObject(objFileAttr);
        } else {
            printf("Object load failed\n");
        }
    }
    if ( m_object ) {
        if ( m_instanceID == -1 ) {
            if ( m_owner->HasAttribute("instanceID")) {
                const int instIDAttr = m_owner->GetAttributeDataPtr<int>("instanceID");
                printf("CubeComp: had instance %i\n", instIDAttr);
                if ( instIDAttr == -1 ) {
                    glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
                    m_instanceID = m_object->AddInstance(position);
                    m_owner->GetAttributeDataPtr<int>("instanceID") = m_instanceID;
                } else {
                    m_instanceID = instIDAttr;
                }
            } else {
                glm::vec3 position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
                m_instanceID = m_object->AddInstance(position);
                m_owner->GetAttributeDataPtr<int>("instanceID") = m_instanceID;
                printf("CubeComp: new instance, id %i\n", m_instanceID);
            }
        }
        // Bounding box
        float blockWidth = BLOCK_RADIUS;
        float width = m_object->cubes->GetWidth()*blockWidth;
        float height = m_object->cubes->GetHeight()*blockWidth;
        glm::vec3& bb = m_owner->GetAttributeDataPtr<glm::vec3>("bb");
        bb = glm::vec3(width,height,width);
    }
}

void CubeComponent::UnloadObject()
{
    if ( m_instanceID != -1 ) {
        Entity* m_owner = m_manager->GetEntity(m_ownerID);
        if ( !m_object->RemoveInstance(m_instanceID) ) {
            printf("CubeComp: unable to remove instance %i\n", m_instanceID);
        }
        m_instanceID = -1;
        m_owner->GetAttributeDataPtr<int>("instanceID") = m_instanceID;
        m_object = NULL;
    }
}

void CubeComponent::Update( double delta ) {
    if ( m_instanceID != -1 ) {
        Entity* m_owner = m_manager->GetEntity(m_ownerID);
        InstanceData* instance = instance = m_object->GetInstance(m_instanceID);
        instance->position = m_owner->GetAttributeDataPtr<glm::vec3>("position");
        instance->rotation = m_owner->GetAttributeDataPtr<glm::quat>("rotation");
        instance->scale = m_owner->GetAttributeDataPtr<glm::vec3>("scale");
    }
}

