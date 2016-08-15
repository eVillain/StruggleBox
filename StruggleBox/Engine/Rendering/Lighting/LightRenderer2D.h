#ifndef LIGHT_RENDERER2D_H
#define LIGHT_RENDERER2D_H

#include <vector>
#include "GFXDefines.h"
#include "Color.h"

class Renderer;
class Light2D;

/**
 * Class containing rendering functions for 2D vertex lights
 */
class LightRenderer2D {
private:
    glm::vec2 ProjectVertForLight(glm::vec2 vert, glm::vec2 light);
    glm::vec2 ProjectVertForLightToInfinity(glm::vec2 vert, glm::vec2 light);
    glm::vec2 ProjectVertForLightToRadius(glm::vec2 vert, glm::vec2 light, float radius);
    int renderedTris;
    void RenderCircleShadow(glm::vec2 center1, glm::vec2 center2, float angle, float radius1, float radius2, float range, Color shadowColor);
    
//    void RenderShadowsForLight(LightComponent* light, cpSpace * space);

    GLuint light_fbo;             // The frame buffer object
    GLuint light_texture;         // The texture object to write our frame buffer object
    GLuint glow_texture;          // The temporary texture object to write our glow to
    
    int FBO_width;              // The width of our FBO
    int FBO_height;             // The height of our FBO
    
    void InitFrameBuffers(void);
    void ReleaseBuffers(void);
    
    Renderer* m_renderer;       // Pointer to owning renderer
public:
    LightRenderer2D( Renderer* renderer );
    ~LightRenderer2D();
    
    void RenderLights( std::vector<Light2D*>& containedLights, void* space, GLuint renderFBO = 0 );
    
    bool DoesEdgeCastShadow(glm::vec2 start, glm::vec2 end, glm::vec2 light);
    void RenderShadow(glm::vec2 vert1, glm::vec2 vert2, glm::vec2 lightCenter, Color shadowColor, bool infiniteShadow);
//    void DrawShapeShadow(cpShape *shape, float lightRadius, glm::vec2 lightCenter, Color shadowColor, bool infiniteShadow);
    
    void Draw2DLightCircle( glm::vec2 center, float radius, float angle, int segs, Color lightColor);
    void Draw2DLightBeam( glm::vec2 center, float arc, float radius, float angle, int segs, Color lightColor);
        
//    void DrawShapeGlow(cpShape *shape);
//    void RenderGlow(cpSpace * space);


    void BlurTextureRadial (unsigned int tex, int passes, float texWidth, float texHeight, float renderWidth, float renderHeight);
    void BlurTextureZoom (unsigned int tex, int passes, float texWidth, float texHeight, float renderWidth, float renderHeight);
    void BlurTexture (unsigned int tex, int passes, float texWidth, float texHeight, float renderWidth, float renderHeight);
    
    void UpdateLightStats( void );
};

#endif
