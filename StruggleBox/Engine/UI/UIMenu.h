#ifndef UI_MENU_H
#define UI_MENU_H

#include <string>
#include <vector>
#include <map>
#include "CoreTypes.h"
#include "UIWidget.h"
#include "UIMenuItem.h"
#include "UISlider.h"
#include "UIButton.h"

class UIMenu : public UIWidget
{
public:
    int                             contentHeight;
    bool                            minimized;

    UIMenu( int posX, int posY,
            int width, int height,
            std::string title="Unnamed Menu",
            UIMenu* parentMenu=NULL,
            std::string texDefault = "",
            std::string texActive = "",
            std::string texPressed = "" );
    virtual ~UIMenu( void );
    
    template <typename T> void AddVar( std::string label, T* data, std::string submenu = "" ) {
        if ( submenu.empty() ) {
            int vH = h-4;
            int posY = y+contentHeight;
            posY -= ( (int)menuItemList.size() * vH );
            UIMenuItem<T>* varItem = new UIMenuItem<T>(x+4, posY-vH, w-8, vH, label, data);
            menuItemList.push_back(varItem);
            if ( !minimized ) {
                y -= varItem->h;
                contentHeight += varItem->h;
                if ( parent ) {
                    parent->Sort();
                }
            } else {
                varItem->visible = false;
            }
        } else {
            // Add to submenu
            std::map<std::string, UIMenu*>::iterator it = subMenus.find(submenu);
            if ( it == subMenus.end() ) {
                AddSubMenu(submenu);
            }
            subMenus[submenu]->AddVar(label, data);
        }
    };
    template <typename T> void AddSlider( std::string label, T* data, T minVal, T maxVal, std::string submenu = "" ) {
        if ( submenu.empty() ) {
            int vH = h-4;
            int posY = y+contentHeight;
            posY -= ( (int)menuItemList.size() * vH );
            UISlider<T>* varItem = new UISlider<T>( x+4, posY-vH, w-8, vH, 5.0, label, data, minVal, maxVal );
            menuItemList.push_back(varItem);
            if ( !minimized ) {
                y -= varItem->h;
                contentHeight += varItem->h;
                if ( parent ) {
                    parent->Sort();
                }
            } else {
                varItem->visible = false;
            }
        } else {
            // Add to submenu
            std::map<std::string, UIMenu*>::iterator it = subMenus.find(submenu);
            if ( it == subMenus.end() ) {
                AddSubMenu(submenu);
            }
            subMenus[submenu]->AddSlider(label, data, minVal, maxVal);
        }
    };
    template <typename UnknownClass> void AddSliderOptF( std::string name, std::string label,
                                                       UnknownClass* object,
                                                       void( UnknownClass::*setFunc )( const std::string&, const float ),
                                                       float( UnknownClass::*getFunc )( const std::string& ),
                                                       float minVal, float maxVal, std::string submenu = "" ) {
        if ( submenu.empty() ) {
            int vH = h-4;
            int posY = y+contentHeight;
            posY -= ( (int)menuItemList.size() * vH );
            
            UISliderOptionFloat<UnknownClass>* varItem = new UISliderOptionFloat<UnknownClass>( x+4, posY-vH, w-8, vH, 5.0,
                                                                                                name, label,
                                                                                                minVal, maxVal,
                                                                                                object, setFunc, getFunc );
            menuItemList.push_back(varItem);
            if ( !minimized ) {
                y -= varItem->h;
                contentHeight += varItem->h;
                if ( parent ) {
                    parent->Sort();
                }
            } else {
                varItem->visible = false;
            }
        } else {
            // Add to submenu
            std::map<std::string, UIMenu*>::iterator it = subMenus.find(submenu);
            if ( it == subMenus.end() ) {
                AddSubMenu(submenu);
            }
            subMenus[submenu]->AddSliderOptF(name, label,
                                         object, setFunc, getFunc,
                                         minVal, maxVal);
        }
    };
    template <typename UnknownClass> void AddSliderOptI( std::string name, std::string label,
                                                        UnknownClass* object,
                                                        void( UnknownClass::*setFunc )( const std::string&, const int ),
                                                        int( UnknownClass::*getFunc )( const std::string& ),
                                                        int minVal, int maxVal, std::string submenu = "" ) {
        if ( submenu.empty() ) {
            int vH = h-4;
            int posY = y+contentHeight;
            posY -= ( (int)menuItemList.size() * vH );
            
            UISliderOptionInt<UnknownClass>* varItem = new UISliderOptionInt<UnknownClass>( x+4, posY-vH, w-8, vH, 5.0,
                                                                                               name, label,
                                                                                               minVal, maxVal,
                                                                                               object, setFunc, getFunc );
            menuItemList.push_back(varItem);
            if ( !minimized ) {
                y -= varItem->h;
                contentHeight += varItem->h;
                if ( parent ) {
                    parent->Sort();
                }
            } else {
                varItem->visible = false;
            }
        } else {
            // Add to submenu
            std::map<std::string, UIMenu*>::iterator it = subMenus.find(submenu);
            if ( it == subMenus.end() ) {
                AddSubMenu(submenu);
            }
            subMenus[submenu]->AddSliderOptI(name, label,
                                         object, setFunc, getFunc,
                                         minVal, maxVal);
        }
    };
    void AddButton( std::string label, std::function<void()> function, const std::string submenu = "", const bool toggleable = false,
                   const std::string frameDefault = "", const std::string frameActive = "", const std::string framePressed = "") {
        if ( submenu.empty() ) {
            int vH = h-4;
            int posY = y+contentHeight;
            posY -= ( (int)menuItemList.size() * vH );
            UIButtonLambda* button = UIButtonLambda::CreateButton(label, x+4, posY-vH, w-8, vH,
                                                                  function, BUTTON_TYPE_DEFAULT, toggleable,
                                                                  frameDefault, frameActive, framePressed );
            menuItemList.push_back((UIWidget*)button);
            if ( !minimized ) {
                y -= ((UIWidget*)button)->h;
                contentHeight += ((UIWidget*)button)->h;
                if ( parent ) {
                    parent->Sort();
                }
            } else {
                ((UIWidget*)button)->visible = false;
            }
        } else {
            std::map<std::string, UIMenu*>::iterator it = subMenus.find(submenu);
            if ( it == subMenus.end() ) {
                AddSubMenu(submenu);
            }
            subMenus[submenu]->AddButton(label, function);
        }
    };
    void AddSubMenu( std::string menuName ) {
        int vH = h-4;
        int posY = y+contentHeight;
        posY -= ( (int)menuItemList.size() * vH );
        UIMenu* subMenu = new UIMenu(x+4, posY-h, w-8, h, menuName, this);
        subMenus[menuName] = subMenu;
        contentHeight += h;
        y -= h;
        subMenu->minimized = true;
    }
    void AddWidget( UIWidget *widget );
    void RemoveWidget( UIWidget* widget );
    void ClearWidgets( void );

    virtual void UpdatePos( int posX, int posY );
    virtual void UpdateSize( int width, int height );
    // Cursor over widget tests
    bool PointTest(const glm::ivec2 coord);
    // Test cursor events on widgets
    bool CheckRelease(const glm::ivec2 coord);
    bool CheckPress(const glm::ivec2 coord);
    bool CheckHover(const glm::ivec2 coord, bool highlight);
    
    // Override these for different cursor events
    virtual void CursorHover(const glm::ivec2 coord, bool highlight);
    virtual void CursorPress(const glm::ivec2 coord);
    virtual void CursorRelease(const glm::ivec2 coord);
        
    virtual void Draw( Renderer* renderer );
    
    virtual void Update();
    void Minimize( void* unused );
    void Sort( void );
    void Hide( void );
    void Show( void );
    
protected:
    std::string                     label;          // Text for menu label
    int                             labelID;        // Text ID for menu label
    UIMenu*                         parent;         // Parent menu if this is a submenu
    std::vector<UIWidget*>          menuItemList;   // Internal widget list
    std::map<std::string, UIMenu*>  subMenus;       // Internal submenu list
    ButtonBase*                     minimizeBtn;    // Minimize button
    
    int                             maxItems;       // Maximum items in list
    int                             scrollPos;      // Scrolling position in list
};

#endif */ UI_MENU_H */
