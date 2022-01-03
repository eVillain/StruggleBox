#include "LightSystem2D.h"
#include "Renderer.h"
#include "RendererGLProg.h"
//#include "Shape.h"
#include "Options.h"

#include "GFXDefines.h"
#include "LightRenderer2D.h"
#include "Console.h"

/** PUBLIC LIGHT STUFF **/

LightSystem2D::LightSystem2D() {
    renderedLights = 0;
}
LightSystem2D::~LightSystem2D( void ) {
    m_lightRenderer = NULL;
}
void LightSystem2D::HookRenderer( Renderer* renderer ) {
        // Hook up new renderer pointer
        m_renderer = renderer;
        if ( renderer ) {
            //m_lightRenderer = renderer->GetLightRenderer();
        } else {
            m_lightRenderer = NULL;
        }
}


void LightSystem2D::GetLightsForArea(float x,float y,float width,float height, std::vector<Light2D*> &containedLights) {
    // All values in pixels
    for ( unsigned int i=0; i < _lights.size(); i++ ) {
        const int entityIndex = i;
        Light2D * theLight = _lights.at(entityIndex);
        float cZ = 1.0f/1.0f;
//            float radius = (float)(theLight->lightWidth * Renderer::GetCamZoom());
        float radius = theLight->lightWidth *cZ;
        glm::vec2 lightPosPX = glm::vec2(theLight->position.x, theLight->position.y) * cZ ;
        if (lightPosPX.x+radius < x-width*0.5f ||
            lightPosPX.x-radius > x+width*0.5f ||
            lightPosPX.y+radius < y-height*0.5f ||
            lightPosPX.y-radius > y+height*0.5f)
        { continue; }
        else
        { containedLights.push_back(theLight); }
    }
}
void LightSystem2D::RenderLighting(void * space) {
    if ( !m_lightRenderer ) return;
//    if ( Options::getInst()->GetBool("r_useShaders") ) return;
    
//    float resX = m_renderer->windowWidth;
//    float resY = m_renderer->windowHeight;
////        GLfloat camX = Renderer::GetCamX();
////        GLfloat camY = Renderer::GetCamY();
////        GLfloat camScale = Renderer::GetCamZoom();
//    GLfloat camX = 0.0f;
//    GLfloat camY = 0.0f;
//    GLfloat camScale = 1.0f;
//    
//    // Get only lights that are visible
//    std::vector<Light2D*> onScrnLights;
//    GetLightsForArea( -camX*camScale, -camY*camScale, resX, resY, onScrnLights );
//    
//    renderedLights = onScrnLights.size();
////    GLuint fbo = 0;
////    if ( Options::getInst()->GetBool("r_useShaders") && Options::getInst()->GetBool("r_deferred") ) {
////        fbo = ((RendererGLProg*)m_renderer)->GetFBO();
////    }
//    if ( renderedLights > 0 ) {
//        m_lightRenderer->RenderLights(onScrnLights, space, 0);
//    };
//    m_lightRenderer->UpdateLightStats();
}


void LightSystem2D::Add(Light2D* newLight) {
    bool containedLight = false;
    for (unsigned int i = 0; i < _lights.size(); ++i) {
        if (_lights.at(i) == newLight) {
            containedLight = true;
            break;
        }
    }
    
    if ( containedLight == false ) {
        _lights.push_back(newLight);
    }
}

void LightSystem2D::Remove(Light2D* oldLight) {
    for (unsigned int i = 0; i < _lights.size(); ++i) {
        if (_lights.at(i) == oldLight) {
            _lights.erase(_lights.begin() + i);
            Log::Debug("[LightSystem] Erased light at:%i", i);
            break;
        }
    }
}

const bool LightSystem2D::Contains(Light2D* theLight)  {
    return (find(theLight) > -1);
}

Light2D* LightSystem2D::at(const int index) {
    return _lights.at(index);
}

const int LightSystem2D::find(Light2D* theLight) {
    for (unsigned int i = 0; i < _lights.size(); ++i) {
        if (_lights.at(i) == theLight) {
            return i;
        }
    }
    return -1;
}
//    const int LightSystem2D::findByID(unsigned int entityID) {
//        for (unsigned int i = 0; i < _lights.size(); ++i) {
//            if (_lights.at(i)->GetID() == entityID) {
//                return i;
//            }
//        }
//        return -1;
//    }
//    const int LightSystem2D::findByBody(Body*body) {
//        for (unsigned int i = 0; i < _lights.size(); ++i) {
//            if (_lights.at(i)->entityBody == body) {
//                return i;
//            }
//        }
//        return -1;
//    }
//    const int LightSystem2D::findByShape(Shape*shape) {
//        for (unsigned int i = 0; i < _lights.size(); ++i) {
//            if (_lights.at(i)->entityShape == shape) {
//                return i;
//            }
//        }
//        return -1;
//    }
const unsigned long LightSystem2D::NumLights() 
{
    return (int)_lights.size();
}

void LightSystem2D::Clear()
{
    for (unsigned int i = 0; i < _lights.size(); ++i) 
    {
        delete _lights.at(i);
    }
    Log::Debug("cleared %lu 2d lights", _lights.size());
    _lights.clear();
}
