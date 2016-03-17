#include "Widget.h"
#include "GUI.h"
#include "Locator.h"
#include "Renderer.h"
#include "Shader.h"

Widget::Widget(Locator& locator) :
_locator(locator),
_size(),
_active(true),
_visible(true),
_focus(false),
moveable(false),
dragging(false),
dragX(0),
dragY(0)
{}

Widget::~Widget( void )
{}

void Widget::setSize(const glm::ivec2& size)
{
    _size = size;
}

// If point is within widget area returns true
const bool Widget::Contains( const glm::ivec2& coord ) const
{
    if ( !_visible ) return false;
    // If point is within button area, then returns true
    int vH = _size.y*0.5;
    int vW = _size.x*0.5;
    if(coord.x > _transform.GetPosition().x-vW &&
       coord.x < _transform.GetPosition().x+vW &&
       coord.y > _transform.GetPosition().y-(vH-1) &&    // For some reason this is offset by 1px, check later
       coord.y < _transform.GetPosition().y+vH+1)
    {
        return true;
    }
    return false;
}

void Widget::Draw(Renderer* renderer)
{
    if ( !_visible ) return;
    
    glm::ivec2 drawPos = glm::ivec2(_transform.GetPosition().x-(_size.x*0.5),
                                    _transform.GetPosition().y-(_size.y*0.5));
    float z = _transform.GetPosition().z;
    
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(drawPos.x, drawPos.y+1),
                           glm::vec2(drawPos.x, drawPos.y+_size.y-1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           z);   // Left
    renderer->Buffer2DLine(glm::vec2(drawPos.x+_size.x-1, drawPos.y+_size.y-1),
                           glm::vec2(drawPos.x+_size.x-1, drawPos.y+1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           z);   // Right
    renderer->Buffer2DLine(glm::vec2(drawPos.x, drawPos.y+_size.y-1),
                           glm::vec2(drawPos.x+_size.x-2, drawPos.y+_size.y-1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           z);   // Top
    renderer->Buffer2DLine(glm::vec2(drawPos.x, drawPos.y),
                           glm::vec2(drawPos.x+_size.x-2, drawPos.y),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           z);   // Bottom

    // Inner gradient fill
    Color gradColTop = COLOR_UI_GRADIENT_TOP;
    Color gradColBottom = COLOR_UI_GRADIENT_BOTTOM;
    if ( !_active)
    {
        gradColTop *= 0.8;
        gradColBottom *= 0.8;
    }
    else if (_focus)
    {
        gradColTop *= 1.1;
        gradColBottom *= 1.1;
    }

    renderer->DrawGradientY(Rect2D(drawPos.x, drawPos.y+1,
                                   _size.x-1, _size.y-1),
                            gradColTop,
                            gradColBottom,
                            z);
    // Inside border
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderer->Draw2DRect(Rect2D(drawPos.x+1, drawPos.y+1,
                                _size.x-2, _size.y-2),
                         COLOR_UI_BORDER_INNER,
                         COLOR_NONE,
                         z);
}

void Widget::DrawBase(Renderer* renderer,
                      const glm::vec3 &position,
                      const glm::ivec2 &size)
{
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(position.x, position.y+1),
                           glm::vec2(position.x, position.y+size.y-1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           position.z);   // Left
    renderer->Buffer2DLine(glm::vec2(position.x+_size.x-1, position.y+size.y-1),
                           glm::vec2(position.x+_size.x-1, position.y+1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           position.z);   // Right
    renderer->Buffer2DLine(glm::vec2(position.x, position.y),
                           glm::vec2(position.x+_size.x-2, position.y),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           position.z);   // Top
    renderer->Buffer2DLine(glm::vec2(position.x, position.y+size.y-1),
                           glm::vec2(position.x+_size.x-2, position.y+size.y-1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER,
                           position.z);   // Bottom
    
    renderer->DrawGradientY(Rect2D(position.x, position.y+1, size.x-1, size.y-1),
                            COLOR_UI_GRADIENT_TOP,
                            COLOR_UI_GRADIENT_BOTTOM,
                            position.z);
    
    // Inside border
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderer->Draw2DRect(Rect2D(position.x+1, position.y+1, size.x-2, size.y-2),
                         COLOR_UI_BORDER_INNER,
                         COLOR_NONE,
                         position.z);
}
