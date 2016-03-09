#include "UIFileMenu.h"
#include <algorithm>
#include "FileUtil.h"
#include "Timer.h"
#include "TextManager.h"
#include "Shader.h"
#include "Texture.h"
#include "Renderer.h"

UIFileMenuBase::UIFileMenuBase( int posX, int posY,
                               int width, int height,
                               std::string path, std::string type,
                               std::string title,
                               std::string defaultName,
                               bool loading,
                               std::string texDefault,
                               std::string texActive,
                               std::string texPressed ) :
UIWidget(posX, posY, width, height, WIDGET_DEPTH, texDefault, texActive, texPressed),
filePath(path),
fileType(type),
defaultFile(defaultName),
isLoading(loading),
lastTimeClicked(0.0),
textWidget(NULL) {
    textWidget = NULL;
    selectButton = NULL;
    closeButton = NULL;
    scrollerList = NULL;
    contentHeight = 0;
    
    // Add menu label
    int textRatio = (int)(height*0.8);
    int xSpacer = 6;
    int ySpacer = (int)(height*0.3);
    
    if ( texture != NULL ) {    // Make the text smaller due to a bigger border
        textRatio = std::max<int>( (int)(width*0.08), (int)(height*0.4) );
        xSpacer = std::max<int>( (int)(width*0.1), (int)(height*0.2) ) ;
    }
    label = title;
    labelID = g_uiMan->GetTextManager()->AddText(label, glm::vec3(x+xSpacer, y+ySpacer, 0.0), true, textRatio, FONT_JURA );

    g_uiMan->AddWidget(this);

    RefreshItems();
}
UIFileMenuBase::~UIFileMenuBase() {
    if ( textWidget ) {
        delete textWidget;
        textWidget = NULL;
    }
    if ( selectButton ) {
        ButtonBase::DeleteButton(selectButton);
        selectButton = NULL;
    }
    if ( closeButton ) {
        ButtonBase::DeleteButton(closeButton);
        closeButton = NULL;
    }
    if ( scrollerList ) {
        delete scrollerList;
        scrollerList = NULL;
    }
    if (textWidget ) {
        delete textWidget;
        textWidget = NULL;
    }
    
    fileList.clear();
    
    g_uiMan->RemoveWidget(this);
    g_uiMan->GetTextManager()->RemoveText(this->labelID);
}
void UIFileMenuBase::RefreshItems() {
    if ( textWidget ) {
        delete textWidget;
        textWidget = NULL;
    }
    if ( selectButton ) {
        ButtonBase::DeleteButton(selectButton);
        selectButton = NULL;
    }
    if ( closeButton ) {
        ButtonBase::DeleteButton(closeButton);
        closeButton = NULL;
    }

    y += contentHeight;
    contentHeight = 0;
    
    fileList.clear();
    if ( fileType.empty() ) {
        FileUtil::GetAllFiles(filePath, fileList);
    } else {
        FileUtil::GetFilesOfType(filePath, fileType, fileList);
    }
    if ( scrollerList == NULL ) {
        scrollerList = new UIScrollerList<UIFileMenuBase>(x+2, y-h, w-4, 22,
                                                          this, &UIFileMenuBase::SelectFileCB );
    }
    // Add new list to scroller
    scrollerList->ListItems(fileList);
    y -= scrollerList->h+scrollerList->contentHeight+2;
    contentHeight += scrollerList->h+scrollerList->contentHeight+2;

    // Show text input widget
    textWidget = new UITextInput<UIFileMenuBase>(x+2, y-40, w-40, 22, 1, "Filename: ",defaultFile, this, &UIFileMenuBase::ReceiveFileName );
    y -= textWidget->h+2;
    contentHeight += textWidget->h+2;
    std::string selectBtnLabel = isLoading ? "Load" : "Save";
    selectButton = (ButtonBase*)UIButton<UIFileMenuBase>::CreateButton( selectBtnLabel, x+2, y-h, (w/2)-4, 22,
                                                                       this, &UIFileMenuBase::SelectCB, NULL, BUTTON_TYPE_DEFAULT );
    closeButton = (ButtonBase*)UIButton<UIFileMenuBase>::CreateButton( "Close", x+2+(w/2), y-h, (w/2)-4, 22, this, &UIFileMenuBase::CloseCB, NULL, BUTTON_TYPE_DEFAULT );
    y -= selectButton->h+2;
    contentHeight += selectButton->h+2;
    
}


//========================
//  File button callback
//=======================
void UIFileMenuBase::SelectFileCB( std::string fileName ) {
    double timeNow = Timer::Seconds();
    if ( fileName == selectedFile ) {
        if (timeNow - lastTimeClicked < 0.3) {
            // double clicked a file, select it
            ReceiveFileName( fileName );
            return;
        }
    }
    selectedFile = fileName;
    lastTimeClicked = timeNow;
    textWidget->SetText( selectedFile );
}
void UIFileMenuBase::SelectCB( void* unused ) {
    if ( selectedFile.length() == 0 ) {
        selectedFile = textWidget->GetText();
    }
    if ( selectedFile.length() == 0 ) {
        ReceiveFileName( defaultFile );
    } else {
        ReceiveFileName( selectedFile );
    }
}
void UIFileMenuBase::CloseCB( void* unused ) {
    DoCallBack("");
}

//========================
//  Text input callback
//=======================
void UIFileMenuBase::ReceiveFileName( std::string fileName )
{
    if ( fileName.length() == 0 ) return;
    // Check if filename contains extension
    if (fileName.length() > fileType.length()) {
        if (fileName.compare (fileName.length() - fileType.length(), fileType.length(), fileType) != 0) {
            // It doesn't, append it
            printf("appending file extension\n");
            fileName.append( fileType );
        }
    } else {
        // FileName was shorter than wanted file extension, append extension
        fileName.append( fileType );
    }
    if ( isLoading ) {
        // Make sure file actually exists before passing it forward
        bool fileExists = FileUtil::DoesFileExist(filePath, fileName);
        if ( !fileExists ) {
            printf("File didnt exist:%s at %s\n", fileName.c_str(), filePath.c_str());
            g_uiMan->GetTextManager()->AddText("No such file!", glm::vec3(-150, -350, 0.0), true, 18, FONT_DEFAULT, 2.0f);
            return;
        }
    }
    std::string returner = filePath;
    returner.append(fileName);
    DoCallBack( returner );
}


void UIFileMenuBase::UpdatePos( int posX, int posY ) {
    int moveX = posX - x;
    int moveY = (posY - contentHeight) - y;
    x = posX;
    y = posY;

    if ( selectButton != NULL ) {
        selectButton->UpdatePos(selectButton->x+moveX, selectButton->y+moveY);
        y -= selectButton->h+2;
    }
    if ( closeButton != NULL ) {
        closeButton->UpdatePos(closeButton->x+moveX, closeButton->y+moveY);
    }
    if (textWidget != NULL ) {
        textWidget->UpdatePos(textWidget->x+moveX, textWidget->y+moveY);
        y -= textWidget->h+2;
    }
    if (scrollerList != NULL ) {
        scrollerList->UpdatePos(scrollerList->x+moveX, scrollerList->y+moveY);
        y -= scrollerList->h+scrollerList->contentHeight+2;
    }
    if ( labelID != -1 ) {
        int xSpacer = 6;
        int ySpacer = (int)(h*0.3);
        if ( texture != NULL ) {
            xSpacer = std::max<int>( (int)(w*0.1), (int)(h*0.2) ) ;
        }
        g_uiMan->GetTextManager()->UpdateTextPos( labelID, glm::vec3(x+xSpacer, y+ySpacer+contentHeight, 0.0) );
    }
}
void UIFileMenuBase::Draw( Renderer *renderer ) {
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
    // Inner gradient fill
    Color gradCol1 = COLOR_UI_GRADIENT1; gradCol1.a = 0.5f;
    Color gradCol2 = COLOR_UI_GRADIENT2; gradCol2.a = 0.5f;
    renderer->DrawGradientY(Rect2D((float)x, (float)yc+1, (float)w-1, (float)h-1), gradCol1, gradCol2);
    // Inside border
    glEnable(GL_BLEND);
    renderer->Draw2DRect(Rect2D(x+0.5f,yc+1.5f,w-2,h-2), COLOR_UI_BORDER2, COLOR_NONE);
    renderer->DrawGradientY(Rect2D((float)x, (float)y+1, (float)w-1, (float)contentHeight-1), gradCol1, gradCol2);
    renderer->Render2DLines();
}
bool UIFileMenuBase::PointTest(const glm::ivec2 coord) {
    if ( !visible ) return false;
    // If point is within button area, then returns true
    int vH = h + contentHeight;
    if( coord.x > x  &&
       coord.x < x+w &&
       coord.y > y-1   &&
       coord.y < y+vH ) {
        return true;
	}
	return false;
}
void UIFileMenuBase::CursorHover(const glm::ivec2 coord, bool highlight)
{
    CheckHover(coord);
    highlighted = highlight;
}
void UIFileMenuBase::CursorPress(const glm::ivec2 coord) {
    CheckPress(coord);
    active = true;
}
void UIFileMenuBase::CursorRelease(const glm::ivec2 coord) {
    CheckRelease(coord);
    active = false;
}
bool UIFileMenuBase::CheckPress(const glm::ivec2 coord) {
    if( ((UIWidget*)scrollerList)->PointTest(coord) ) {
        scrollerList->CursorPress(coord);
        return true;
    }
    //    if( ((UIWidget*)scrollUpButton)->PointTest(tx,ty) ) {
//        scrollUpButton->CursorPress( tx, ty );
//        return true;
//    } else if( ((UIWidget*)scrollDownButton)->PointTest(tx,ty) ) {
//        scrollDownButton->CursorPress( tx, ty );
//        return true;
//    } else
    if( ((UIWidget*)selectButton)->PointTest(coord) ) {
        selectButton->CursorPress(coord);
        return true;
    } else if( ((UIWidget*)closeButton)->PointTest(coord) ) {
        closeButton->CursorPress(coord);
        return true;
    } else if( ((UIWidget*)textWidget)->PointTest(coord) ) {
        textWidget->CursorPress(coord);
        return true;
    }
    
    int vY = y+contentHeight;
    
    if( coord.x > x  && coord.x < x+w && coord.y > vY-1 && coord.y < vY+h ) {
        dragging = true;        // Clicked menu bar, moving menu
        dragX = coord.x; dragY = coord.y; // Store click coordinates
        return true;
    }
//    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
//        UIWidget* w = menuItemList[i];
//        if( w->PointTest(tx,ty) ) {
//            w->CursorPress( tx, ty );
//            return true;
//        }
//    }
    return false;
}

bool UIFileMenuBase::CheckRelease(const glm::ivec2 coord) {
    if ( dragging ) {
        dragging = false;
        return true;
    }
//    if( ((UIWidget*)scrollUpButton)->PointTest(tx,ty) ) {
//        scrollUpButton->CursorRelease( tx, ty );
//        return true;
//    }
//    if( ((UIWidget*)scrollDownButton)->PointTest(tx,ty) ) {
//        scrollDownButton->CursorRelease( tx, ty );
//        return true;
//    }
    if( ((UIWidget*)selectButton)->PointTest(coord) ) {
        selectButton->CursorRelease(coord);
        return true;
    }
    if( ((UIWidget*)closeButton)->PointTest(coord) ) {
        closeButton->CursorRelease(coord);
        return true;
    }
    if( ((UIWidget*)textWidget)->PointTest(coord) ) {
        textWidget->CursorRelease(coord);
        return true;
    }
    if ( ((UIWidget*)scrollerList->PointTest(coord) ) ) {
        scrollerList->CursorRelease(coord);
    }
//    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
//        UIWidget* w = menuItemList[i];
//        if( w->PointTest(tx,ty) ) {
//            w->CursorRelease(tx, ty);
//            return true;
//        }
//    }
    return false;
}
bool UIFileMenuBase::CheckHover(const glm::ivec2 coord) {
    if ( !visible ) return false;
    if ( dragging ) {
        int distX = coord.x-dragX;
        int distY = coord.y-dragY;
        dragX = coord.x; dragY = coord.y;
        int vY = y;
        vY += contentHeight;
        UpdatePos(x+distX, vY+distY);
        return true;
    }
    bool overWidget = false;
//    if( ((UIWidget*)scrollUpButton)->PointTest(tx,ty) ) {
//        scrollUpButton->CursorHover( true, tx, ty );
//        overWidget = true;
//    } else {
//        scrollUpButton->CursorHover( false, tx, ty );
//    }
//    if( ((UIWidget*)scrollDownButton)->PointTest(tx,ty) ) {
//        scrollDownButton->CursorHover( true, tx, ty );
//        overWidget = true;
//    } else {
//        scrollDownButton->CursorHover( false, tx, ty );
//    }
    if( ((UIWidget*)selectButton)->PointTest(coord) ) {
        selectButton->CursorHover(coord, true);
        overWidget = true;
    } else {
        selectButton->CursorHover(coord, false);
    }
    if( ((UIWidget*)closeButton)->PointTest(coord) ) {
        closeButton->CursorHover(coord, true);
        overWidget = true;
    } else {
        closeButton->CursorHover(coord, false);
    }
    if( ((UIWidget*)textWidget)->PointTest(coord) ) {
        textWidget->CursorHover(coord, true);
        overWidget = true;
    } else {
        textWidget->CursorHover(coord, false);
    }
    if ( !overWidget ) {
//        for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
//            UIWidget* w = menuItemList[i];
//            if( w->PointTest(tx,ty) ) {
//                w->CursorHover( true, tx, ty );
//                overWidget = true;
//            } else {
//                w->CursorHover( false, tx, ty );
//            }
//        }
    }
    return overWidget;
}

