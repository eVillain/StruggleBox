//
//  LightSystem3D.cpp
//  Ingenium
//
//  Created by The Drudgerist on 11/01/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "LightSystem3D.h"
#include "HyperVisor.h"
#include "Renderer.h"
#include "Camera.h"

LightSystem3D::LightSystem3D() {
    renderedLights = 0;
}
LightSystem3D::~LightSystem3D( void ) {
    m_renderer = NULL;
}
void LightSystem3D::HookRenderer( Renderer* renderer ) {
    // Hook up new renderer pointer
    m_renderer = renderer;
    m_lights.clear();
}

void LightSystem3D::GetLightsForArea(glm::vec3 pos, glm::vec3 radius, std::vector<Light3D*> &containedLights) {
    // All values in pixels
    for ( unsigned int i=0; i < m_lights.size(); i++ ) {
        const int entityIndex = i;
        Light3D * theLight = m_lights.at(entityIndex);
        float lightRadius = theLight->position.w;
        if (theLight->position.x+lightRadius < pos.x-radius.x ||
            theLight->position.x-lightRadius > pos.x+radius.x||
            theLight->position.y+lightRadius < pos.y-radius.y ||
            theLight->position.y-lightRadius > pos.y+radius.y ||
            theLight->position.z+lightRadius < pos.x-radius.z ||
            theLight->position.z-lightRadius > pos.x+radius.z )
        { continue; }
        else
        { containedLights.push_back(theLight); }
    }
}
void LightSystem3D::RenderLighting( GLuint fbo ) {
    if ( !m_renderer ) return;
}


void LightSystem3D::Add(Light3D* newLight) {
    bool containedLight = false;
    for (unsigned int i = 0; i < m_lights.size(); ++i) {
        if (m_lights.at(i) == newLight) {
            containedLight = true;
            break;
        }
    }
    
    if ( containedLight == false ) {
        m_lights.push_back(newLight);
    }
}

void LightSystem3D::Remove(Light3D* oldLight) {
    for (unsigned int i = 0; i < m_lights.size(); ++i) {
        if (m_lights.at(i) == oldLight) {
            m_lights.erase(m_lights.begin() + i);
//            Console::Print("[LightSystem3D] Erased light at:%i\n", i);
            break;
        }
    }
}

const bool LightSystem3D::Contains(Light3D* theLight)  {
    return (Find(theLight) > -1);
}

const int LightSystem3D::Find(Light3D* theLight) {
    for (unsigned int i = 0; i < m_lights.size(); ++i) {
        if (m_lights.at(i) == theLight) {
            return i;
        }
    }
    return -1;
}
const unsigned long LightSystem3D::NumLights() {
    return (int)m_lights.size();
}
void LightSystem3D::Clear() {
    for (unsigned int i = 0; i < m_lights.size(); ++i) {
        delete m_lights.at(i);
    }
    printf("[LightSystem3D] Cleared %lu lights\n", m_lights.size());
    m_lights.clear();
}

