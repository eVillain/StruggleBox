#pragma once

#include "RenderCore.h"
#include "FrameBuffer.h"
#include "RendererDefines.h"
#include "Rect2D.h"
#include "Color.h"
#include "Shape2D.h"
#include <map>
#include <vector>

class Renderer2DDeferred;

struct Light2D
{
    glm::vec2 position;
    Color lightColor;
    float lightWidth;
    float lightHeight;
    float lightArc;
    float lightFlicker;         // set a threshold between 0 and 1 for random flicker
    float lightFlickerVar;      // set a threshold between 0 and 1 for random flicker
    float lightPulse;           // time period for light to pulse
    float lightTimer;           // timestamp since last light animation update
    bool lightDynamic;
};

typedef uint32_t Light2DID;

class Lighting2DDeferred
{    
public:
    Lighting2DDeferred(Renderer2DDeferred& renderer);
    ~Lighting2DDeferred();

    void initialize();
    void terminate();
    
    void renderLighting(const std::vector<Shape2D*>& occluders);

    Light2DID addLight();
    void removeLight(Light2DID lightID);
    Light2D* getLightForID(Light2DID lightID);
    void clear();
    
    FrameBuffer& getFrameBuffer() { return m_frameBuffer; }

    std::vector<Light2DID> getLightsForArea(const Rect2D& area);

private:
    Renderer2DDeferred& m_renderer;
    FrameBuffer m_tempFrameBuffer;
    FrameBuffer m_frameBuffer;
    std::map<Light2DID, Light2D> m_lights;
    Light2DID m_nextLightID;

    DrawDataID m_coloredVertsDrawDataID;
    DrawDataID m_texturedVertsDrawDataID;
    ShaderID m_coloredVertsShaderID;
    ShaderID m_texturedVertsShaderID;
    TempVertBuffer m_coloredTriVertsBuffer;
    TempVertBuffer m_texturedTriVertsBuffer;

    void renderLights(const std::vector<Light2DID>& lights, const std::vector<Shape2D*>& occluders);
    void draw2DLightCircle(const float radius, const Color& lightColor, const glm::mat4& projection);

    void drawCircleShadow(const float lightRadius, const float circleRadius, const glm::vec2& circlePos, const glm::mat4& projection);
};
