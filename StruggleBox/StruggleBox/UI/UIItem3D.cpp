//
//  UIItem3D.cpp
//  Ingenium
//
//  Created by The Drudgerist on 20/03/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "UIItem3D.h"
#include "Entity.h"
#include "Cubeject.h"
#include "World3D.h"
#include "Renderer.h"
#include "TextManager.h"
#include "EntityManager.h"
#include "Timer.h"

UIItem3D::UIItem3D( const int posX, const int posY, const int ownerID, EntityManager* manager, const int width, const int height ) :
UIWidget(posX, posY, width, height),
m_ownerID(ownerID), m_manager(manager) {
    nameBlobID = -1;
    damageBlobID = -1;
}
UIItem3D::~UIItem3D() {
    if ( nameBlobID != -1 ) {
        g_uiMan->GetTextManager()->RemoveText(nameBlobID);
        nameBlobID = -1;
    }
    if ( damageBlobID != -1 ) {
        g_uiMan->GetTextManager()->RemoveText(damageBlobID);
        damageBlobID = -1;
    }
}

void UIItem3D::Draw( Renderer* renderer ) {
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(x,y+1), glm::vec2(x,y+h), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // L
    renderer->Buffer2DLine(glm::vec2(x,y+h), glm::vec2(x+w-1,y+h), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);   // T
    renderer->Buffer2DLine(glm::vec2(x+w,y+h), glm::vec2(x+w,y+1), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);   // R
    renderer->Buffer2DLine(glm::vec2(x+w-1,y), glm::vec2(x,y), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // B
    renderer->Render2DLines();
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    if ( m_owner != NULL ) {
        int typeAttrib = m_owner->GetAttributeDataPtr<int>("type");
        if ( typeAttrib != ENTITY_ITEM ) {
            printf("[UIItem3D] Error, entity not of item type\n");
        }
        const std::string objectName = m_owner->GetAttributeDataPtr<std::string>("objectFile");
        Cubeject* object = m_manager->world->LoadObject(objectName);
        if ( object ) {
            std::vector<InstanceData> instances;    // Temporary instance buffer to render just the one object
            glm::quat rot = glm::quat(glm::vec3(0, Timer::Seconds(), 0));
            
            if ( highlighted ) {
                rot = glm::quat(glm::vec3(0,90.0f,0));
                if ( nameBlobID == -1 ) {
                    const std::string itemName = m_owner->GetAttributeDataPtr<std::string>("name");
                    nameBlobID = g_uiMan->GetTextManager()->AddText(itemName, glm::vec3(x+64,y+48,z), true, 16, FONT_BENTHAM, 0.0, COLOR_UI_TEXT_HIGHLIGHT);
                }
                if ( damageBlobID == -1 ) {
                    std::string itemDmg = "Damage: ";
                    itemDmg.append( intToString( m_owner->GetAttributeDataPtr<int>("damage") ) );
                    damageBlobID = g_uiMan->GetTextManager()->AddText(itemDmg, glm::vec3(x+64,y+32,z), true, 16, FONT_BENTHAM, 0.0, COLOR_UI_TEXT_HIGHLIGHT);
                }
            } else {
                if ( nameBlobID != -1 ) {
                    g_uiMan->GetTextManager()->RemoveText(nameBlobID);
                    nameBlobID = -1;
                }
                if ( damageBlobID != -1 ) {
                    g_uiMan->GetTextManager()->RemoveText(damageBlobID);
                    damageBlobID = -1;
                }
            }
            instances.push_back({glm::vec3(x+32,y+32,z), rot, glm::vec3(3.0f)});
            renderer->RenderInstancedBuffer(object->vBuffer, instances,
                                            object->vBuffer->numVerts+object->vBuffer->numTVerts, 0,
                                            NULL, false);
        } else {
            printf("[UIItem3D] Error getting object %s for rendering\n", objectName.c_str());
        }
    } else {    // Render empty item box?
        
    }
}

void UIItem3D::Update()
{ }

// Override these for different cursor events
void UIItem3D::CursorHover(const glm::ivec2 coord, bool highlight)
{
    this->highlighted = highlight;
}

void UIItem3D::CursorPress(const glm::ivec2 coord)
{
    this->active = true;
}

void UIItem3D::CursorRelease(const glm::ivec2 coord)
{
    this->active = false;
}
