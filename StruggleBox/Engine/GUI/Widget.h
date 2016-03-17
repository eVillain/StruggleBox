#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "GFXDefines.h"
#include "Transform.h"
#include <string>
#include <vector>

class Locator;
class Renderer;
class Texture;

///  Serves as a base class for all the user-interface widgets
///  such as buttons, sliders, text input, etc.
class Widget
{
    friend class GUI;
public:
    virtual void setSize(const glm::ivec2& size);
    virtual const glm::ivec2& getSize() const { return _size; }
    
    /// Active is true when the widget is 'on' and interactable (default = true)
    virtual void setActive( const bool active ) { _active = active;}
    
    /// Toggles rendering of widget
    virtual void setVisibility(const bool visible) { _visible = visible; }

    Transform& GetTransform() { return _transform; };
    const bool active() const { return _active; };
    const bool visible() const { return _visible; };
    const bool focus() const { return _focus; };
    
protected:
    Widget(Locator& locator);
    virtual ~Widget();
    
    // Override this for drawing different widgets
    virtual void Draw(Renderer* renderer);
    void DrawBase(Renderer* renderer,
                  const glm::vec3& position,
                  const glm::ivec2& size);
    
    // Update - Unused for most widgets but some will need it
    virtual void Update(const double deltaTime)
    { /* printf("[Widget] update called, override me!\n"); */ }
    
    /// When clicked/pressed
    virtual void OnInteract(const bool interact,
                            const glm::ivec2& coord) { };
    /// When dragged with cursor, most widgets won't support this
    virtual void OnDrag(const glm::ivec2& coord) { };
    /// Cursor over widget test, returns true if point is inside widget
    virtual const bool Contains(const glm::ivec2& coord) const;
    /// Focus is true when the cursor is over a widget
    virtual void setFocus(const bool focus) { _focus = focus; };

    Locator& _locator;
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
