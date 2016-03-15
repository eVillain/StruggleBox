#include "UITextInput.h"
#include "TextManager.h"
#include "HyperVisor.h"
#include "SysCore.h"
#include "Texture.h"
#include "Renderer.h"

#define TEXTSIZE 16
#define CURSOR_BLINK_RATE 0.5

UITextInputBase::UITextInputBase(int posX, int posY,
                                 int width,int height,
                                 int depth,
                                 std::string description,
                                 std::string defaultVal) :
UIWidget(posX, posY, width, height, depth),
inputValue(),
label(description),
startValue(defaultVal)
{
    // Add description text label to text manager
    labelID = g_uiMan->GetTextManager()->AddText(label,
                                                 glm::vec3(x+TEXTSIZE, y-contentHeight+TEXTSIZE*1.5+2, 0.0f),
                                                 true, TEXTSIZE, FONT_JURA, 0.0, COLOR_UI_TEXT );
    h += 18;
    inputLabelID = g_uiMan->GetTextManager()->AddText(defaultVal,
                                                      glm::vec3(x+TEXTSIZE, y-2+TEXTSIZE/2, 0.0f),
                                                      true, TEXTSIZE, FONT_JURA, 0.0, COLOR_UI_TEXT );
    
    Clear();
    lastCursorBlink = SysCore::GetSeconds();
    lastBackSpace = 0;
    grabKeyboardInput = false;
    g_uiMan->AddWidget(this);
}

UITextInputBase::~UITextInputBase()
{
    CancelTextInput();
    g_uiMan->GetTextManager()->RemoveText( labelID );
    g_uiMan->GetTextManager()->RemoveText( inputLabelID );
    g_uiMan->RemoveWidget(this);
}

void UITextInputBase::UpdatePos(int posX,
                                int posY)
{
    x = posX; y = posY;
    g_uiMan->GetTextManager()->UpdateTextPos(labelID, glm::vec3(x+TEXTSIZE, y-contentHeight+TEXTSIZE+8, 0.0f));
    g_uiMan->GetTextManager()->UpdateTextPos(inputLabelID, glm::vec3(x+TEXTSIZE, y+TEXTSIZE/2, 0.0f));
}

void UITextInputBase::CursorPress(const glm::ivec2 coord)
{
    active = true;
}

void UITextInputBase::CursorRelease(const glm::ivec2 coord)
{
    if ( active ) {
        StartTextInput();
    } else if ( !highlighted ) {
        
    }
}

void UITextInputBase::CursorHover(const glm::ivec2 coord, bool highlight)
{
    highlighted = highlight;
}

void UITextInputBase::Draw(Renderer* renderer)
{    
    // Pixel perfect outer border (should render with 1px shaved off corners)
    renderer->Buffer2DLine(glm::vec2(x,y+1), glm::vec2(x,y+h), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // L
    renderer->Buffer2DLine(glm::vec2(x,y+h), glm::vec2(x+w-1,y+h), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);   // T
    renderer->Buffer2DLine(glm::vec2(x+w,y+h), glm::vec2(x+w,y+1), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);   // R
    renderer->Buffer2DLine(glm::vec2(x+w-1,y), glm::vec2(x,y), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // B
    renderer->Buffer2DLine(glm::vec2(x+w-1,y+TEXTSIZE+4), glm::vec2(x,y+TEXTSIZE+4), COLOR_UI_BORDER_OUTER, COLOR_UI_BORDER_OUTER);       // B
    // Inner gradient fill
    renderer->DrawGradientY(Rect2D((float)x, (float)y+1, (float)w-1, (float)h-1), COLOR_UI_GRADIENT_TOP, COLOR_UI_GRADIENT_BOTTOM);
    // Inside border
    glEnable(GL_BLEND);
    renderer->Draw2DRect(Rect2D(x+1,y+1,w-2,h-2), COLOR_UI_BORDER_INNER, COLOR_NONE);
    
    // Render blinking cursor
    if ( grabKeyboardInput ) {
        double timeNow = SysCore::GetSeconds();
        double cursorBlinkDelta = timeNow - lastCursorBlink;
        if (cursorBlinkDelta > CURSOR_BLINK_RATE) {
            cursorBlink = !cursorBlink;
            lastCursorBlink = timeNow;
        }
        if (cursorBlink) {
            float textWidth, textHeight;
            g_uiMan->GetTextManager()->GetTextSize(inputLabelID, textWidth, textHeight);
            renderer->Buffer2DLine(glm::vec2(x+TEXTSIZE+textWidth+4, y-8+(h-TEXTSIZE)*0.6f),
                                   glm::vec2(x+TEXTSIZE+textWidth+4 , y-8+h-(h-TEXTSIZE)*0.6f),
                                   COLOR_WHITE, COLOR_WHITE);       // Cursor
        }
    }
    renderer->Render2DLines();
    
    if ( active ) {
        g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT_ACTIVE);
    } else {
        if ( highlighted ) {
            g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT_HIGHLIGHT);
        } else {
            g_uiMan->GetTextManager()->UpdateTextColor(labelID, COLOR_UI_TEXT);
        }
    }
}

//========================================================================
// Text input functions
//========================================================================
void UITextInputBase::StartTextInput() {
    if ( !grabKeyboardInput ) {
        // Register for keyboard input
        g_uiMan->GetInputManager()->RegisterEventObserver(this);
        g_uiMan->GetInputManager()->StartTextInput(this);
        g_uiMan->GetTextManager()->UpdateText(inputLabelID, inputValue);
        active = true;
        grabKeyboardInput = true;
    } else {
        printf("[UITextInput] WARNING: trying to grab input twice!");
    }
}
void UITextInputBase::StopTextInput() {
    if ( grabKeyboardInput ) {
        // Unregister keyboard input
        g_uiMan->GetInputManager()->UnRegisterEventObserver(this);
        g_uiMan->GetInputManager()->StopTextInput(this);
        grabKeyboardInput = false;
        active = false;
    }
}
void UITextInputBase::CancelTextInput() {
    if ( grabKeyboardInput ) {
        grabKeyboardInput = false;
        Clear();
        g_uiMan->GetTextManager()->UpdateText( inputLabelID, "<enter text>" );
        active = false;
        // Unregister keyboard input
        g_uiMan->GetInputManager()->UnRegisterEventObserver(this);
        g_uiMan->GetInputManager()->StopTextInput(this);
//        printf("TextInput cancelled\n");
    }
}
void UITextInputBase::Clear() {
    inputValue = "";
    if ( cursorPosition != 0 ) {
        cursorPosition = 0;
        UpdateTextInput();
    }
}
void UITextInputBase::UpdateTextInput()
{
    g_uiMan->GetTextManager()->UpdateText(inputLabelID, inputValue);
}

std::string UITextInputBase::GetText()
{
    return inputValue;
}

void UITextInputBase::SetText(std::string newText)
{
    inputValue = newText;
    cursorPosition = (int)inputValue.length();
    UpdateTextInput();
}

bool UITextInputBase::OnEvent(const std::string& event,
             const float& amount)
{
    if (amount == 1.0f)
    {
        if (event == INPUT_ERASE_LEFT)
        {
            if (grabKeyboardInput && cursorPosition > 0)
            {
                double timeNow = SysCore::GetSeconds();
                if ( timeNow - lastBackSpace > 0.05 ) {
                    lastBackSpace = timeNow;
                    
                    std::string firstPart;
                    if (cursorPosition > 1) {
                        firstPart = inputValue.substr(0, cursorPosition-1);
                    }
                    std::string secondPart;
                    if (cursorPosition < inputValue.length()) {
                        secondPart = inputValue.substr(cursorPosition, inputValue.length()-cursorPosition);
                    }

                    inputValue = firstPart + secondPart;

                    cursorPosition--;

                    UpdateTextInput();
                }
                return true;
            }
        }
    } else if (amount == -1.0f) {
        if (event == INPUT_BACK) {
            StopTextInput();
            return true;
        } else if (event == INPUT_START ||
                   event == INPUT_JUMP) {
            DoCallBack();
            return true;
        } else if ( event == INPUT_ERASE_LEFT ) {
            lastBackSpace = 0;
            return true;
        }
    }
    return false;
}

void UITextInputBase::OnTextInput(const std::string& text)
{
//    printf("text %s\n", text.c_str());
    if (grabKeyboardInput)
    {
        inputValue = text;
        cursorPosition = (int)inputValue.length();
        UpdateTextInput();
    }
}
