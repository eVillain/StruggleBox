#ifndef GUI_H
#define GUI_H

#include "Input.h"
#include "GFXDefines.h"
#include <vector>
#include <memory>

class Locator;
class Widget;
class Renderer;
class SpriteBatch;
class Shader;

class GUI : public InputEventListener, public MouseEventListener
{
public:
    GUI();
    
    // Factory method for spawning widgets - USE IT :)
    template <class WidgetType>
    std::shared_ptr<WidgetType> CreateWidget()
    {
        std::shared_ptr<WidgetType> widget = std::shared_ptr<WidgetType>(new WidgetType());
        _widgets.push_back(std::dynamic_pointer_cast<Widget>(widget));
        return widget;
    }
    
    void DestroyWidget(std::shared_ptr<Widget> widget);
    
    void Draw(Renderer* renderer);
    void Update(const double deltaTime);
    
    bool Initialize(Locator& locator);
    bool Terminate();
    
protected:
    // Test cursor events on widgets - return value true means event was swallowed
    bool OnCursorHover(const glm::ivec2& coord);
    bool OnCursorPress(const glm::ivec2& coord);
    bool OnCursorDrag(const glm::ivec2& coord );
    bool OnCursorRelease(const glm::ivec2& coord);

private:
    bool OnEvent(const std::string& theEvent,
                 const float& amount );
    bool OnMouse(const glm::ivec2& coord);
    
    const glm::ivec2 ConvertSDLCoordToScreen(const glm::ivec2& coord) const;

    std::vector<std::shared_ptr<Widget>> _widgets;
    
    // Dependencies
    Input* _input;
    Renderer* _renderer;
    
    SpriteBatch* _spriteBatch;  // Sprite batch for UI items
    Shader* _shaderColor;  // Shader for colored UI widgets
    Shader* _shaderTex;    // Shader for textured UI widgets
    
    GLuint _vao;    // Vertex array object ID for UI vertex layout
    GLuint _vbo;    // Vertex array buffer ID for UI verts

    glm::ivec2 _currentMouseCoord;
    bool _mouseDrag;
};

#endif /* GUI_H */
