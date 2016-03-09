//
//  UIScrollerList.cpp
//  Ingenium
//
//  Created by The Drudgerist on 06/03/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "UIScrollerList.h"
#include "SysCore.h"
#include "Renderer.h"

UIScrollerListBase::UIScrollerListBase( int posX, int posY,
                               int width, int height,
                               std::string texDefault,
                               std::string texActive,
                               std::string texPressed ) :
UIWidget(posX, posY, width, height, 1, texDefault, texActive, texPressed)
{
    maxItems = 12;
    scrollPos = 0;
    
    dragging = false;
    
    scrollUpButton = NULL;
    scrollDownButton = NULL;
    
    g_uiMan->AddWidget(this);
    
    Refresh();
}

UIScrollerListBase::~UIScrollerListBase() {
    if ( scrollUpButton ) {
        ButtonBase::DeleteButton(scrollUpButton);
        scrollUpButton = NULL;
    }
    if ( scrollDownButton ) {
        ButtonBase::DeleteButton(scrollDownButton);
        scrollDownButton = NULL;
    }
    // Clear old list
    for ( unsigned int i=0; i<listedSet.size(); i++ ) {
        delete listedSet[i];
    }
    listedSet.clear();
    g_uiMan->RemoveWidget(this);
}
void UIScrollerListBase::Draw( Renderer *renderer ) {
    float yc = (float)(y+contentHeight);
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(x,y+1), glm::vec2(x,yc+h), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // L
    renderer->Buffer2DLine(glm::vec2(x,yc+h), glm::vec2(x+w-1,yc+h), COLOR_UI_BORDER1, COLOR_UI_BORDER1);   // T
    renderer->Buffer2DLine(glm::vec2(x+w,yc+h), glm::vec2(x+w,y+1), COLOR_UI_BORDER1, COLOR_UI_BORDER1);   // R
    renderer->Buffer2DLine(glm::vec2(x+w-1,y), glm::vec2(x,y), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // B
    renderer->Buffer2DLine(glm::vec2(x+w-1,yc), glm::vec2(x,yc), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // B
    // Inner gradient fill
    renderer->DrawGradientY(Rect2D((float)x, (float)yc+1, (float)w-1, (float)h-1), COLOR_UI_GRADIENT1, COLOR_UI_GRADIENT2);
    // Inside border
    glEnable(GL_BLEND);
    renderer->Draw2DRect(Rect2D(x+0.5f,yc+1.5f,w-2,h-2), COLOR_UI_BORDER2, COLOR_NONE);
    renderer->DrawGradientY(Rect2D((float)x, (float)y+1, (float)w-1, (float)contentHeight-1), COLOR_UI_GRADIENT1, COLOR_UI_GRADIENT2);
    renderer->Render2DLines();
}
void UIScrollerListBase::AddButton( std::string label, std::function<void()> function, std::string submenu ) {
    int vH = h-4;
    int posY = y+contentHeight;
    posY -= ( (int)listedSet.size() * vH );
    UIButtonLambda* button = UIButtonLambda::CreateButton(label, x+4, posY-vH, w-30, vH,
                                                          function, BUTTON_TYPE_DEFAULT, false );
    listedSet.push_back((UIWidget*)button);
    y -= ((UIWidget*)button)->h;
    contentHeight += ((UIWidget*)button)->h;
};
void UIScrollerListBase::ListItems( const std::vector<std::string> newSet ) {
    fullSet.clear();
    fullSet = newSet;
    Refresh();
}
void UIScrollerListBase::Refresh() {
    // Clear old list
    for ( unsigned int i=0; i<listedSet.size(); i++ ) {
        delete listedSet[i];
    }
    listedSet.clear();
    
    y += contentHeight;
    contentHeight = 0;
    
    // Add items from list
    if ( scrollPos > (int)(fullSet.size()-maxItems) ) scrollPos = (int)fullSet.size()-maxItems;
    for ( unsigned int i=scrollPos; i < fullSet.size(); i++ ) {
        if ( i >= (scrollPos+maxItems) ) break;
        this->AddButton(fullSet[i], ( [=]() { this->SelectItemCB(fullSet[i]); } ) );
    }
    
    if ( fullSet.size() > maxItems ) {    // Add scroll up and down buttons
        if ( !scrollUpButton ) {
            scrollUpButton = (ButtonBase*)UIButtonLambda::CreateButton("", x+w-22, y+contentHeight, 22, 22,
                                                                       ( [=]() {
                if ( fullSet.size() >= maxItems &&
                    scrollPos > 0 ) {
                    scrollPos--;
                    Refresh();
                }
            } ), BUTTON_TYPE_DEFAULT, false, "ArrowUpDefault.png", "ArrowUpActive.png", "ArrowUpPressed.png" );
        }
        if ( !scrollDownButton ) {
            scrollDownButton = (ButtonBase*)UIButtonLambda::CreateButton("", x+w-22, y, 22, 22,
                                                                         ( [=]() {
                if ( fullSet.size() > scrollPos+maxItems ) {
                    scrollPos++;
                    Refresh();
                }
            } ), BUTTON_TYPE_DEFAULT, false, "ArrowDownDefault.png", "ArrowDownActive.png", "ArrowDownPressed.png" );
        }
    } else {                                // Remove scroll up and down buttons
        if ( scrollUpButton ) {
            ButtonBase::DeleteButton(scrollUpButton);
            scrollUpButton = NULL;
        }
        if ( scrollDownButton ) {
            ButtonBase::DeleteButton(scrollDownButton);
            scrollDownButton = NULL;
        }
    }
}
void UIScrollerListBase::UpdatePos(int posX, int posY) {
    int moveX = posX - x;
    int moveY = posY - y;
    x = posX;
    y = posY;
    for ( unsigned int i=0; i<listedSet.size(); i++ ) {
        UIWidget* w = listedSet[i];
        if ( w->visible ) {
            w->UpdatePos(w->x+moveX, w->y+moveY);
        }
    }
    if ( scrollUpButton != NULL ) {
        scrollUpButton->UpdatePos(scrollUpButton->x+moveX, scrollUpButton->y+moveY);
    }
    if ( scrollDownButton != NULL ) {
        scrollDownButton->UpdatePos(scrollDownButton->x+moveX, scrollDownButton->y+moveY);
    }

}
//========================
//  Selected item callback
//=======================
void UIScrollerListBase::SelectItemCB(const std::string selection) {
    double timeNow = SysCore::GetSeconds();
//    if ( selection == selectedItem ) {
//        if (timeNow - lastTimeClicked < 0.3) {
//            // double clicked a line, select it
//            DoCallBack(selectedItem);
//            return;
//        }
//    }
    selectedItem = selection;
    lastTimeClicked = timeNow;
    
    // Above commented out code will only return value on double clicking item, here we return on single click
    DoCallBack(selectedItem);
}

void UIScrollerListBase::CursorHover(const glm::ivec2 coord, bool highlight)
{
    CheckHover(coord);
    highlighted = highlight;
}
void UIScrollerListBase::CursorPress(const glm::ivec2 coord)
{
    CheckPress(coord);
    active = true;
}

void UIScrollerListBase::CursorRelease(const glm::ivec2 coord)
{
    CheckRelease(coord);
    active = false;
}

bool UIScrollerListBase::CheckPress(const glm::ivec2 coord)
{
    if ( scrollUpButton ) {
        if( ((UIWidget*)scrollUpButton)->PointTest(coord) ) {
            scrollUpButton->CursorPress(coord);
            return true;
        }
    }
    if ( scrollDownButton ) {
        if( ((UIWidget*)scrollDownButton)->PointTest(coord) ) {
            scrollDownButton->CursorPress(coord);
            return true;
        }
    }
    
    int vY = y+contentHeight;
    if( coord.x > x  && coord.x < x+w && coord.y > vY-1 && coord.y < vY+h ) {
        dragging = true;        // Clicked scroller bar, moving menu
        dragX = coord.x; dragY = coord.y; // Store click coordinates
        return true;
    }
    for ( unsigned int i=0; i<listedSet.size(); i++ ) {
        UIWidget* w = listedSet[i];
        if( w->PointTest(coord) ) {
            w->CursorPress(coord);
            return true;
        }
    }
    return false;
}

bool UIScrollerListBase::CheckRelease(const glm::ivec2 coord)
{
    if ( dragging ) {
        dragging = false;
        return true;
    }
    if ( scrollUpButton ) {
        if( ((UIWidget*)scrollUpButton)->PointTest(coord) ) {
            scrollUpButton->CursorRelease(coord);
            return true;
        }
    }
    if ( scrollDownButton ) {
        if( ((UIWidget*)scrollDownButton)->PointTest(coord) ) {
            scrollDownButton->CursorRelease(coord);
            return true;
        }
    }
    for ( unsigned int i=0; i<listedSet.size(); i++ ) {
        UIWidget* w = listedSet[i];
        if( w->PointTest(coord) ) {
            w->CursorRelease(coord);
            return true;
        }
    }
    return false;
}
bool UIScrollerListBase::CheckHover(const glm::ivec2 coord)
{
    bool overWidget = false;
    if ( scrollUpButton ) {
        if( ((UIWidget*)scrollUpButton)->PointTest(coord) ) {
            scrollUpButton->CursorHover(coord, true);
            overWidget = true;
        } else {
            scrollUpButton->CursorHover(coord, false);
        }
    }
    if ( scrollDownButton ) {
        if( ((UIWidget*)scrollDownButton)->PointTest(coord) ) {
            scrollDownButton->CursorHover(coord, true);
            overWidget = true;
        } else {
            scrollDownButton->CursorHover(coord, false);
        }
    }
    
    if ( !overWidget ) {
        for ( unsigned int i=0; i<listedSet.size(); i++ ) {
            UIWidget* w = listedSet[i];
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


