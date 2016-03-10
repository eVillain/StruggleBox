#ifndef UI_MENU_ITEM_H
#define UI_MENU_ITEM_H

#include "UIWidget.h"
#include "TextManager.h"
#include "GFXHelpers.h"
#include "Renderer.h"

#define MENU_TEXT_SIZE 11

// TODO: have destructors of specialized classes call destructor for UIMenuItemBase

// Base class which defines default functionality for a menu item
template <typename T>
class UIMenuItemBase : public UIWidget {
protected:
    std::string m_label;    
    T* dataPointer;
    int labelID, varLabelID;
public:   
    UIMenuItemBase( int posX, int posY,
               int width, int height,
               std::string label, 
               T* variable )  :
    UIWidget(posX, posY, width, height) {
        m_label=label;
        dataPointer=variable;
        labelID = g_uiMan->GetTextManager()->AddText(m_label, glm::vec3(x+4, y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH),
                                       true, MENU_TEXT_SIZE, FONT_JURA );
        varLabelID=-1;
    };
    virtual ~UIMenuItemBase( void ) {
        if ( varLabelID != -1 ) {
            g_uiMan->GetTextManager()->RemoveText(varLabelID);
        }
        if ( labelID != -1 ) {
            g_uiMan->GetTextManager()->RemoveText(labelID);
        }
    };
    virtual void Update( void ) {
        if ( visible && labelID == -1 ) {
            labelID = g_uiMan->GetTextManager()->AddText(m_label, glm::vec3(x+4, y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH),
                                           true, MENU_TEXT_SIZE, FONT_JURA );
        } else if ( !visible && labelID != -1 ) {
            g_uiMan->GetTextManager()->RemoveText(labelID);
            labelID = -1;
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        x = posX; y = posY;
        if ( labelID != -1 ) {
            g_uiMan->GetTextManager()->UpdateTextPos( labelID, glm::vec3(x+4, y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH) );
        }
    };
    virtual void Draw( Renderer* renderer ) {
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
        renderer->Draw2DRect(Rect2D(x+1,y+1,w-2,h-2), COLOR_UI_BORDER_INNER, COLOR_NONE);
        renderer->Render2DLines();
    };
};

// Template class naming, not sure if this is the best way to go about this but it seems solid
template <typename T>
class UIMenuItem : public UIMenuItemBase<T> { };    // Specialize this class for each datatype

// Specialized classes for different data types
template<> class UIMenuItem<std::string> : public UIMenuItemBase<std::string> {
private:
public:
    UIMenuItem( int posX, int posY,
               int width, int height,
               std::string label, 
               std::string* variable ) :
    UIMenuItemBase(posX, posY, width, height, label, variable) { };

    void Update( void ) {
        UIMenuItemBase::Update();
        if ( visible && dataPointer != NULL ) {
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(*dataPointer, glm::vec3(x+w-((*dataPointer).length()*(MENU_TEXT_SIZE/2)),
                                                                          y+(MENU_TEXT_SIZE/2),
                                                                          WIDGET_TEXT_DEPTH),
                                                  true, MENU_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateText(varLabelID, *dataPointer);
            }
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UIMenuItemBase::UpdatePos(posX, posY);
        x = posX; y = posY;
        if ( varLabelID != -1 ) 
            g_uiMan->GetTextManager()->UpdateTextPos( varLabelID, glm::vec3(x+w-((*dataPointer).length()*(MENU_TEXT_SIZE/2)),
                                                          y+(MENU_TEXT_SIZE/2),
                                                          WIDGET_TEXT_DEPTH) );
    };
};
template<> class UIMenuItem<bool> : public UIMenuItemBase<bool> {
private:
public:
    UIMenuItem( int posX, int posY,
               int width, int height,
               std::string label, 
               bool* variable ) :
    UIMenuItemBase(posX, posY, width, height, label, variable) { };
    void Update( void ) {
        UIMenuItemBase::Update();
        if ( visible && dataPointer != NULL ) {
            std::string varLabel = ( *dataPointer == true ? "On" : "Off");
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*8.0f), y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH),
                                                  true, MENU_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        } else if ( varLabelID != -1 ) {
            g_uiMan->GetTextManager()->RemoveText(varLabelID);
            varLabelID = -1;
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UIMenuItemBase::UpdatePos(posX, posY);
        x = posX; y = posY;
        if ( varLabelID != -1 && dataPointer != NULL ) {
            std::string varLabel = ( *dataPointer == true ? "On" : "Off");
            g_uiMan->GetTextManager()->UpdateTextPos( varLabelID, glm::vec3(x+w-(varLabel.length()*8.0f), y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH) );
        }
    };
    virtual void CursorPress( int x, int y ) { *dataPointer = !*dataPointer; }; // Toggle boolean value by pressing
};
template<> class UIMenuItem<int> : public UIMenuItemBase<int> {
private:
public:
    UIMenuItem( int posX, int posY,
               int width, int height,
               std::string label, 
               int* variable ) :
    UIMenuItemBase(posX, posY, width, height, label, variable) { };
    void Update( void ) {
        UIMenuItemBase::Update();
        if ( visible && dataPointer != NULL ) {
            std::string varLabel = intToString( *dataPointer );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*8.0f), y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH),
                                                  true, MENU_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        } else if ( varLabelID != -1 ) {
            g_uiMan->GetTextManager()->RemoveText(varLabelID);
            varLabelID = -1;
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UIMenuItemBase::UpdatePos(posX, posY);
        x = posX; y = posY;
        if ( varLabelID != -1 && dataPointer != NULL ) {
            std::string varLabel = intToString( *dataPointer );
            g_uiMan->GetTextManager()->UpdateTextPos( varLabelID, glm::vec3(x+w-(varLabel.length()*8.0f), y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH) );
        }
    };
};
template<> class UIMenuItem<float> : public UIMenuItemBase<float> {
private:
public:
    UIMenuItem( int posX, int posY,
               int width, int height,
               std::string label, 
               float* variable ) :
    UIMenuItemBase(posX, posY, width, height, label, variable) { };
    void Update( void ) {
        UIMenuItemBase::Update();
        if ( visible && dataPointer != NULL ) {
            std::string varLabel = floatToString( *dataPointer );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*4.0f), y+(MENU_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                  true, MENU_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        } else if ( varLabelID != -1 ) {
            g_uiMan->GetTextManager()->RemoveText(varLabelID);
            varLabelID = -1;
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UIMenuItemBase::UpdatePos(posX, posY);
        x = posX; y = posY;
        if ( varLabelID != -1 && dataPointer != NULL ) {
            std::string varLabel = floatToString( *dataPointer );
            g_uiMan->GetTextManager()->UpdateTextPos( varLabelID, glm::vec3(x+w-(varLabel.length()*4.0f), y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH) );
        }
    };
};
template<> class UIMenuItem<double> : public UIMenuItemBase<double> {
private:
public:
    UIMenuItem( int posX, int posY,
               int width, int height,
               std::string label, 
               double* variable ) :
    UIMenuItemBase(posX, posY, width, height, label, variable) { };
    void Update( void ) {
        UIMenuItemBase::Update();
        if ( visible && dataPointer != NULL ) {
            std::string varLabel = doubleToString( *dataPointer );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*4), y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH),
                                                  true, MENU_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        } else if ( varLabelID != -1 ) {
            g_uiMan->GetTextManager()->RemoveText(varLabelID);
            varLabelID = -1;
        }
    };
    
};
template<> class UIMenuItem<Color> : public UIMenuItemBase<Color> {
    private:
public:
    UIMenuItem( int posX, int posY,
               int width, int height,
               std::string label, 
               Color* variable ) :
    UIMenuItemBase(posX, posY, width, height, label, variable) { };
    void Update( void ) {
        UIMenuItemBase::Update();
        if ( visible && dataPointer != NULL ) {
            std::string varLabel = floatToString( (*dataPointer).r );
            varLabel.append(", ");
            varLabel.append( floatToString( (*dataPointer).g ) );
            varLabel.append(", ");
            varLabel.append( floatToString( (*dataPointer).b ) );
            varLabel.append(", ");
            varLabel.append( floatToString( (*dataPointer).a ) );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*4), y+(MENU_TEXT_SIZE/2), WIDGET_TEXT_DEPTH),
                                                  true, MENU_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        } else if ( varLabelID != -1 ) {
            g_uiMan->GetTextManager()->RemoveText(varLabelID);
            varLabelID = -1;
        }
    };
};

#endif /* UI_MENU_ITEM_H */
