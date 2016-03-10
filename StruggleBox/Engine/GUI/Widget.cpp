#include "Widget.h"
#include "GUI.h"
#include "Locator.h"
#include "Renderer.h"
#include "Shader.h"
//#include "Primitives2D.h"

    Widget::Widget() :
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
    
    void Widget::SetSize(const glm::ivec2& size)
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

//        Rect2D rect = Rect2D(x,y,w,h);
//        if ( _texture ) {
//            Rect2D texRect;
//            if ( _active && !_frameActive.empty() ) {
//                texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameActive );
//            } else if ( highlighted && !_framePressed.empty() ) {
//                texRect = g_uiMan->GetBatch()->GetTexRectForFrame( framePressed );
//            } else {
//                texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameDefault );
//            }
//            //        const GLint texVerts[] = {
//            //            (GLint)texRect.x                , (GLint)(texRect.y - texRect.h),
//            //            (GLint)(texRect.x + texRect.w)  , (GLint)(texRect.y - texRect.h),
//            //            (GLint)(texRect.x + texRect.w)  , (GLint)texRect.y,
//            //            (GLint)texRect.x                , (GLint)texRect.y
//            //        };
//            renderer->DrawTexture( rect, texRect, texture->GetID() );
//        } else {
//            Color col = RGBAColor(0.6f,0.6f,0.6f, 1.0f);;
//            Color lineCol = COLOR_GREY;
//            if ( active ) {
//                col = RGBAColor(0.3f,0.3f,0.3f,1.0f);
//                lineCol = RGBAColor(0.2f,0.2f,0.2f, 1.0f);
//            }
//            renderer->Draw2DRect(rect, lineCol, col);
//        }
//
        glm::ivec2 drawPos = glm::ivec2(_transform.GetPosition().x-(_size.x*0.5), _transform.GetPosition().y-(_size.y*0.5));
//
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
//        primitives.Line(glm::vec2(drawPos.x+_size.x+1,drawPos.y+_size.y),
//                        glm::vec2(drawPos.x+_size.x+1,drawPos.y),
//                        COLOR_UI_BORDER_OUTER,
//                        COLOR_UI_BORDER_OUTER,
//                        _transform.GetPosition().z);  // R
//        primitives.Line(glm::vec2(drawPos.x+_size.x,drawPos.y-1),
//                        glm::vec2(drawPos.x,drawPos.y-1),
//                        COLOR_UI_BORDER_OUTER,
//                        COLOR_UI_BORDER_OUTER,
//                        _transform.GetPosition().z);  // B
        
        // Inner gradient fill
//        Color gradColTop = COLOR_UI_GRADIENT_TOP;
//        Color gradColBottom = COLOR_UI_GRADIENT_BOTTOM;
//        if ( !_active)
//        {
//            gradColTop *= 0.9;
//            gradColBottom *= 0.9;
//        }
//        else if (_focus)
//        {
//            gradColTop *= 1.1;
//            gradColBottom *= 1.1;
//        }
//        
//        primitives.RectangleGradientY(glm::vec2(_transform.GetPosition().x,_transform.GetPosition().y),
//                                      glm::vec2(_size.x,_size.y),
//                                      gradColTop,
//                                      gradColBottom,
//                                      _transform.GetPosition().z);
    }
