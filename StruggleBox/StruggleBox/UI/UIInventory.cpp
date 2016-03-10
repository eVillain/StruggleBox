//
//  UIInventory.cpp
//  Ingenium
//
//  Created by The Drudgerist on 28/12/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#include "UIInventory.h"
#include "EntityManager.h"
#include "Renderer.h"
#include "Options.h"
#include "Entity.h"
#include "InventoryComponent.h"
#include "HumanoidComponent.h"
#include "CubeComponent.h"
#include "TextManager.h"
#include "UIButton.h"
#include "World3D.h"
#include "Cubeject.h"
#include "UIItem3D.h"

UIInventory::UIInventory( const int posX, const int posY, const int ownerID, EntityManager* manager ) :
UIWidget(posX,posY,512,386) {
    m_ownerID = ownerID;
    m_manager = manager;
    g_uiMan->AddWidget(this);
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    int charType = m_owner->GetAttributeDataPtr<int>("type");
    TextManager* tMan = g_uiMan->GetTextManager();
    typeLabelID = tMan->AddText(NameForEntity((EntityType)charType), glm::vec3(x+8,y+h-8,0) );
    closeButton = (UIWidget*)UIButtonLambda::CreateButton("Close", x+2+(w/2), y-22, (w/2)-4, 22, { [=](){
        this->wantsClose = true;
    } }, BUTTON_TYPE_DEFAULT, false);

    wantsClose = false;
    rhItemWidget = NULL;
    lhItemWidget = NULL;
    RefreshItems();
}
UIInventory::~UIInventory() {
    if ( typeLabelID != -1 ) {
        TextManager* tMan = g_uiMan->GetTextManager();
        tMan->RemoveText(typeLabelID);
    }
    if ( closeButton ) {
        ButtonBase::DeleteButton((ButtonBase*)closeButton);
        closeButton = NULL;
    }
    if ( rhItemWidget ) {
        delete rhItemWidget;
        rhItemWidget = NULL;
    }
    if ( lhItemWidget ) {
        delete lhItemWidget;
        lhItemWidget = NULL;
    }

    for (int i=0; i < menuItemList.size(); i++) {
        delete menuItemList[i];
    }
    menuItemList.clear();
    m_ownerID = 0;
    m_manager = 0;
    g_uiMan->RemoveWidget(this);
}
void UIInventory::RefreshItems() {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    int typeAttrib = m_owner->GetAttributeDataPtr<int>("type");
    if ( typeAttrib == ENTITY_HUMANOID ) {
        const HumanoidComponent* humanComp = (HumanoidComponent*)m_manager->GetComponent(m_ownerID, "Humanoid");
        if ( humanComp != NULL ) {
            Entity* bp = humanComp->backPack;
            if ( bp ) {             // Backpack
                int bpID = bp->GetAttributeDataPtr<int>("ID");
                InventoryComponent* inventory = (InventoryComponent*)m_manager->GetComponent(bpID, "Inventory");
                if ( inventory != NULL ) {
                    std::vector<Entity*> items = inventory->GetInventory();
                    for (int i=0; i < menuItemList.size(); i++) {
                        delete menuItemList[i];
                    }
                    menuItemList.clear();
                    if ( items.size() > 0 ) {
                        const int itemGridSize = 64;
                        const int maxItemsX = (w-128)/itemGridSize;
                        const int maxItemsY = (h/itemGridSize) - 1;
                        int renderedItem = 0;
                        // Render items in backpack
                        for (int yItem=0; yItem < maxItemsY; yItem++) {
                            for (int xItem=0; xItem < maxItemsX; xItem++) {
                                Entity* item = items[renderedItem];
                                int typeAttrib = item->GetAttributeDataPtr<int>("type");
                                if ( typeAttrib == ENTITY_ITEM ) {
                                    int itemID = item->GetAttributeDataPtr<int>("ID");
                                    glm::vec3 itemPos = glm::vec3(x+xItem*itemGridSize, y+h-(64+yItem*itemGridSize), z);
                                    UIItem3D* newItem = new UIItem3D( itemPos.x,itemPos.y, itemID, m_manager);
                                    menuItemList.push_back(newItem);
                                }
                                renderedItem++;
                                if ( renderedItem >= items.size() ) break;
                            }
                            if ( renderedItem >= items.size() ) break;
                        }
                    }
                } else {
                    printf("[UIInventory] Error: no inventory component in backpack\n");
                }
            }
        }
    }
}
void UIInventory::DrawObjectAt( Renderer* renderer, const::std::string& objectName,
                               glm::vec3 pos, glm::quat rot, glm::vec3 scale ) {
    Cubeject* object = m_manager->world->LoadObject(objectName);
    if ( object ) {
        std::vector<InstanceData> instances;    // Temporary instance buffer to render just the one object
        instances.push_back({pos, rot, scale});
        renderer->RenderInstancedBuffer(object->vBuffer, instances,
                                        object->vBuffer->numVerts+object->vBuffer->numTVerts, 0,
                                        NULL, false);
    } else {
        printf("[UIInventory] Error getting object %s for rendering\n", objectName.c_str());
    }

}
void UIInventory::Draw(Renderer* renderer) {
    if ( !visible ) return;
    int vY = y;
    int vH = h;
    if ( closeButton ) {
        vY -= 24;
        vH += 24;
        renderer->Buffer2DLine(glm::vec2(x+w-1,vY+22+4), glm::vec2(x,vY+22+4), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // B
    }
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(x,vY+1), glm::vec2(x,vY+vH), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // L
    renderer->Buffer2DLine(glm::vec2(x,vY+vH), glm::vec2(x+w-1,vY+vH), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);   // T
    renderer->Buffer2DLine(glm::vec2(x+w,vY+vH), glm::vec2(x+w,vY+1), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);   // R
    renderer->Buffer2DLine(glm::vec2(x+w-1,vY), glm::vec2(x,vY), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // B
    if ( closeButton ) {
    }
    // Inner gradient fill
    renderer->DrawGradientY(Rect2D((float)x, (float)vY+1, (float)w-1, (float)vH-1), COLOR_UI_GRADIENT_TOP, COLOR_UI_GRADIENT_BOTTOM);
    // Inside border
    glEnable(GL_BLEND);
    renderer->Draw2DRect(Rect2D(x+1,vY+1,w-2,vH-2), COLOR_UI_BORDER_INNER, COLOR_NONE);
    renderer->Render2DLines();
    glDepthMask(GL_TRUE);   // Need depth mask for 3D object display
    
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    int typeAttrib = m_owner->GetAttributeDataPtr<int>("type");
    if ( typeAttrib == ENTITY_HUMANOID ) {
        const HumanoidComponent* humanComp = (HumanoidComponent*)m_manager->GetComponent(m_ownerID, "Humanoid");
        if ( humanComp != NULL ) {
            const std::string torsoObject = m_owner->GetAttributeDataPtr<std::string>("torsoObject");
            const std::string headObject = m_owner->GetAttributeDataPtr<std::string>("headObject");
            const std::string headAccessoryObject = m_owner->GetAttributeDataPtr<std::string>("headAccessoryObject");
            const std::string leftFootObject = m_owner->GetAttributeDataPtr<std::string>("leftFootObject");
            const std::string rightFootObject = m_owner->GetAttributeDataPtr<std::string>("rightFootObject");
            const std::string leftHandObject = m_owner->GetAttributeDataPtr<std::string>("leftArmObject");
            const std::string rightHandObject = m_owner->GetAttributeDataPtr<std::string>("rightArmObject");
            const float z = 20.0f;
            const float scaleCube = 2.0f;
            glm::vec3 torsoPos = glm::vec3(x+w-80,y+80,z);
            glm::vec3 headPos = torsoPos+glm::vec3(0.0f,28*scaleCube,0.0f);
            glm::vec3 footLPos  = torsoPos+glm::vec3( 8*scaleCube, -14*scaleCube, 0.0f );
            glm::vec3 footRPos  = torsoPos+glm::vec3(-8*scaleCube, -14*scaleCube, 0.0f );
            glm::vec3 handLPos  = torsoPos+glm::vec3( 14*scaleCube, 0.0f, 0.0f );
            glm::vec3 handRPos  = torsoPos+glm::vec3(-14*scaleCube, 0.0f, 0.0f );
            
            // Turn head and torso to face cursor
            glm::vec2 headLook = lookAtScrnPos-glm::vec2(headPos.x,headPos.y);
            if ( headLook.x || headLook.y ) {
                const float headRotSpeed = 0.1f;
                const float torsoRotSpeed = 0.05f;
                headLook = glm::normalize(headLook);
                glm::quat wantedRot = glm::quat(glm::vec3(-headLook.y*(headLook.x*headLook.x),headLook.x,0));
                glm::quat newRot = glm::slerp(headRot, wantedRot, headRotSpeed);
                headRot = newRot;
                glm::quat wantedTorsoRot = glm::quat(glm::vec3(0,headLook.x*0.5f,0));
                torsoRot = glm::slerp(torsoRot, wantedTorsoRot, torsoRotSpeed);
            } else {
                headRot = glm::quat();
            }
//            glm::quat rot = glm::quat(glm::vec3(0,glfwGetTime(),0));
            glm::vec3 scale = glm::vec3(scaleCube*4.0f);

            DrawObjectAt(renderer, torsoObject, torsoPos, torsoRot, scale);
            DrawObjectAt(renderer, headObject, headPos, headRot, scale);
            DrawObjectAt(renderer, headAccessoryObject, headPos, headRot, scale);
            DrawObjectAt(renderer, leftFootObject, footLPos, glm::quat(), scale);
            DrawObjectAt(renderer, rightFootObject, footRPos, glm::quat(), scale);
            DrawObjectAt(renderer, leftHandObject, handLPos, glm::quat(), scale);
            DrawObjectAt(renderer, rightHandObject, handRPos, glm::quat(), scale);

            Entity* rhi = humanComp->rightHandItem;
            Entity* lhi = humanComp->leftHandItem;
            Entity* bp = humanComp->backPack;
            
            if ( rhItemWidget && rhi ) {
                int rhiID = rhi->GetAttributeDataPtr<int>("ID");
                if ( rhItemWidget->GetOwnerEntity() != rhiID ) {
                    delete rhItemWidget;
                    rhItemWidget = NULL;
                }
            }
            if ( lhItemWidget && lhi ) {
                int lhiID = lhi->GetAttributeDataPtr<int>("ID");
                if ( lhItemWidget->GetOwnerEntity() != lhiID ) {
                    delete lhItemWidget;
                    lhItemWidget = NULL;
                }
            }
            if ( rhi && !rhItemWidget ) {   // Right hand item
                int rhiID = rhi->GetAttributeDataPtr<int>("ID");
                glm::vec3 rhiPos = torsoPos+glm::vec3(-128, 0, 0);
                rhItemWidget = new UIItem3D( rhiPos.x,rhiPos.y,rhiID,m_manager );
            }
            if ( lhi && !lhItemWidget ) {   // Left hand item
                int lhiID = lhi->GetAttributeDataPtr<int>("ID");
                glm::vec3 lhiPos = torsoPos+glm::vec3(+128, 0, 0);
                lhItemWidget = new UIItem3D( lhiPos.x,lhiPos.y,lhiID,m_manager );
            }
            if ( bp ) {                     // Backpack
                int bpID = bp->GetAttributeDataPtr<int>("ID");
                InventoryComponent* inventory = (InventoryComponent*)m_manager->GetComponent(bpID, "Inventory");
                if ( inventory != NULL ) {
                    std::vector<Entity*> items = inventory->GetInventory();
                    if ( inventory->updated ) {
                        RefreshItems();
                        inventory->updated = false;
                    }
                } else {
                    printf("[UIInventory] Error: no inventory component in backpack\n");
                }
            }
        } else {
            printf("[UIInventory] Error: no humanoid component in humanoid entity\n");
        }
    }
    if ( rhItemWidget ) rhItemWidget->Draw(renderer);
    if ( lhItemWidget ) lhItemWidget->Draw(renderer);
    for (int i=0; i < menuItemList.size(); i++) {
        menuItemList[i]->Draw(renderer);
    }
    glDepthMask(GL_FALSE);
}

void UIInventory::SelectItem(const int itemID) {
    Entity* m_owner = m_manager->GetEntity(m_ownerID);
    int typeAttrib = m_owner->GetAttributeDataPtr<int>("type");
    if ( typeAttrib == ENTITY_HUMANOID ) {
        HumanoidComponent* humanComp = (HumanoidComponent*)m_manager->GetComponent(m_ownerID, "Humanoid");
        if ( humanComp != NULL ) {
            Entity* bp = humanComp->backPack;
            if ( bp ) {             // Backpack
                int bpID = bp->GetAttributeDataPtr<int>("ID");
                InventoryComponent* inventory = (InventoryComponent*)m_manager->GetComponent(bpID, "Inventory");
                if ( inventory != NULL ) {
                    std::vector<Entity*> items = inventory->GetInventory();
                    if ( items.size() > itemID ) {
                        Entity * clickedItem = items[itemID];
                        inventory->RemoveItem(clickedItem);
                        humanComp->Wield(clickedItem);
                    }
                }
            }
        }
    }
}
bool UIInventory::CheckPress(const glm::ivec2 coord) {
    if ( rhItemWidget ) {
        if ( rhItemWidget->PointTest(coord) ) {
            rhItemWidget->CursorPress(coord);
            return true;
        }
    }
    if ( lhItemWidget ) {
        if ( lhItemWidget->PointTest(coord) ) {
            lhItemWidget->CursorPress(coord);
            return true;
        }
    }
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if( w->PointTest(coord) ) {
            w->CursorPress(coord);
            return true;
        }
    }
    int vY = y;
    if ( !minimized ) { vY += contentHeight; }
    if( coord.x > x  && coord.x < x+w && coord.y > vY-1 && coord.y < vY+h ) {
        dragging = true;        // Clicked menu bar, moving menu
        dragX = coord.x; dragY = coord.y; // Store click coordinates
        return true;
    }
    return false;
}
bool UIInventory::CheckRelease(const glm::ivec2 coord) {
    if ( dragging ) {
        dragging = false;
        return true;
    }
    if ( rhItemWidget ) {
        if ( rhItemWidget->PointTest(coord) ) {
            rhItemWidget->CursorRelease(coord);
            HumanoidComponent* humanComp = (HumanoidComponent*)m_manager->GetComponent(m_ownerID, "Humanoid");
            if ( humanComp != NULL ) { humanComp->Wield(NULL); }
            return true;
        }
    }
    if ( lhItemWidget ) {
        if ( lhItemWidget->PointTest(coord) ) {
            lhItemWidget->CursorRelease(coord);
            return true;
        }
    }
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if( w->PointTest(coord) ) {
            w->CursorRelease(coord);
            SelectItem(i);
            return true;
        }
    }
    return false;
}
bool UIInventory::CheckHover(const glm::ivec2 coord) {
    if ( !visible ) return false;
    lookAtScrnPos = coord;
    if ( coord.y < y-1 ) return false;
    if ( dragging ) {
        int distX = coord.x-dragX;
        int distY = coord.y-dragY;
        dragX = coord.x; dragY = coord.y;
        int vY = y;
        if ( !minimized) { vY += contentHeight; }
        UpdatePos(x+distX, vY+distY);
        return true;
    }
    if ( rhItemWidget ) {
        if ( rhItemWidget->PointTest(coord) ) {
            rhItemWidget->CursorHover(coord, true);
            return true;
        } else {
            rhItemWidget->CursorHover(coord, false);
        }
    }
    if ( lhItemWidget ) {
        if ( lhItemWidget->PointTest(coord) ) {
            lhItemWidget->CursorHover(coord, true);
            return true;
        } else {
            lhItemWidget->CursorHover(coord, false);
        }
    }

    bool overWidget = false;
    if ( !overWidget ) {
        for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
            UIWidget* w = menuItemList[i];
            if( w->PointTest(coord) ) {
                w->CursorHover(coord, true);
                overWidget = true;
            } else {
                w->CursorHover(coord, false);
            }
        }
    }
    return overWidget;
}

void UIInventory::CursorHover(const glm::ivec2 coord, bool highlight)
{
    CheckHover(coord);
    highlighted = highlight;
}

void UIInventory::CursorPress(const glm::ivec2 coord)
{
    CheckPress(coord);
    active = true;
}

void UIInventory::CursorRelease(const glm::ivec2 coord)
{
    CheckRelease(coord);
    active = false;
}

void UIInventory::UpdatePos( int posX, int posY ) {
    int moveX = posX - x;
    int moveY = posY - y;
    if ( !minimized ) moveY = (posY - contentHeight) - y;
    
    x = posX; y = posY;
    if ( !minimized ) {
        for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
            UIWidget* w = menuItemList[i];
            if ( w->visible ) {
                w->UpdatePos(w->x+moveX, w->y+moveY);
            }
        }
    }
    if ( rhItemWidget ) {
        rhItemWidget->UpdatePos(rhItemWidget->x+moveX, rhItemWidget->y+moveY);
    }
    if ( lhItemWidget ) {
        lhItemWidget->UpdatePos(lhItemWidget->x+moveX, lhItemWidget->y+moveY);
    }
    if ( closeButton ) {
        ((UIWidget*)closeButton)->UpdatePos(closeButton->x+moveX, closeButton->y+moveY);
    }
}
