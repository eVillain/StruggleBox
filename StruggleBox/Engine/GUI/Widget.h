#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "GFXDefines.h"
#include "Transform.h"
#include <string>
#include <vector>

class Renderer;
class Texture;

///  Serves as a base class for all the user-interface widgets
///  such as buttons, sliders, text input, etc.
class Widget
{
    friend class GUI;
public:
    virtual void SetSize(const glm::ivec2& size);
    virtual const glm::ivec2& GetSize() const { return _size; };
    
    /// Active is true when the widget is 'on' and interactable (default = true)
    virtual void SetActive( const bool active ) { _active = active;} ;
    
    /* Override these for different cursor events */
    /// Focus is true when the cursor is over a widget
    virtual void SetFocus( const bool focus) { _focus = focus; };
    /// When clicked/pressed
    virtual void OnInteract( const bool interact, const glm::ivec2& coord ) { };
    /// When dragged with cursor, most widgets won't support this
    virtual void OnDrag( const glm::ivec2& coord ) { };
    /// Cursor over widget test, returns true if point is inside widget
    virtual const bool Contains( const glm::ivec2& coord ) const;
    
    // Override this for drawing different widgets
    virtual void Draw(Renderer* renderer);
    
    // Update - Unused for most widgets but some will need it
    virtual const void Update()
    { /* printf("[Widget] update called, override me!\n"); */ };
    
    Transform& GetTransform() { return _transform; };
    const bool IsActive() const { return _active; };
    const bool IsVisible() const { return _visible; };
    const bool HasFocus() const { return _focus; };
    
protected:
    Widget();
    virtual ~Widget();
    
    Transform _transform;
    glm::ivec2 _size;
    
    Texture * _texture;                          // NULL if not a textured widget
    std::string _frameDefault;
    std:: string _frameActive;
    std::string _framePressed;
    
    bool _active;                                // Whether UIWidget is active (interactable)
    bool _focus;                                 // Whether cursor is over widget
    bool _visible;                               // Whether we should render this widget
    
    bool moveable;                              // Whether widget can be moved
    bool dragging;                              // Moving widget by dragging menu bar
    int dragX, dragY;                           // Dragging amount
};

#endif /* GUI_WIDGET_H */
