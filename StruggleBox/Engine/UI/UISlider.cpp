#include "UISlider.h"
#include "Shader.h"
#include "Texture.h"
#include "Renderer.h"

UISliderBase::UISliderBase(int posX, int posY,
                           int width, int height,
                           int depth,
                           std::string label,
                           std::string texDefault,
                           std::string texActive,
                           std::string texPressed) :
UIWidget(posX, posY, width, height, depth,
         texDefault, texActive, texPressed )
{
    m_label = label;
    
    glm::vec3 labelPos = glm::vec3(x+4.0f, y+(SLIDER_TEXT_SIZE/2), WIDGET_TEXT_DEPTH);
    
    labelID = g_uiMan->GetTextManager()->AddText(m_label,
                                                 labelPos,
                                                 true,
                                                 SLIDER_TEXT_SIZE,
                                                 FONT_JURA);
    varLabelID=-1;
    sliderPos = x+width/2;
    dragging = false;
    // Add each slider to list on creation
    g_uiMan->AddWidget(this);
}

UISliderBase::~UISliderBase()
{
    g_uiMan->RemoveWidget(this);
    
    if ( varLabelID != -1 ) {
        g_uiMan->GetTextManager()->RemoveText(varLabelID);
    }
    if ( labelID != -1 ) {
        g_uiMan->GetTextManager()->RemoveText(labelID);
    }
}

bool UISliderBase::SliderTest(const glm::ivec2 coord)
{
    int hF = 5;
    // If point is within slider area, then returns true
    if(coord.x > sliderPos-hF  &&
       coord.x < sliderPos+hF &&
       coord.y > y-1   &&
       coord.y < y+h ) {
        return true;
	}
	return false;
}

void UISliderBase::CursorPress(const glm::ivec2 coord)
{
    if ( SliderTest(coord) ) {
        dragging = true;
    } else {
        sliderPos = coord.x;
        DoCallBack();        // Refresh value
        dragging = true;
    }
}

void UISliderBase::CursorRelease(const glm::ivec2 coord)
{
    dragging = false;
}

void UISliderBase::CursorHover(const glm::ivec2 coord, bool highlight)
{
    if( highlighted && !highlight ) {        // Mouse left button area
        if ( dragging ) dragging = false;
    } else if ( dragging && highlight ) {
        sliderPos = coord.x;
        DoCallBack();        // Refresh value
    }
    highlighted = highlight;
}

void UISliderBase::Draw( Renderer* renderer ) {
    if ( !visible ) return;
    
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(x,y+1), glm::vec2(x,y+h), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // L
    renderer->Buffer2DLine(glm::vec2(x,y+h), glm::vec2(x+w-1,y+h), COLOR_UI_BORDER1, COLOR_UI_BORDER1);   // T
    renderer->Buffer2DLine(glm::vec2(x+w,y+h), glm::vec2(x+w,y+1), COLOR_UI_BORDER1, COLOR_UI_BORDER1);   // R
    renderer->Buffer2DLine(glm::vec2(x+w-1,y), glm::vec2(x,y), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // B
    renderer->Buffer2DLine(glm::vec2(x+w-1,y), glm::vec2(x,y), COLOR_UI_BORDER1, COLOR_UI_BORDER1);       // B
    // Inner gradient fill
    renderer->DrawGradientY(Rect2D((float)x, (float)y+1, (float)w-1, (float)h-1), COLOR_UI_GRADIENT1, COLOR_UI_GRADIENT2);
    // Inside border
    glEnable(GL_BLEND);
    renderer->Draw2DRect(Rect2D(x+1,y+1,w-2,h-2), COLOR_UI_BORDER2, COLOR_NONE);
    renderer->Render2DLines();

    if ( texture ) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Rect2D texRect;
        if ( active && !frameActive.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameActive );
        } else if ( highlighted && !framePressed.empty() ) {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( framePressed );
        } else {
            texRect = g_uiMan->GetBatch()->GetTexRectForFrame( frameDefault );
        }
		renderer->DrawTexture(Rect2D((float)x, (float)y, (float)w, (float)h), texRect, texture->GetID());
    }
    // Slider pointer
    float hF = 3.5f;
    float pad = 2.0f;
    glm::vec2 vertices[] = {
        glm::vec2(sliderPos-hF, y+pad+hF),
        glm::vec2(sliderPos   , y+pad),
        glm::vec2(sliderPos+hF, y+pad+hF),
        glm::vec2(sliderPos+hF, y+h-pad),
        glm::vec2(sliderPos-hF, y+h-pad),
    };
    renderer->DrawPolygon(5, vertices, COLOR_BLACK, COLOR_GREY);

}

void UISliderBase::Update() {
    // TODO: Test to see if value has changed and move slider to relative position    
    if ( visible && labelID == -1 ) {
        // Add slider text label to text manager
        labelID = g_uiMan->GetTextManager()->AddText(m_label, glm::vec3(x+4.0f, y+(SLIDER_TEXT_SIZE/2), WIDGET_TEXT_DEPTH),
                                       true, SLIDER_TEXT_SIZE, FONT_JURA );
    } else if ( !visible && labelID != -1 ) {
        g_uiMan->GetTextManager()->RemoveText(this->labelID);
        labelID = -1;
    }
}

void UISliderBase::UpdatePos( int posX, int posY ) {
    sliderPos += (posX-x);
    x = posX; y = posY;
    if ( labelID != -1 ) {
        g_uiMan->GetTextManager()->UpdateTextPos( labelID, glm::vec3(x+4.0f, y+(SLIDER_TEXT_SIZE/2), WIDGET_TEXT_DEPTH) );
    }
}

