#include "UIHUD.h"

#include "Renderer.h"
#include "TextManager.h"
#include "Entity.h"

UIHUD::UIHUD() :
UIWidget(0,0,100,100) {
    controlEntity = NULL;
}

UIHUD::~UIHUD() {
   
}

void UIHUD::AddMessage( std::string msg, Color msgColor ) {
    glm::vec3 txtPos = glm::vec3(0,0,0);
    g_uiMan->GetTextManager()->AddText(msg, txtPos, true, 16, FONT_DEFAULT, 0.0, msgColor);
    // Move old messages up
    for ( unsigned int i=0; i<scrnMsgs.size(); i++ ) {
        txtPos.y += 16;
        g_uiMan->GetTextManager()->UpdateTextPos(scrnMsgs[i], txtPos);
    }
}

void UIHUD::Draw( Renderer *renderer ) {
    // Draw UI dependent messages
    
}

