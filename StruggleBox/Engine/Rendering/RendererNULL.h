
#ifndef RENDERER_NULL_H
#define RENDERER_NULL_H

#include "Renderer.h"

//  This class is a debug renderer which only contains
//  logging capabilities without the actual rendering
// TODO: Complete implementation and route printfs to Log
class RendererNULL : public Renderer
{
    
public:
    RendererNULL() { printf("[RendererNULL] created\n"); };
    ~RendererNULL() { printf("[RendererNULL] destroyed\n"); };
    
    void Initialize() { initialized = true; printf("[RendererNULL] initialized.\n"); };
    void ShutDown() { initialized =  false; printf("[RendererNULL] shut down.\n"); };
    
    void BeginDraw() { printf("[RendererNULL] beginning of frame draw\n"); };
    void EndDraw() { printf("[RendererNULL] ending of frame draw\n"); };
    
    void BufferLine( glm::vec3 a, glm::vec3 b, Color aColor, Color bColor ) { printf("[RendererNULL] buffering line\n"); };
    void RenderLines() { printf("[RendererNULL] rendering lines\n"); };
    
    void DrawPolygon( int count, GLfloat *verts,
                     Color lineColor, Color fillColor, float z = 0.0f ) { printf("[RendererNULL] drawing polygon\n"); };
    void DrawRect( Rect2D rect,
                  Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) { printf("[RendererNULL] drawing rect 1\n"); };
    void DrawRect( glm::vec2 center, float width, float height,
                  Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) { printf("[RendererNULL] drawing rect 2\n"); };

    void DrawImage( glm::vec2 center, float width, float height,
                   std::string texName, float z = 0.0f, Color color = COLOR_WHITE) { printf("[RendererNULL] drawing image %s\n", texName.c_str() ); };

    // Common accessors
    SDL_Window* GetWindow() { printf("[RendererNULL] Window pointer requested"); return NULL; };

};

#endif
