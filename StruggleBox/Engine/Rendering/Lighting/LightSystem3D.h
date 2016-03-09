//
//  LightSystem3D.h
//  Ingenium
//
//  Created by The Drudgerist on 11/01/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#ifndef NGN_LIGHT_SYSTEM3D_H
#define NGN_LIGHT_SYSTEM3D_H

#include "Light3D.h"
#include <vector>
class Renderer;
class LightSystem3D {
private:
    std::vector<Light3D*> m_lights;		/**< The list of Light pointers */
    unsigned long renderedLights;        // Number of lights on screen in last render pass
    
    Renderer* m_renderer;
    
public:
    
    LightSystem3D();
    ~LightSystem3D();
    void HookRenderer( Renderer* renderer );

    void RenderLighting(GLuint fbo);
    
    void Add(Light3D* newLight);
    void Remove(Light3D* oldLight);
    void Clear();
    
    const unsigned long NumLights();
    const bool Contains(Light3D* theLight);
    
    const int Find(Light3D* theLight);

    void GetLightsForArea(glm::vec3 pos, glm::vec3 radius, std::vector<Light3D*>& containedLights);
    std::vector<Light3D*>& GetLights() { return m_lights; };
};

#endif /* defined(NGN_LIGHT_SYSTEM3D_H) */
