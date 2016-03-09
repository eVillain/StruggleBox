#include "UIWidget.h"
#include "UIManager.h"

#include "GFXDefines.h"
#include "TextManager.h"
#include "SpriteBatch.h"
#include "Texture.h"
#include "SysCore.h"
#include "Rect2D.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "Renderer.h"
#include "Options.h"

UIManager* UIWidget::g_uiMan = NULL;   // Global pointer to UIManager instance

UIWidget::UIWidget( int posX, int posY,
                   int width, int height,
                   int depth,
                   std::string texDefault,
                   std::string texActive,
                   std::string texPressed ) :
x(posX), y(posY),
w(width), h(height),
z(depth),
highlighted(false),
active(false),
visible(true) {
    if ( !texDefault.empty() ) {
        const SpriteBatch* batch = g_uiMan->GetBatch();
        if ( !batch ) {
            printf("[UIWidget] BATCH FAIL!");
        } else {
            texture = batch->texture;
        }
        if ( g_uiMan->GetBatch()->GetIDForFrame(texDefault) == -1 ) {
            printf("couldnt get id for frame %s\n", texDefault.c_str() );
            texture = NULL;
        } else {
            // Set frames for widget
            frameDefault = texDefault;
            if ( !texActive.empty() ) {
                frameActive = texActive;
            }
            if ( !texPressed.empty() ) {
                framePressed = texPressed;
            }
        }
    } else {
        texture = NULL;
    }
    moveable=false;
    dragging=false;
    minimizeable=false;
    minimized=false;
    dragX=0;
    dragY=0;
    contentHeight=0;
}
UIWidget::~UIWidget( void )
{  }

void UIWidget::UpdatePos( int posX, int posY ) {
    x = posX; y = posY;
}
void UIWidget::UpdateSize( int width, int height ) {
    w = width; h = height;
}

// If point is within button area returns true
bool UIWidget::PointTest(const glm::ivec2 coord)
{
    if ( !visible ) return false;
    // If point is within button area, then returns true
    int vH = h + contentHeight;
    if(coord.x > x  &&
       coord.x < x+w &&
       coord.y > y-1   &&
       coord.y < y+vH) {
        return true;
	}
	return false;
}

void UIWidget::Draw( Renderer* renderer ) {
    if ( !visible ) return;
    Rect2D rect = Rect2D(x,y,w,h);
    if ( texture ) {
        Rect2D texRect;
        if ( active && !frameActive.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameActive );
        } else if ( highlighted && !framePressed.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( framePressed );
        } else {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameDefault );
        }
//        const GLint texVerts[] = {
//            (GLint)texRect.x                , (GLint)(texRect.y - texRect.h),
//            (GLint)(texRect.x + texRect.w)  , (GLint)(texRect.y - texRect.h),
//            (GLint)(texRect.x + texRect.w)  , (GLint)texRect.y,
//            (GLint)texRect.x                , (GLint)texRect.y
//        };
        renderer->DrawTexture( rect, texRect, texture->GetID() );
    } else {
        Color col = RGBAColor(0.6f,0.6f,0.6f, 1.0f);;
        Color lineCol = COLOR_GREY;
        if ( active ) {
            col = RGBAColor(0.3f,0.3f,0.3f,1.0f);
            lineCol = RGBAColor(0.2f,0.2f,0.2f, 1.0f);
        }
        renderer->Draw2DRect(rect, lineCol, col);
    }
}
