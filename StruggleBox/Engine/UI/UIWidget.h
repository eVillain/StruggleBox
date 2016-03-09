#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <string>
#include <vector>
#include "UIManager.h"
#include "GFXIncludes.h"
#include "Texture.h"

/// Serves as a base class for all the user-interface widgets
/// (buttons, sliders, text input boxes etc.)
class UIWidget
{
public:
    static void SetUIManager( UIManager* uiMan ) { g_uiMan = uiMan; };
    
    // UIWidget attributes
    int x,y,w,h,z;
    int contentHeight;
    bool highlighted;                           // Whether cursor is over widget
    bool active;                                // Whether UIWidget is active (clicked)
    bool visible;                               // Whether we should render this widget
    
    UIWidget(int posX, int posY,
             int width, int height,
             int depth = 1,
             std::string texDefault = "",       // Must specify at least default texture if textured
             std::string texActive = "",
             std::string texPressed = "" );
    virtual ~UIWidget( void );
    
    virtual void UpdatePos( int posX, int posY );
    virtual void UpdateSize( int width, int height );
    
    // Cursor over widget test
    virtual bool PointTest(const glm::ivec2 coord);
    
    // Override these for different cursor events
    virtual void CursorHover(const glm::ivec2 coord, bool highlight) { highlighted = highlight; };
    virtual void CursorPress(const glm::ivec2 coord) { active = true;} ;
    virtual void CursorRelease(const glm::ivec2 coord) { active = false; };
    
    // Override these for drawing different widgets
    virtual void Draw( Renderer* renderer );    
    virtual void Update( void ) { printf("[Widget] update called, override\n"); }; // Unused for most widgets, menu items need it though

    inline bool IsActive( void ) { return active; };
    
protected:
    // UIWidget parameters
    Texture * texture;                          // NULL if not a textured widget
    std::string frameDefault;
    std:: string frameActive;
    std::string framePressed;
    
    static UIManager* g_uiMan;                  // Global pointer to UIManager instance
    
    bool moveable;                              // Whether widget can be moved
    bool dragging;                              // Moving widget by dragging menu bar
    int dragX, dragY;                           // Dragging amount
    
    bool minimizeable;                          // Whether widget can be minimized
    bool minimized;                             // Widget is currently minimized
};

#endif /* UI_WIDGET_H */
