#include "Lighting2DDeferred.h"

#include "Renderer2DDeferred.h"
#include "GFXDefines.h"
#include "DefaultShaders.h"

const int32_t LIGHT_FBO_SIZE = 1024;

Lighting2DDeferred::Lighting2DDeferred(Renderer2DDeferred& renderer)
    : m_renderer(renderer)
    , m_tempFrameBuffer(renderer.getRenderCore(), "Light2DTempFrameBuffer")
    , m_frameBuffer(renderer.getRenderCore(), "Light2DFrameBuffer")
    , m_nextLightID(0)
    , m_coloredVertsDrawDataID(0)
    , m_texturedVertsDrawDataID(0)
    , m_coloredVertsShaderID(0)
    , m_texturedVertsShaderID(0)
    , m_coloredTriVertsBuffer()
    , m_texturedTriVertsBuffer()
{
}

Lighting2DDeferred::~Lighting2DDeferred()
{
}

void Lighting2DDeferred::initialize()
{
    RenderCore& renderCore = m_renderer.getRenderCore();
    m_tempFrameBuffer.initialize(LIGHT_FBO_SIZE, LIGHT_FBO_SIZE);
    m_frameBuffer.initialize(renderCore.getRenderResolution().x, renderCore.getRenderResolution().y);
    m_coloredVertsDrawDataID = renderCore.createDrawData(ColoredVertexConfig);
    m_texturedVertsDrawDataID = renderCore.createDrawData(TexturedVertex3DConfig);
    m_coloredVertsShaderID = renderCore.getShaderIDFromSource(coloredVertexShaderSource, coloredFragmentShaderSource, "ColoredVertsShader");
    m_texturedVertsShaderID = renderCore.getShaderIDFromSource(texturedVertexShaderSource, texturedFragmentShaderSource, "TexturedVertsShader");
}

void Lighting2DDeferred::terminate()
{
    RenderCore& renderCore = m_renderer.getRenderCore();
    renderCore.removeShader(m_texturedVertsShaderID);
    renderCore.removeShader(m_coloredVertsShaderID);
    
    m_tempFrameBuffer.terminate();
    m_frameBuffer.terminate();
}

void Lighting2DDeferred::renderLighting(const std::vector<Shape2D*>& occluders)
{   
    m_frameBuffer.bindAndClear(COLOR_NONE);
    const Camera2D& camera = m_renderer.getDefaultCamera();
    const Rect2D cameraRect = camera.getWorldRect();
    std::vector<Light2DID> onScrnLights = getLightsForArea(cameraRect);
    if (onScrnLights.size() > 0)
    {
        renderLights(onScrnLights, occluders);
    };
}

Light2DID Lighting2DDeferred::addLight()
{
    const Light2DID lightID = m_nextLightID++;
    m_lights[lightID] = { glm::vec2(), COLOR_WHITE, 10.f, 10.f, 0.f, 0.f, 0.f, 0.f, 0.f, false };
    return lightID;
}

void Lighting2DDeferred::removeLight(Light2DID lightID)
{
    auto it = m_lights.find(lightID);
    if (it != m_lights.end())
    {
        m_lights.erase(it);
    }
}

Light2D* Lighting2DDeferred::getLightForID(Light2DID lightID)
{
    auto it = m_lights.find(lightID);
    if (it != m_lights.end())
    {
        return &it->second;
    }
    return nullptr;
}

void Lighting2DDeferred::clear()
{
    Log::Debug("[LightSystem2D::Clear] cleared %lu 2d lights", m_lights.size());
    m_lights.clear();
}

std::vector<Light2DID> Lighting2DDeferred::getLightsForArea(const Rect2D& area)
{
    std::vector<Light2DID> lights;
    for (const auto& pair : m_lights)
    {
        const Light2D& light = pair.second;
        const Rect2D lightArea = Rect2D(light.position, glm::vec2(light.lightWidth * 2.f, light.lightHeight * 2.f));
        if (area.intersects(lightArea))
        {
            lights.push_back(pair.first);
        }
    }
    return lights;
}

void Lighting2DDeferred::renderLights(const std::vector<Light2DID>& lights, const std::vector<Shape2D*>& occluders)
{
    const Camera2D& camera = m_renderer.getDefaultCamera();
    const Rect2D cameraRect = camera.getWorldRect();
    const float camScale = camera.getZoom();
    const glm::vec2 camPosPX = glm::vec2(cameraRect.x, cameraRect.y);

    for (const Light2DID lightID : lights)
    {
        const Light2D& light = m_lights[lightID];
        const float lightRadius = light.lightWidth;
        const glm::vec2 lightPos = light.position;
        const glm::vec2 lightPosPX = lightPos * camScale;
        const glm::vec2 lightPosScrn = lightPosPX - (camPosPX * camScale);
        const Rect2D lightScrnRect = Rect2D(lightPosScrn, glm::vec2(lightRadius * camScale * 2.0f, lightRadius * camScale * 2.0f));

        // Dynamically scale the light rendering resolution
        const float scaleX = m_tempFrameBuffer.getWidth() / lightScrnRect.w;
        const float scaleY = m_tempFrameBuffer.getHeight() / lightScrnRect.h;
        const float renderScale = fminf(scaleX, scaleY);
        const float lightRadiusPX = lightRadius * camScale * renderScale;
        const float lightDiameter = lightRadiusPX * 2.f;

        m_tempFrameBuffer.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, lightDiameter, lightDiameter);

        // Render light circle or beam
        const glm::mat4 projection = glm::ortho<float>(0.f, lightDiameter, 0.f, lightDiameter, -1.f, 1.f);
        //if (light.subType == LIGHT_SPOT)
        {
            draw2DLightCircle(lightRadiusPX, light.lightColor, projection);
        }
//      else if (theLight->subType == LIGHT_BEAM)
//      { LightRenderer::DrawLightBeam( cpv(0.0f, lightFBOPos.y), (float)(theLight->lightArc), (float)lightRadiusPX, (float)lightAngle, segs, lightColor); }
        for (const Shape2D* shape : occluders)
        {
            const glm::vec2 lightBottomLeft = lightPos - glm::vec2(lightRadius, lightRadius);
            if (shape->getType() == Shape2DType::Circle)
            {
                const glm::vec2 circlePos = (shape->getPosition() - lightBottomLeft) * camScale * renderScale;
                const float circleRadius = ((Shape2DCircle*)shape)->getRadius() * camScale * renderScale;
                drawCircleShadow(lightRadiusPX, circleRadius, circlePos, projection);
            }
        }

        // The temporary framebuffer now contains the rendered light and shadows, render to final framebuffer
        m_frameBuffer.bind();
        //glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer.getFBOHandle());
        const glm::ivec2 renderSize = m_renderer.getRenderCore().getRenderResolution();
        const glm::mat4 projection2D = glm::ortho<float>(0.f, renderSize.x, 0.f, renderSize.y, -1.f, 1.f);
        glViewport(0, 0, renderSize.x, renderSize.y);

        // FBO texture coordinates range 0.0 to 1.0
        const float texCoordBottom = 0.0f;
        const float texCoordLeft = 0.0f;
        const float texCoordTop = lightDiameter / (float)m_tempFrameBuffer.getHeight();
        const float texCoordRight = lightDiameter / (float)m_tempFrameBuffer.getWidth();

        m_renderer.getRenderCore().setupTempVertBuffer<TexturedVertex3DData>(m_texturedTriVertsBuffer, 4);
        TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)m_texturedTriVertsBuffer.data;
        dataPtr[0] = { glm::vec3(lightScrnRect.x, lightScrnRect.y, 0.0), glm::vec2(texCoordLeft, texCoordBottom) };
        dataPtr[1] = { glm::vec3(lightScrnRect.x + lightScrnRect.w, lightScrnRect.y, 0.0), glm::vec2(texCoordRight, texCoordBottom) };
        dataPtr[2] = { glm::vec3(lightScrnRect.x + lightScrnRect.w, lightScrnRect.y + lightScrnRect.h, 0.0), glm::vec2(texCoordRight, texCoordTop) };
        dataPtr[3] = { glm::vec3(lightScrnRect.x, lightScrnRect.y + lightScrnRect.h, 0.0), glm::vec2(texCoordLeft, texCoordTop) };
        m_renderer.getRenderCore().draw(m_texturedVertsShaderID, m_tempFrameBuffer.getTextureID(), m_texturedVertsDrawDataID, projection2D, DrawMode::TriangleFan, dataPtr, 0, 4, BLEND_MODE_ADDITIVE, DEPTH_MODE_DISABLED);
    }
}

void Lighting2DDeferred::draw2DLightCircle(const float radius, const Color& lightColor, const glm::mat4& projection)
{
    const uint32_t segs = std::max<uint32_t>((radius * 2.f * M_PI) / 16, 8);
    const float coef = (float)(2.0f * (M_PI / segs));
    const uint32_t numVerts = segs * 3;
    const glm::vec2 center = glm::vec2(radius, radius);

    m_renderer.getRenderCore().setupTempVertBuffer<ColoredVertex3DData>(m_coloredTriVertsBuffer, numVerts);
    ColoredVertex3DData* vertices = (ColoredVertex3DData*)m_coloredTriVertsBuffer.data;
    m_coloredTriVertsBuffer.count = numVerts;

    for (uint32_t i = 0; i < segs; i++)
    {
        const float segmentRads = (i * coef);
        const float segmentRadsNext = ((i + 1) * coef);
        const float x1 = cosf(segmentRads) * radius;
        const float y1 = sinf(segmentRads) * radius;
        const float x2 = cosf(segmentRadsNext) * radius;
        const float y2 = sinf(segmentRadsNext) * radius;
        const glm::vec3 segmentPos = glm::vec3(center.x + x1, center.y + y1, 0.f);
        const glm::vec3 segmentPosNext = glm::vec3(center.x + x2, center.y + y2, 0.f);
        vertices[i * 3] = { glm::vec3(center.x, center.y, 0.f), lightColor };
        vertices[i * 3 + 1] = { segmentPos, COLOR_NONE };
        vertices[i * 3 + 2] = { segmentPosNext, COLOR_NONE };
    }

    m_renderer.getRenderCore().draw(m_coloredVertsShaderID, 0, m_coloredVertsDrawDataID, projection, DrawMode::Triangles, m_coloredTriVertsBuffer.data, 0, m_coloredTriVertsBuffer.count, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);
}

void Lighting2DDeferred::drawCircleShadow(const float lightRadius, const float circleRadius, const glm::vec2& circlePos, const glm::mat4& projection)
{
    const uint32_t shadowSegs = 32;
    const glm::vec2 lightPos = glm::vec2(lightRadius, lightRadius);
    const glm::vec2 lightToCircle = circlePos - lightPos;
    const glm::vec2 shadowPos = circlePos + lightToCircle;
    const glm::vec2 normal = glm::normalize(glm::vec2(lightToCircle.y, -1.f * lightToCircle.x));
    const float shadowAngleIn = atan2(normal.y, normal.x);
    const float shadowRatio = 1.f - ((glm::distance(lightPos, circlePos) - circleRadius) / lightRadius);
    const float shadowAngleOut = (shadowAngleIn + M_PI_2 - (M_PI_2 * shadowRatio));
    const float coefInside = (float)(M_PI / shadowSegs);
    const float coefOutside = shadowRatio * (M_PI / shadowSegs); // part of circle

    const uint32_t numVerts = (shadowSegs + 1) * 2;
    m_renderer.getRenderCore().setupTempVertBuffer<ColoredVertex3DData>(m_coloredTriVertsBuffer, numVerts);
    ColoredVertex3DData* vertices = (ColoredVertex3DData*)m_coloredTriVertsBuffer.data;
    m_coloredTriVertsBuffer.count = numVerts;
    for (uint32_t i = 0; i <= shadowSegs; i++)
    {
        const float radsIn = i * coefInside + shadowAngleIn;
        const glm::vec2 inside = glm::vec2(cosf(radsIn), sinf(radsIn)) * circleRadius + circlePos;
        vertices[i * 2] = { glm::vec3(inside.x, inside.y, 0.f), COLOR_BLACK };
        const float radsOut = (i * coefOutside) + shadowAngleOut;
        const glm::vec2 outside = glm::vec2(cosf(radsOut), sinf(radsOut)) * lightRadius * shadowRatio + shadowPos;
        vertices[i * 2 + 1] = { glm::vec3(outside.x, outside.y, 0.f), COLOR_BLACK };
    }
    m_renderer.getRenderCore().draw(m_coloredVertsShaderID, 0, m_coloredVertsDrawDataID, projection, DrawMode::TriangleStrip, m_coloredTriVertsBuffer.data, 0, m_coloredTriVertsBuffer.count, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);
}