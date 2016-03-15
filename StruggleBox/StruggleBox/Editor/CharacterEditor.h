#ifndef CHARACTER_EDITOR_H
#define CharacterEditor_hpp

#include "EditorScene.h"

class Locator;

class CharacterEditor : public EditorScene
{
public:
    CharacterEditor(Locator& locator);
    
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(double deltaTime);
    void Draw();
    
protected:
    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);
};

#endif /* CHARACTER_EDITOR_H */
