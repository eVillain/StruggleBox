#ifndef NGN_RENDERER_BASE_H
#define NGN_RENDERER_BASE_H

#include "GFXIncludes.h"
#include "GFXDefines.h"
#include "Rect2D.h"
#include <string>
#include <vector>

class HyperVisor;
class TextManager;
class LightRenderer2D;
class VertexBuffer;
class Texture;
class Sprite;
class SpriteBatch;
class SDL_Window;
class Locator;

class Renderer {
    
public:
    virtual ~Renderer() { };
    
    virtual void Initialize(Locator& locator) = 0;
    virtual void ShutDown() = 0;
    
    // Start and end frame drawing
    virtual void BeginDraw() = 0;
    virtual void EndDraw() = 0;
    // Post-processing
    virtual void RenderLighting( const Color& fogColor ) = 0;
    virtual void PostProcess() = 0;
    virtual void RenderFromTexture( const GLuint tex ) = 0;
    // Matrix functionality
    virtual void GetUIMatrix( glm::mat4& target ) = 0;      // Shaders
    virtual void SetUIMatrix() = 0;                         // Fixed-pipeline
    virtual void GetGameMatrix( glm::mat4& target ) = 0;    // Shaders
    virtual void SetGameMatrix() = 0;                       // Fixed-pipeline
    // Vertex buffer operations
    virtual void RenderVertBuffer( VertexBuffer* vBuffer,
                                  const unsigned int range, const unsigned int first=0,
                                  const Texture* tex=NULL, const bool render3D=true ) = 0;
    virtual void RenderInstancedBuffer( VertexBuffer* vBuffer, const std::vector<InstanceData>& instances,
                                  const unsigned int range, const unsigned int first=0,
                                  const Texture* tex=NULL, const bool render3D=true ) = 0;
    virtual void RenderSkyBuffer( VertexBuffer* vBuffer, glm::vec3 sunDir ) = 0;
    virtual void RenderGroundPlane( Color& horizonColor ) = 0;
    // General triangle buffering functions
    virtual void BufferVerts( const ColorVertexData* verts, const int numVerts ) = 0;
    virtual void BufferVerts( const NormalVertexData* verts, const int numVerts ) = 0;
    virtual void BufferSpheres( const ColorVertexData* spheres, const int numSpheres ) = 0;
    virtual void RenderVerts() = 0;
    virtual void RenderSpheres() = 0;
    // 2D Buffering functions
    virtual void Buffer2DLine( glm::vec2 a, glm::vec2 b, Color aColor, Color bColor, float z = 0.0f ) = 0;
    virtual void Render2DLines() = 0;
    // 3D Buffering functions
    virtual void Buffer3DLine( glm::vec3 a, glm::vec3 b, Color aColor, Color bColor ) = 0;
    virtual void Render3DLines() = 0;
    virtual void Buffer3DCube(CubeInstance& instance) = 0;
    virtual void Render3DCubes() = 0;
    virtual void Render2DCube( const glm::vec2& center, const Color color, const glm::vec3 rotation) = 0;
    // 2D Drawing functions ( drawn immediately )
    virtual void DrawPolygon( const int count, const GLfloat *verts,
                             const Color lineColor, const Color fillColor, const float z = 0.0f ) = 0;
    virtual void DrawPolygon( const int count, const glm::vec2 *verts,
                             const Color lineColor, const Color fillColor, const float z = 0.0f ) = 0;
    virtual void Draw2DRect( Rect2D rect,
                            Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    virtual void Draw2DRect( glm::vec2 center, float width, float height,
                            Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    virtual void Draw2DRect3D( glm::vec3 center, float width, float height,
                              Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    virtual void Draw2DProgressBar( glm::vec3 center, float width, float height, float amount,
                           Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    // Gradients
    virtual void DrawGradientY( Rect2D rect, Color topColor, Color btmColor, float z = 0.0f ) = 0;
    virtual void DrawGradientX( Rect2D rect, Color leftColor, Color rightColor, float z = 0.0f ) = 0;
    // Circles and grids
    virtual void DrawCircle( glm::vec2 center, float angle, float radius,
                    Color lineColor, Color fillColor, float z = 0.0f, const int pixelPerSeg=8 ) = 0;
    virtual void DrawRing( glm::vec2 center, float radius1, float radius2, int segs,
                  Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    virtual void DrawGrid( float gridSize, Rect2D rect, int subDivisions, Color color = COLOR_WHITE ) = 0;
    virtual void Draw3DGrid( const glm::vec3& pos, const float size, const int divisions ) = 0;
    // 2D Drawing textured
    virtual void DrawSprite( const Sprite& sprite ) = 0;
    virtual void DrawSpriteBatch( const SpriteBatch& batch ) = 0;

    virtual void DrawImage( const glm::vec2 center, const float width, const float height,
                           const std::string texName, const float z = 0.0f, const Color color = COLOR_WHITE) = 0;
    virtual void DrawTexture( const Rect2D rect, const Rect2D texRect,
                             const GLuint tex, const float z = 0.0f, const Color color = COLOR_WHITE) = 0;
    virtual void DrawTextureArray( const Rect2D rect, const Rect2D texRect,
                                  const int width, const int height,
                                 const GLbyte* data, const float z = 0.0f, const Color color = COLOR_WHITE) = 0;
    // 3D Drawing functions ( drawn immediately )
    virtual void DrawBoxOutline( glm::vec3 center, glm::vec3 boxSize, Color color ) = 0;
    // Get cursor 3D coordinates from screen pos
    virtual const glm::vec3 GetCursor3DPos( const glm::vec2 cursorPos ) const = 0;
    // Extra renderer components
    LightRenderer2D* lightRenderer;
    
    // Common accessors
    virtual const std::string GetInfo() const = 0;
    LightRenderer2D* GetLightRenderer() { return lightRenderer; };
    
    
    // Variables    
    bool initialized;
    bool shouldClose;

    int windowWidth, windowHeight;
    // Variables for the current camera attributes
    GLfloat r_camX, r_camY, r_camZoom;
    // Variables for the wanted camera attributes
    GLfloat r_camTX, r_camTY, r_camTZoom;
    
    // Statistics (per-frame)
    double r_frameStartTime;        // Timestamp from previous frame for delta time calculation
    double r_frameCounterTime;      // Timestamp from last time FPS counter was updated
    int r_frameCounter;             // Total number of frames rendered
    
    unsigned int renderedTris;      // Total number of triangles rendered this frame
    unsigned int renderedSegs;      // Total number of line segments rendered this frame
    unsigned int renderedSprites;   // Total number of sprites rendered this frame
};

#endif
