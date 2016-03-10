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
    if ( _behavior )
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
                           COLOR_UI_BORDER_OUTER);   // L
    renderer->Buffer2DLine(glm::vec2(drawPos.x,drawPos.y+_size.y),
                           glm::vec2(drawPos.x+_size.x,drawPos.y+_size.y),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER);   // T
    renderer->Buffer2DLine(glm::vec2(drawPos.x+_size.x+1,drawPos.y+_size.y),
                           glm::vec2(drawPos.x+_size.x+1,drawPos.y),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER);   // R
    renderer->Buffer2DLine(glm::vec2(drawPos.x+_size.x,drawPos.y-1),
                           glm::vec2(drawPos.x,drawPos.y-1),
                           COLOR_UI_BORDER_OUTER,
                           COLOR_UI_BORDER_OUTER);   // B
    // Inner gradient fill
    Color gradColTop = COLOR_UI_GRADIENT_TOP;
    Color gradColBottom = COLOR_UI_GRADIENT_BOTTOM;
    if ( _focus)
    {
        if ( _pressed )
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
                            COLOR_UI_GRADIENT_TOP,
                            COLOR_UI_GRADIENT_BOTTOM);
    // Inside border
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderer->Draw2DRect(Rect2D(drawPos.x+1, drawPos.y+1, _size.x-2, _size.y-2),
                         COLOR_UI_BORDER_INNER,
                         COLOR_NONE);
    renderer->Render2DLines();
    
//    if ( state == BUTTON_ON ) {
//        g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT_ACTIVE);
//        g_uiMan->GetTextManager()->UpdateTextPos( labelID, glm::vec3(x+h/4.0, y+h/2.0-h*0.3*TEXT_RATIO, WIDGET_TEXT_DEPTH) );
//    } else {
//        if ( highlighted ) {
//            g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT_HIGHLIGHT);
//        } else {
//            g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT);
//        }
//        g_uiMan->GetTextManager()->UpdateTextPos( labelID, glm::vec3(x+h/4.0, y+h/2.0-h*0.25*TEXT_RATIO, WIDGET_TEXT_DEPTH) );
//    }

    //        // Pixel perfect outer border (should render with 1px shaved off corners)
    //        primitives.Line(glm::vec2(drawPos.x,drawPos.y),
    //                        glm::vec2(drawPos.x,drawPos.y+_size.y),
    //                        COLOR_UI_BORDER_OUTER,
    //                        COLOR_UI_BORDER_OUTER,
    //                        _transform.GetPosition().z);  // L
    //        primitives.Line(glm::vec2(drawPos.x,drawPos.y+_size.y),
    //                        glm::vec2(drawPos.x+_size.x,drawPos.y+_size.y),
    //                        COLOR_UI_BORDER_OUTER,
    //                        COLOR_UI_BORDER_OUTER,
    //                        _transform.GetPosition().z);  // T
//            primitives.Line(glm::vec2(drawPos.x+_size.x+1,drawPos.y+_size.y),
//                            glm::vec2(drawPos.x+_size.x+1,drawPos.y),
    //                        COLOR_UI_BORDER_OUTER,
    //                        COLOR_UI_BORDER_OUTER,
    //                        _transform.GetPosition().z);  // R
    //        primitives.Line(glm::vec2(drawPos.x+_size.x,drawPos.y-1),
    //                        glm::vec2(drawPos.x,drawPos.y-1),
    //                        COLOR_UI_BORDER_OUTER,
    //                        COLOR_UI_BORDER_OUTER,
    //                        _transform.GetPosition().z);  // B
    //
//            // Inner gradient fill
//            Color gradColTop = COLOR_UI_GRADIENT_TOP;
//            Color gradColBottom = COLOR_UI_GRADIENT_BOTTOM;
//            if ( _focus)
//            {
//                if ( _pressed )
//                {
//                    gradColTop *= 0.9;
//                    gradColBottom *= 0.9;
//                }
//                else
//                {
//                    gradColTop *= 1.1;
//                    gradColBottom *= 1.1;
//                }
//            }
    //        primitives.RectangleGradientY(glm::vec2(_transform.GetPosition().x,_transform.GetPosition().y),
    //                                      glm::vec2(_size.x,_size.y),
    //                                      gradColTop, gradColBottom,
    //                                      _transform.GetPosition().z);
    //
    //        // Inside border
    //        glEnable(GL_BLEND);
    //        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //        primitives.RectOutline(glm::vec2(_transform.GetPosition().x,_transform.GetPosition().y),
    //                               glm::vec2(_size.x-2,_size.y-2),
    //                               COLOR_UI_BORDER_INNER,
    //                               _transform.GetPosition().z+1);
}

const void Button::Update()
{ }

// When clicked/pressed
void Button::OnInteract( const bool interact, const glm::ivec2& coord )
{
    if ( _pressed && !interact && _focus)
    {
        // Pushed and released while in focus, trigger
        if ( _behavior )
        {
            _behavior->Trigger();
        }
    }
    _pressed = interact;
}
