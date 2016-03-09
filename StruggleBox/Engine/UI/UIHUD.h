#ifndef UI_HUD_H
#define UI_HUD_H

#include <vector>
#include <string>
#include "UIWidget.h"
#include "GFXDefines.h"

class Renderer;
class Entity;

class UIHUD : public UIWidget
{

public:
    UIHUD();
    ~UIHUD();
    
    void AddMessage( std::string msg, Color msgColor );
    void Draw( Renderer* renderer );
private:
    std::vector<int> scrnMsgs;      // Text labels
    Entity* controlEntity;          // Entity this HUD belongs to
};

#endif /* UI_HUD_H */
