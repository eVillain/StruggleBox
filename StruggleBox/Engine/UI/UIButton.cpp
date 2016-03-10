#include "UIButton.h"
#include "UIManager.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Console.h"
#include "GFXDefines.h"
#include "TextManager.h"
#include "SpriteBatch.h"
#include "Texture.h"
#include "Rect2D.h"
#include "Shader.h"
#include "Renderer.h"


#define BUTTON_TEXT_HEIGHT 16
#define TEXT_RATIO 0.6
#define TEXTUREBTN_BORDER 4.0

// Our set of buttons
std::vector<ButtonBase*> ButtonBase::_buttonList;

ButtonBase::ButtonBase( std::string buttonLabel,
                       int posX, int posY,
                       int width,int height,
                       int buttonType,
                       bool canToggle,
                       std::string texDefault,
                       std::string texActive,
                       std::string texPressed )  :
UIWidget(posX, posY, width, height, (int)BUTTON_BG_DEPTH, texDefault, texActive, texPressed ),
label(buttonLabel),
toggleable(canToggle),
type(buttonType),
state(BUTTON_OFF) {
    if ( buttonType == BUTTON_TYPE_TEXTURED ) {
        if ( texDefault.empty() ) { 
            type = BUTTON_TYPE_DEFAULT;
        }
    }
    // Add each button to list on creation
    _buttonList.push_back(this);
    
    g_uiMan->AddWidget(this);
    
    labelID = -1;
    Update();
}
ButtonBase::~ButtonBase( void ) {
    g_uiMan->RemoveWidget(this);
    g_uiMan->GetTextManager()->RemoveText(this->labelID);
}
ButtonBase* ButtonBase::CreateButton( std::string buttonLabel,
                                     int posX, int posY,
                                     int width,int height,
                                     int buttonType,
                                     bool canToggle ) {
    ButtonBase * newButton;
    newButton = new ButtonBase( buttonLabel, posX,posY,width,height, buttonType, canToggle );
    return newButton;
}
ButtonBase* ButtonBase::CreateButton(std::string buttonLabel,
                                     int posX, int posY,
                                     int width,int height,
                                     bool canToggle,
                                     std::string texDefault,
                                     std::string texActive,
                                     std::string texPressed ) {
    ButtonBase * newButton;
    newButton = new ButtonBase(buttonLabel, posX,posY,width,height, BUTTON_TYPE_TEXTURED, canToggle, texDefault, texActive, texPressed );
    return newButton;
}
void ButtonBase::DeleteButton( ButtonBase* button ) {
    for ( unsigned int i=0; i<_buttonList.size(); i++ ) {
        if ( _buttonList[i] == button ) {
            _buttonList.erase(_buttonList.begin()+i);
            delete button;
            return;
        }
    }
}
void ButtonBase::DetachButton( ButtonBase* button ) {
    for ( unsigned int i=0; i<_buttonList.size(); i++ ) {
        if ( _buttonList[i] == button ) {
            _buttonList.erase(_buttonList.begin()+i);
            return;
        }
    }
}
// Unsafe - refer to widgets by pointer instead of label
void ButtonBase::DeleteButtonByLabel(std::string label) {
    for ( unsigned int i=0; i<_buttonList.size(); i++ ) {
        if ( label.compare(_buttonList[i]->label) == 0 ) {
            ButtonBase* b = _buttonList[i];
            _buttonList.erase(_buttonList.begin()+i);
            delete b;
            return;
        }
    }
    Console::Print("ButtonBase: Button '%s' not found for removal!\n", label.c_str());
}
void ButtonBase::DeleteAllButtons( void ) {
    for ( unsigned int i=0; i < _buttonList.size(); i++) {
            ButtonBase* b = _buttonList[i];
            delete b;
    }
    _buttonList.clear();
}
void ButtonBase::ToggleButton( ButtonBase* button ) {
    for ( unsigned int i=0; i<_buttonList.size(); i++ ) {
        if ( _buttonList[i] == button ) {
            ButtonBase* b = _buttonList[i];
            if ( b->state == BUTTON_ON ) {
                b->state = BUTTON_OFF;
            } else if ( b->state == BUTTON_OFF ) {
                b->state = BUTTON_ON;
            }
            return;
        }
    }
}
// Unsafe - use above by referring to widget directly by pointer
void ButtonBase::ToggleButtonByLabel(std::string label) {
    for ( unsigned int i=0; i<_buttonList.size(); i++ ) {
        if ( label.compare(_buttonList[i]->label) == 0 ) {
            ButtonBase* b = _buttonList[i];
            if ( b->state == BUTTON_ON ) {
                b->state = BUTTON_OFF;
            } else if ( b->state == BUTTON_OFF ) {
                b->state = BUTTON_ON;
            }
            return;
        }
    }
}
void ButtonBase::CursorPress(const glm::ivec2 coord)
{
    if ( !toggleable ) {
        state = BUTTON_ON;
    } else {
        if ( state == BUTTON_OFF ) {
            state = BUTTON_TOGGLING_ON;
        } else if ( state == BUTTON_ON ) {
            state = BUTTON_TOGGLING_OFF;
        }
    }
}

void ButtonBase::CursorRelease(const glm::ivec2 coord)
{
    
    if ( !toggleable && state == BUTTON_ON ) {
        state = BUTTON_OFF;
        // If a callback function has been set, call it.
        DoCallBack();
    } else {
        if ( state == BUTTON_TOGGLING_ON) {
            state = BUTTON_ON;
            DoCallBack();
        } else if ( state == BUTTON_TOGGLING_OFF) {
            state = BUTTON_OFF;
            DoCallBack();
        }
    }
}

void ButtonBase::CursorHover(const glm::ivec2 coord, bool highlight)
{
    if( highlighted && !highlight ) {
        // Mouse left button area
        if ( !toggleable || state == BUTTON_TOGGLING_ON ) {
            state = BUTTON_OFF;
        }
    }
    highlighted = highlight;
}

void ButtonBase::Draw( Renderer* renderer ) {
    if ( !visible ) return;
    if ( type == BUTTON_TYPE_DEFAULT ) {
        DrawDefaultButton( renderer );
    } else if (type == BUTTON_TYPE_NEON ) {
        DrawNeonButton( renderer );
    } else if (type == BUTTON_TYPE_SIMPLE ) {
        DrawSimpleButton( renderer );
    } else if (type == BUTTON_TYPE_TEXTURED ) {
        DrawTexturedButton( renderer );
    }
}

void ButtonBase::DrawDefaultButton( Renderer* renderer ) {
    if ( !visible ) return;
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(x,y+1), glm::vec2(x,y+h), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // L
    renderer->Buffer2DLine(glm::vec2(x,y+h), glm::vec2(x+w-1,y+h), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);   // T
    renderer->Buffer2DLine(glm::vec2(x+w,y+h), glm::vec2(x+w,y+1), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);   // R
    renderer->Buffer2DLine(glm::vec2(x+w-1,y), glm::vec2(x,y), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // B
    // Inner gradient fill
    renderer->DrawGradientY(Rect2D((float)x, (float)y+1, (float)w-1, (float)h-1), COLOR_UI_GRADIENT_TOP, COLOR_UI_GRADIENT_BOTTOM);
    // Inside border
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderer->Draw2DRect(Rect2D(x+1,y+1,w-2,h-2), COLOR_UI_BORDER_INNER, COLOR_NONE);
    renderer->Render2DLines();

    if ( state == BUTTON_ON ) {
        g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT_ACTIVE);
        g_uiMan->GetTextManager()->UpdateTextPos( labelID, glm::vec3(x+h/4.0, y+h/2.0-h*0.3*TEXT_RATIO, WIDGET_TEXT_DEPTH) );
    } else {
        if ( highlighted ) {
            g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT_HIGHLIGHT);
        } else {
            g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT);
        }
        g_uiMan->GetTextManager()->UpdateTextPos( labelID, glm::vec3(x+h/4.0, y+h/2.0-h*0.25*TEXT_RATIO, WIDGET_TEXT_DEPTH) );
    }
    
    if ( texture ) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Rect2D texRect;
        if ( state == BUTTON_ON && !frameActive.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( framePressed );
        } else if ( highlighted && !framePressed.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameActive );
        } else {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameDefault );
        }
		renderer->DrawTexture(Rect2D((float)x, (float)y, (float)w, (float)h), texRect, texture->GetID());
    }
}
void ButtonBase::DrawNeonButton( Renderer* renderer ) {
    if ( !visible ) return;
    int hF = h/4;

    if ( state ) {
        glColor4f(0.1f,0.4f,0.1f,1.0f);
    } else if ( highlighted ) {
        glColor4f(0.2f,0.4f,0.2f, 1.0f);
    } else {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    }
    /*
     *	draw background for the button.
     */
    GLint vertices[] = {
        x       , y,
        x+w-hF  , y,
        x+w     , y+hF,
        x+w     , y+h,
        x+hF    , y+h,
        x       , y+h-hF,
    };
    glVertexPointer(2, GL_INT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
    
    /*
     *	Draw an outline around the button with width 1
     */
    glLineWidth( 1.0 );
    if ( highlighted )
    { glColor4f(0.4f,1.0f,0.4f, 1.0f); }
    else
    { glColor4f(0.0f,0.8f,0.0f, 1.0f); }
    glDrawArrays(GL_LINE_LOOP, 0, 6);
    glLineWidth( 1.0f );

    /*
     *	If the cursor is currently over the button we offset the text string and draw a shadow
     */
    if( highlighted ) {
        g_uiMan->GetTextManager()->UpdateTextColor( labelID, COLOR_WHITE );
    } else {
        g_uiMan->GetTextManager()->UpdateTextColor( labelID, RGBAColor((GLfloat)0.8, (GLfloat)1.0, (GLfloat)0.8, (GLfloat)1.0) );
    }
}
void ButtonBase::DrawSimpleButton( Renderer* renderer ) {

    Color col = COLOR_WHITE;
    Color lineCol = COLOR_GREY;
    if ( state ) {
        col = RGBAColor(0.8f,0.8f,0.8f,1.0f);
        lineCol = RGBAColor(0.2f,0.2f,0.2f,1.0f);
    } else {
        if ( highlighted ) {
            col = RGBAColor(0.2f,0.2f,0.2f,1.0f);
            lineCol = RGBAColor(1.0f,1.0f,1.0f,1.0f);
        } else {
            col = RGBAColor(0.0f,0.0f,0.0f,1.0f);
            lineCol = RGBAColor(0.5f,0.5f,0.5f,1.0f);
        }
    }
	renderer->Draw2DRect(Rect2D((float)x, (float)y, (float)w, (float)h), lineCol, col, float(z));

    if ( state ) {
        g_uiMan->GetTextManager()->UpdateTextColor( labelID, COLOR_BLACK );
    } else {
        if( highlighted ) {
            g_uiMan->GetTextManager()->UpdateTextColor( labelID, COLOR_WHITE );
        } else {
            g_uiMan->GetTextManager()->UpdateTextColor( labelID, COLOR_GREY );
        }
    }
}

void ButtonBase::DrawTexturedButton( Renderer* renderer ) {
    
    if ( texture ) {
        Rect2D texRect;
        if ( state == BUTTON_ON && !frameActive.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameActive );
        } else if ( highlighted && !framePressed.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( framePressed );
        } else {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameDefault );
        }
		renderer->DrawTexture(Rect2D((float)x, (float)y, (float)w, (float)h), texRect, texture->GetID());
    } else {
        Color col;
        Color lineCol;
        if ( active ) {
            col = RGBAColor( 1.0f,1.0f,1.0f, 1.0f );
        } else {
            col = RGBAColor( 0.3f,0.3f,0.3f, 1.0f );
        }
        if ( highlighted ) {
            lineCol = RGBAColor(0.8f,0.8f,0.8f, 1.0f);
        } else {
            lineCol = RGBAColor(0.5f,0.5f,0.5f, 1.0f);
        }
        renderer->Draw2DRect(Rect2D((float)x, (float)y, (float)w, (float)h), lineCol, col);
    }
    if ( state ) {
        g_uiMan->GetTextManager()->UpdateTextColor( labelID, COLOR_WHITE );
    } else {
        if( highlighted ) {
            g_uiMan->GetTextManager()->UpdateTextColor( labelID, LAColor(0.6f, 1.0f) );
        } else {
            g_uiMan->GetTextManager()->UpdateTextColor( labelID, LAColor(0.4f, 1.0f) );
        }
    }
}

void ButtonBase::Update( void ) {
    if ( visible && labelID == -1 ) {
        // Add button text label to text manager
        int textRatio = (int)(h*TEXT_RATIO);
        int xSpacer = 1;
        int ySpacer = (int)(h*0.2);
        if ( textRatio < 15 ) {
            textRatio = 15;
        } 
        if ( type == BUTTON_TYPE_TEXTURED ) {
            textRatio = std::max<int>( (int)(w*0.08), (int)(h*0.4) );
            xSpacer = std::max<int>( (int)(w*0.1), (int)(h*0.2) ) ;
        }
        labelID = g_uiMan->GetTextManager()->AddText(label, glm::vec3(x+xSpacer, y+ySpacer, WIDGET_TEXT_DEPTH), true, textRatio, FONT_JURA );
    } else if ( !visible && labelID != -1 ) {
        // Remove button text label from manager
        g_uiMan->GetTextManager()->RemoveText(this->labelID);
        labelID = -1;
    }
}

void ButtonBase::UpdatePos(int posX, int posY) {
    x = posX; y = posY;
    int xSpacer = 1;
    int ySpacer = (int)(h*0.2);
    if ( type == BUTTON_TYPE_TEXTURED ) {
        xSpacer = std::max<int>( (int)(w*0.1), (int)(h*0.2) ) ;
    }
    if ( labelID != -1 ) {
        g_uiMan->GetTextManager()->UpdateTextPos(labelID, glm::vec3(x+xSpacer, y+ySpacer, WIDGET_TEXT_DEPTH)) ;
    }
}

void ButtonBase::UpdateLabel(const std::string newLabel) {
    if ( label != newLabel ) {
        label = newLabel;
        if ( labelID != -1 ) {
            g_uiMan->GetTextManager()->UpdateText(labelID, label);
        }
    }
}


