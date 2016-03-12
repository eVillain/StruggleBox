#include "Button.h"
#include "Locator.h"
#include "Renderer.h"
#include "Color.h"

Button::Button()
{
    _pressed = false;
    _behavior = nullptr;
}

Button::~Button()
{
    if (_behavior)
    {
        delete _behavior;
        _behavior = nullptr;
    }
}

void Button::Draw(Renderer* renderer)
{
    if ( !_visible ) return;
    
    glm::ivec2 drawPos = glm::ivec2(_transform.GetPosition().x-(_size.x*0.5),
                                    _transform.GetPosition().y-(_size.y*0.5));
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(drawPos.x,drawPos.y),
                           glm::vec2(drawPos.x,drawPos.y+_size.y),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER);   // Left
    renderer->Buffer2DLine(glm::vec2(drawPos.x,drawPos.y+_size.y),
                           glm::vec2(drawPos.x+_size.x,drawPos.y+_size.y),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER);   // Top
    renderer->Buffer2DLine(glm::vec2(drawPos.x+_size.x+1,drawPos.y+_size.y),
                           glm::vec2(drawPos.x+_size.x+1,drawPos.y),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER);   // Right
    renderer->Buffer2DLine(glm::vec2(drawPos.x+_size.x,drawPos.y-1),
                           glm::vec2(drawPos.x,drawPos.y-1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER);   // Bottom
    // Inner gradient fill
    Color gradColTop = COLOR_UI_GRADIENT_TOP;
    Color gradColBottom = COLOR_UI_GRADIENT_BOTTOM;
    
    if (_focus)
    {
        if (_pressed)
        {
            gradColTop *= 0.9;
            gradColBottom *= 0.9;
        }
        else
        {
            gradColTop *= 1.1;
            gradColBottom *= 1.1;
        }
    }
    renderer->DrawGradientY(Rect2D(drawPos.x, drawPos.y+1, _size.x-1, _size.y-1),
                            gradColTop,
                            gradColBottom);
    // Inside border
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderer->Draw2DRect(Rect2D(drawPos.x+1, drawPos.y+1, _size.x-2, _size.y-2),
                         COLOR_UI_BORDER_INNER,
                         COLOR_NONE);
    renderer->Render2DLines();
}

void Button::OnDrag(const glm::ivec2& coord)
{
    if (_pressed) { _focus = true; }
}

void Button::OnInteract(const bool interact,
                        const glm::ivec2& coord)
{
    if (_pressed &&
        !interact &&
        _focus)
    {
        // Pushed and released while in focus, trigger behavior
        if (_behavior) { _behavior->Trigger(); }
    }
    _pressed = interact;
}
