#include "UIMenu.h"
#include "TextManager.h"
#include "SpriteBatch.h"
#include "Texture.h"
#include "SysCore.h"
#include "GFXHelpers.h"

#include "Shader.h"
#include "Renderer.h"
#include "UIButton.h"

UIMenu::UIMenu( int posX, int posY,
                int width, int height,
                std::string title,
                UIMenu* parentMenu,
                std::string texDefault,
                std::string texActive,
                std::string texPressed ) :
UIWidget( posX, posY, width, height) {
    contentHeight = 0;

    if ( !texDefault.empty() ) {
        texture = g_uiMan->GetBatch()->texture;
        if ( g_uiMan->GetBatch()->GetIDForFrame(texDefault) == -1 ) {
            printf("[UIMenu] couldnt get id for frame %s\n", texDefault.c_str() );
            texture = NULL;
        } else {
            // Set frames for widget
            frameDefault = texDefault;
            if ( !texActive.empty() ) { frameActive = texActive; }
            if ( !texPressed.empty() ) { framePressed = texPressed; }
        }
    } else {
        texture = NULL;
    }
    // Tie to parent if submenu
    parent = parentMenu;
    if ( parentMenu == NULL ) moveable = true;
    
    // Add menu label
    int textRatio = (int)(height-4);
    int xSpacer = 6;
    int ySpacer = (int)(height*0.3);

    if ( texture != NULL ) {
        textRatio = std::max<int>( (int)(width*0.08), (int)(height*0.4) );
        xSpacer = std::max<int>( (int)(width*0.1), (int)(height*0.2) ) ;
    }
    label = title;
    labelID = g_uiMan->GetTextManager()->AddText(label, glm::vec3(x+xSpacer, y+ySpacer, 0.0), true, textRatio, FONT_JURA );
    g_uiMan->AddWidget(this);
    
    minimizeBtn = (ButtonBase*)UIButton<UIMenu>::CreateButton("-", x+width-(xSpacer+12), y+(h/2)-6, 12, 12,
                                                              this, &UIMenu::Minimize, NULL, BUTTON_TYPE_SIMPLE, false );
    minimized = false;
    dragging = false;
    
    maxItems = 10;
    scrollPos = 0;
}
UIMenu::~UIMenu( void ) {
    if ( minimizeBtn ) {
        ButtonBase::DeleteButton(minimizeBtn);
        minimizeBtn = NULL;
    }
    ClearWidgets();
    g_uiMan->RemoveWidget(this);
    g_uiMan->GetTextManager()->RemoveText(this->labelID);
}
void UIMenu::AddWidget( UIWidget *widget ) {
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if ( widget == w ) {
            // Tried to add widget twice
            return;
        }
    }
    menuItemList.push_back(widget);
}
void UIMenu::RemoveWidget( UIWidget* widget ) {
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if ( widget == w ) {
            menuItemList.erase( menuItemList.begin()+i );
            return;
        }
    }
}
void UIMenu::ClearWidgets( void ) {
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        delete menuItemList[i];
    }
    std::map<std::string, UIMenu*>::iterator it;
    for ( it=subMenus.begin(); it != subMenus.end(); it++ ) {
        UIMenu* w = it->second;
        delete w;
    }
    menuItemList.clear();
    subMenus.clear();
}
void UIMenu::UpdatePos( int posX, int posY ) {
    int moveX = posX - x;
    int moveY = posY - y;
    if ( !minimized ) moveY = (posY - contentHeight) - y;
    
    x = posX; y = posY;
//    if ( !minimized ) {
        std::map<std::string, UIMenu*>::iterator it;
        for ( it=subMenus.begin(); it != subMenus.end(); it++ ) {
            UIMenu* w = it->second;
            if ( w->visible ) {
                y -= w->h;
            }
            int mY = w->y+moveY;
            if ( w->visible && !w->minimized ) {
                y -= w->contentHeight;
                mY += w->contentHeight;
            }
            w->UpdatePos(w->x+moveX, mY);
        }
        for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
            UIWidget* w = menuItemList[i];
            w->UpdatePos(w->x+moveX, w->y+moveY);
            if ( w->visible ) {
                y -= w->h;
            }
        }
//    }
    int xSpacer = 6;
    if ( labelID != -1 ) {
        int ySpacer = (int)(h*0.3);
        if ( texture != NULL ) {
            xSpacer = std::max<int>( (int)(w*0.1), (int)(h*0.2) ) ;
        }
        g_uiMan->GetTextManager()->UpdateTextPos( labelID, glm::vec3(x+xSpacer, y+ySpacer+contentHeight, 0.0) );
    }
    if ( minimizeBtn ) {
        minimizeBtn->UpdatePos( x+w-(xSpacer+12), y+6+contentHeight );
    }
}
void UIMenu::UpdateSize( int width, int height ) {
    float wRatio = (float)w / width;
    float hRatio = (float)h / height;
    w = width; h = height;
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        w->UpdateSize( (int)(w->w*wRatio), (int)(w->h*hRatio) );
    }
}

bool UIMenu::PointTest(const glm::ivec2 coord)
{
    if ( !visible ) return false;
    // If point is within button area, then returns true
    int vH = h;
    if ( !minimized ) {
        vH += contentHeight;
    }
    if(coord.x > x  &&
       coord.x < x+w &&
       coord.y > y-1   &&
       coord.y < y+vH ) {
        return true;
	}
	return false;
}

bool UIMenu::CheckPress(const glm::ivec2 coord)
{
    if( ((UIWidget*)minimizeBtn)->PointTest(coord) ) {
        minimizeBtn->CursorPress(coord);
        return true;
    }
    
    int vY = y;
    if ( !minimized ) { vY += contentHeight; }
    if( coord.x > x  && coord.x < x+w && coord.y > vY-1 && coord.y < vY+h ) {
        dragging = true;        // Clicked menu bar, moving menu
        
        dragX = coord.x; dragY = coord.y; // Store click coordinates
        return true;
    }
    
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if( w->PointTest(coord) ) {
            w->CursorPress(coord);
            return true;
        }
    }
    std::map<std::string, UIMenu*>::iterator it;
    for ( it=subMenus.begin(); it != subMenus.end(); it++ ) {
        UIMenu* w = it->second;
        if( w->PointTest(coord) ) {
            w->CursorPress(coord);
            return true;
        }
    }
    return false;
}

bool UIMenu::CheckRelease(const glm::ivec2 coord)
{
    if ( dragging ) {
        dragging = false;
        return true;
    }
    if( ((UIWidget*)minimizeBtn)->PointTest(coord) ) {
        minimizeBtn->CursorRelease(coord);
        return true;
    }
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if( w->PointTest(coord) ) {
            w->CursorRelease(coord);
            return true;
        }
    }
    std::map<std::string, UIMenu*>::iterator it;
    for ( it=subMenus.begin(); it != subMenus.end(); it++ ) {
        UIMenu* w = it->second;
        if( w->PointTest(coord) ) {
            w->CursorRelease(coord);
            return true;
        }
    }
    return false;
}

bool UIMenu::CheckHover(const glm::ivec2 coord, bool highlight)
{
    if ( !visible ) return false;
    if ( dragging ) {
        int distX = coord.x-dragX;
        int distY = coord.y-dragY;
        dragX = coord.x; dragY = coord.y;
        int vY = y;
        if ( !minimized) { vY += contentHeight; }
        UpdatePos(x+distX, vY+distY);
        return true;
    }
    bool overWidget = false;
    if( ((UIWidget*)minimizeBtn)->PointTest(coord) ) {
        minimizeBtn->CursorHover(coord, true);
        overWidget = true;
    } else {
        minimizeBtn->CursorHover(coord, false);
    }
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
    if ( !overWidget ) {
        std::map<std::string, UIMenu*>::iterator it;
        for ( it=subMenus.begin(); it != subMenus.end(); it++ ) {
            UIMenu* w = it->second;
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
void UIMenu::CursorHover(const glm::ivec2 coord, bool highlight)
{
    CheckHover(coord, highlight);
    highlighted = highlight;
}

void UIMenu::CursorPress(const glm::ivec2 coord)
{
    CheckPress(coord);
    active = true;
}

void UIMenu::CursorRelease(const glm::ivec2 coord)
{
    CheckRelease(coord);
    active = false;
}

void UIMenu::Draw( Renderer* renderer ) {
    if ( !visible ) return;
//    glDepthMask(GL_FALSE);
    if ( texture ) {
        Rect2D texRect;
        if ( active && !frameActive.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameActive );
        } else if ( highlighted && !framePressed.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( framePressed );
        } else {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameDefault );
        }
		renderer->DrawTexture(Rect2D((float)x, (float)y, (float)w, (float)h+contentHeight), texRect, texture->GetID());
    } else {
        if ( highlighted ) {
            g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT_HIGHLIGHT);
        } else {
            g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT);
        }
        float yc = (float)(y+contentHeight);
        // Pixel perfect outer border (should render with 1px shaved off corners)
        renderer->Buffer2DLine(glm::vec2(x,y+1), glm::vec2(x,yc+h), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // L
        renderer->Buffer2DLine(glm::vec2(x,yc+h), glm::vec2(x+w-1,yc+h), COLOR_UI_BORDER1, COLOR_UI_BORDER1);   // T
        renderer->Buffer2DLine(glm::vec2(x+w,yc+h), glm::vec2(x+w,y+1), COLOR_UI_BORDER1, COLOR_UI_BORDER1);   // R
        renderer->Buffer2DLine(glm::vec2(x+w-1,y), glm::vec2(x,y), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // B
        renderer->Buffer2DLine(glm::vec2(x+w-1,yc), glm::vec2(x,yc), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // B
        glEnable(GL_BLEND);
        // Inner gradient fill
        Color gradCol1 = COLOR_UI_GRADIENT1; gradCol1.a = 0.5f;
        Color gradCol2 = COLOR_UI_GRADIENT2; gradCol2.a = 0.5f;
        renderer->DrawGradientY(Rect2D((float)x, (float)yc+1, (float)w-1, (float)h-1), gradCol1, gradCol2);
        // Inside border
        renderer->Draw2DRect(Rect2D(x+0.5f,yc+1.5f,w-2,h-2), COLOR_UI_BORDER2, COLOR_NONE);
        renderer->DrawGradientY(Rect2D((float)x, (float)y+1, (float)w-1, (float)contentHeight-1), gradCol1, gradCol2);
        renderer->Render2DLines();
    }
    
    for ( unsigned int i=0; i < menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if ( i == 16 ) continue;
        // Update inner labels
        w->Update();
        // Render button at i
        w->Draw( renderer );
    }
    std::map<std::string, UIMenu*>::iterator it;
    for ( it=subMenus.begin(); it != subMenus.end(); it++ ) {
        UIMenu* w = it->second;
        w->Update();
    }
//    glDepthMask(GL_TRUE);
}

void UIMenu::Minimize(void *unused) {
    minimized = !minimized;
    if ( !visible ) return;
    if ( minimized ) {
        minimizeBtn->UpdateLabel("+");
    } else {
        minimizeBtn->UpdateLabel("-");
    }
    std::map<std::string, UIMenu*>::iterator it;
    for ( it=subMenus.begin(); it != subMenus.end(); it++ ) {
        UIMenu* w = it->second;
        if ( minimized ) {
            w->Hide();
        } else {
            w->Show();
        }
        w->Update();
    }
    
    for ( unsigned int i=0; i < menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if ( minimized ) {
            w->visible = false;
        } else {
            w->visible = true;
        }
        w->Update();
    }
    Sort();
    if ( parent ) parent->Sort();
}

void UIMenu::Update() {
//    if ( visible ) {
//        if ( labelID == -1 ) {
//            int textRatio = (int)(h*0.8);
//            int xSpacer = 6;
//            int ySpacer = (int)(h*0.3);
//            if ( texture != NULL ) {
//                textRatio = std::max<int>( (int)(w*0.08), (int)(h*0.4) );
//                xSpacer = std::max<int>( (int)(w*0.1), (int)(h*0.2) ) ;
//            }
//            labelID = g_uiMan->GetTextManager()->AddText(label, glm::vec3(x+xSpacer, y+ySpacer+contentHeight, 0.0), true, textRatio, FONT_JURA );
//        }
//        if ( minimizeBtn == NULL ) {
//            minimizeBtn = (ButtonBase*)UIButton<UIMenu>::CreateButton("+", x+w-16, y+2+contentHeight, 10, 10, this, &UIMenu::Minimize, NULL, BUTTON_TYPE_SIMPLE, true );
//            minimized = false;
//        }
//    } else {
//        if ( minimizeBtn ) {
//            ButtonBase::DeleteButton(minimizeBtn);
//            minimizeBtn = NULL;
//        }
//        if ( labelID != -1 ) {
//            g_uiMan->GetTextManager()->RemoveText(this->labelID);
//            labelID = -1;
//        }
//    }    
//    
//    for ( unsigned int i=0; i < menuItemList.size(); i++ ) {
//        UIWidget* w = menuItemList[i];
//        if ( minimized || !visible ) {
//            w->visible = false;
//        } else {
//            w->visible = true;
//        }
//        w->Update();
//    }
}


void UIMenu::Sort() {
    y += contentHeight;
    contentHeight = 0;
    if ( !visible ) return;
    
    std::map<std::string, UIMenu*>::iterator it;
    for ( it=subMenus.begin(); it != subMenus.end(); it++ ) {
        UIMenu* w = it->second;
        if ( w->visible ) {
            y -= w->h;
            contentHeight += w->h;
            w->UpdatePos(w->x, y);
            if ( !w->minimized ) {
                y -= w->contentHeight;
                contentHeight += w->contentHeight;
            }
//            y -= 4;
//            contentHeight += 4;
        }
    }
    if ( !minimized ) {
        for ( unsigned int i=0; i < menuItemList.size(); i++ ) {
            UIWidget* w = menuItemList[i];
            if ( w->visible ) {
                y -= w->h;
                contentHeight += w->h;
                w->UpdatePos(w->x, y);
//                y -= 2;
//                contentHeight += 2;
            }
        }
    }
}

void UIMenu::Hide() {
    visible = false;
    if ( !minimized  ) {
        for ( unsigned int i=0; i < menuItemList.size(); i++ ) {
            UIWidget* w = menuItemList[i];
            w->visible = false;
            w->Update();
        }
    }

    if ( minimizeBtn ) {
        ButtonBase::DeleteButton(minimizeBtn);
        minimizeBtn = NULL;
    }
    if ( labelID != -1 ) {
        g_uiMan->GetTextManager()->RemoveText(this->labelID);
        labelID = -1;
    }
}
void UIMenu::Show() {
    visible = true;
    if ( !minimized ) {
//        for ( unsigned int i=0; i < menuItemList.size(); i++ ) {
        for ( int i=0; i < menuItemList.size(); i++ ) {
            UIWidget* w = menuItemList[i];
            w->visible = true;
            w->Update();
        }
    }
    if ( labelID == -1 ) {
        int textRatio = (int)(h*0.8);
        int xSpacer = 6;
        int ySpacer = (int)(h*0.3);
        if ( texture != NULL ) {
            textRatio = std::max<int>( (int)(w*0.08), (int)(h*0.4) );
            xSpacer = std::max<int>( (int)(w*0.1), (int)(h*0.2) ) ;
        }
        labelID = g_uiMan->GetTextManager()->AddText(label, glm::vec3(x+xSpacer, y+ySpacer+contentHeight, 0.0), true, textRatio, FONT_JURA );
    }
    if ( minimizeBtn == NULL ) {
        std::string minSymbol = minimized ? "+" : "-";
        minimizeBtn = (ButtonBase*)UIButton<UIMenu>::CreateButton(minSymbol, x+w-16, y+4+contentHeight, 10, 10, this, &UIMenu::Minimize, NULL, BUTTON_TYPE_SIMPLE, false );
    }
}
