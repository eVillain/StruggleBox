#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <vector>
#include "SpriteBatch.h"
#include "InputListener.h"

class Locator;
class UIWidget;
class Input;

#define WIDGET_DEPTH 1.0
#define WIDGET_TEXT_DEPTH 3.0
#define BUTTON_BG_DEPTH 2.0

class UIManager : public InputEventListener, public MouseEventListener
{
public:
    UIManager();
    ~UIManager();
    
    void Initialize(Locator& locator);
    void Terminate();
    
    void LoadUIBatch( const std::string batchName );

    void AddWidget( UIWidget* widget );
    void RemoveWidget( UIWidget* widget );
    void RenderWidgets( void );
    void CleanUp( void );
    void Refresh( void );
    
    // Test cursor events on widgets
    bool OnCursorHover(const glm::ivec2& coord);
    bool OnCursorPress(const glm::ivec2& coord);
    bool OnCursorRelease(const glm::ivec2& coord);
    
    void SetTextManager(TextManager* textMan) { g_textMan = textMan; };
    TextManager* GetTextManager() { return g_textMan; };
    Input* GetInputManager() { return g_input; };
    Renderer* GetRenderer() { return g_renderer; };
    
    const SpriteBatch* GetBatch() { return uiBatch; };
    const Shader* GetShaderColor() { return uiShaderColor; };
    const Shader* GetShaderTex() { return uiShaderTex; };
    
private:
    TextManager* g_textMan;             // Global pointer to text manager
    Renderer* g_renderer;               // Global pointer to renderer
    Input* g_input;
    
    std::vector<UIWidget*> _widgetList; // Vector of widgets to keep track of
    SpriteBatch * uiBatch;              // Sprite batch for UI items
    Shader* uiShaderColor;              // Shader for colored UI widgets
    Shader* uiShaderTex;                // Shader for textured UI widgets

    GLuint widget_vao;                  // Vertex array object ID for UI shaders
    GLuint widget_vbo;                  // Vertex array buffer ID for UI shaders
    
    glm::ivec2 _currentMouseCoord;
    bool _mouseDrag;

    void LoadUIShaders( void );
    void UnloadUIShaders( void );
    void UnloadUIBatch( void );
    
    bool OnEvent(const std::string& theEvent, const float& amount);
    bool OnMouse(const glm::ivec2& coord);
    
    glm::ivec2 ConvertSDLCoordToScreen(const glm::ivec2& coord) const;
};


#endif /* UI_MANAGER_H */
