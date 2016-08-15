#ifndef LIGHT_SYSTEM2D_H
#define LIGHT_SYSTEM2D_H

#include <vector>
#include "Light2D.h"

/**
 * Class containing a list of Lights in game world
 */
class Renderer;
class LightRenderer2D;

class LightSystem2D {
private:
    std::vector<Light2D*> _lights;		/**< The list of Light pointers */
    unsigned long renderedLights;        // Number of lights on screen in last render pass

    Renderer* m_renderer;
    LightRenderer2D* m_lightRenderer;
    
public:
    LightSystem2D();
    ~LightSystem2D();
    
    void HookRenderer( Renderer* renderer );

//    unsigned long GetRenderedLights() { return renderedLights; };

    void RenderLighting(void * space);

    void Add(Light2D* newLight);
    void Remove(Light2D* oldLight);
    void Clear();

    const unsigned long NumLights();
    const bool Contains(Light2D* theLight);
    
    Light2D* at(const int index);
    const int find(Light2D* theLight);

    
    void GetLightsForArea(float x,float y,float width,float height, std::vector<Light2D*>& containedLights);
};

#endif
