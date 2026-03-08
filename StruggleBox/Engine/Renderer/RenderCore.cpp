#include "RenderCore.h"

#include "Allocator.h"
#include "ArenaOperators.h"
#include "DefaultShaders.h"
#include "GLErrorUtil.h"
#include "GLUtils.h"
#include "Log.h"
#include "TextureAtlas.h"
#include "TextureAtlasLoader.h"
#include "TextAtlas.h"
#include "TextAtlasLoader.h"
#include "Texture2D.h"
#include "Shader.h"
#include "ShaderLoader.h"
#include <GL/glew.h>

const size_t FRAME_ALLOCATOR_SIZE = 64 * 1024 * 1024;
static uint32_t GL_DRAW_MODES[] = {
    GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN
};

RenderCore::RenderCore(Allocator& reanderAllocator, ThreadPool& threadPool)
: m_allocator(reanderAllocator)
, m_frameAllocator(FRAME_ALLOCATOR_SIZE, reanderAllocator.allocate(FRAME_ALLOCATOR_SIZE))
, m_textureLoader(reanderAllocator, threadPool)
, m_textureCache()
, m_atlasCache()
, m_shaderCache()
, m_frameCache()
, m_textAtlasCache()
, m_drawDataCache()
, m_renderResolution(1920, 1080)
{
}

RenderCore::~RenderCore()
{
}

void RenderCore::terminate()
{
    // TODO: get all cached things and delete them :)
    for (auto pair : m_textureCache.m_textures)
    {
        Texture2D* texture = pair.second;
        CUSTOM_DELETE(texture, m_allocator);
    }
    m_textureCache.m_textures.clear();
    m_textureCache.m_textureNames.clear();

    for (auto pair : m_atlasCache.m_atlases)
    {
        TextureAtlas* atlas = pair.second;
        CUSTOM_DELETE(atlas, m_allocator);
    }
    m_atlasCache.m_atlases.clear();
    m_atlasCache.m_atlasNames.clear();

    for (auto pair : m_shaderCache.m_shaders)
    {
        Shader* shader = pair.second;
        CUSTOM_DELETE(shader, m_allocator);
    }
    m_shaderCache.m_shaders.clear();
    m_shaderCache.m_shaderNames.clear();

    for (auto pair : m_textAtlasCache.m_atlases)
    {
        TextAtlas* atlas = pair.second;
        CUSTOM_DELETE(atlas, m_allocator);
    }
    m_textAtlasCache.m_atlases.clear();
    m_textAtlasCache.m_atlasNames.clear();

    m_drawDataCache.terminate();

    m_frameAllocator.clear();
    void* frameAllocatorStart = m_frameAllocator.getStart();
    CUSTOM_DELETE(frameAllocatorStart, m_allocator);
}

TextureID RenderCore::getTextureID(const std::string& textureName, bool load)
{
    TextureID cachedID = m_textureCache.getTextureID(textureName);
    if (cachedID == TextureCache::NO_TEXTURE_ID && load)
    {
        Texture2D* texture = m_textureLoader.loadFromFile(textureName);
        if (texture)
        {
            cachedID = m_textureCache.addTexture(texture, textureName);
        }
    }
    return cachedID;
}

void RenderCore::getTextureIDAsync(const std::string& textureName, const std::function<void(TextureID)>& callback)
{
    TextureID cachedID = m_textureCache.getTextureID(textureName);
    if (cachedID != TextureCache::NO_TEXTURE_ID)
    {
        callback(cachedID);
        return;
    }

    m_textureLoader.asyncLoadFromFile(textureName, [this, textureName, callback](Texture2D* texture) {
        if (texture)
        {
            TextureID cachedID = m_textureCache.addTexture(texture, textureName);
            callback(cachedID);
            //Log::Debug("RenderCore::getTextureIDAsync loaded %s", textureName.c_str());
        }
        else
        {
            Log::Error("RenderCore::getTextureIDAsync failed to load %s", textureName.c_str());
        }
    });
}

const Texture2D* RenderCore::getTextureByID(const TextureID textureID)
{
    return m_textureCache.getTextureByID(textureID);
}

void RenderCore::removeTexture(const TextureID textureID)
{
    Texture2D* texture = m_textureCache.getTextureByID(textureID);
    if (!texture)
    {
        Log::Warn("RenderCore::removeTexture no texture with ID %lu", textureID);
        return;
    }
    CUSTOM_DELETE(texture, m_allocator);

    m_textureCache.removeTexture(textureID);
}

TextureAtlasID RenderCore::getTextureAtlasID(const std::string& textureAtlasName)
{
    TextureAtlasID cachedID = m_atlasCache.getTextureAtlasID(textureAtlasName);
    if (cachedID == TextureAtlasCache::NO_ATLAS_ID)
    {
        TextureAtlas* textureAtlas = TextureAtlasLoader::load(textureAtlasName, m_allocator, *this);
        if (textureAtlas)
        {
            cachedID = m_atlasCache.addTextureAtlas(textureAtlas, textureAtlasName);
            m_frameCache.addAtlas(textureAtlas, cachedID);
            //for (const auto& pair : textureAtlas->getFrames())
            //{
            //    Log::Debug("%s", pair.first.c_str());
            //}
        }
    }
    return cachedID;
}

const TextureAtlas* RenderCore::getTextureAtlasByID(const TextureAtlasID textureAtlasID)
{
    return m_atlasCache.getTextureAtlasByID(textureAtlasID);
}

TextureAtlasID RenderCore::getTextureAtlasIDForFrame(const std::string& frameName)
{
    return m_frameCache.getAtlasForFrame(frameName);
}

void RenderCore::removeTextureAtlas(const TextureAtlasID textureAtlasID)
{
    TextureAtlas* atlas = m_atlasCache.getTextureAtlasByID(textureAtlasID);
    if (!atlas)
    {
        return;
    }
    CUSTOM_DELETE(atlas, m_allocator);
    m_atlasCache.removeTextureAtlas(textureAtlasID);
}

ShaderID RenderCore::getShaderID(const std::string& shaderVertexName, const std::string& shaderFragName)
{
    ShaderID cachedID = m_shaderCache.getShaderID(shaderVertexName, shaderFragName);
    if (cachedID == ShaderCache::NO_SHADER_ID)
    {
        Shader* shader = ShaderLoader::load(shaderVertexName, shaderFragName, m_allocator);
        if (shader)
        {
            cachedID = m_shaderCache.addShader(shader, shaderVertexName, shaderFragName);
        }
    }
    return cachedID;
}

ShaderID RenderCore::getShaderID(const std::string& shaderGeometryName, const std::string& shaderVertexName, const std::string& shaderFragName)
{
    ShaderID cachedID = m_shaderCache.getShaderID(shaderGeometryName, shaderVertexName, shaderFragName);
    if (cachedID == ShaderCache::NO_SHADER_ID)
    {
        Shader* shader = ShaderLoader::load(shaderGeometryName, shaderVertexName, shaderFragName, m_allocator);
        if (shader)
        {
            cachedID = m_shaderCache.addShader(shader, shaderGeometryName, shaderVertexName, shaderFragName);
        }
    }
    return cachedID;
}

ShaderID RenderCore::getShaderIDFromSource(const std::string& shaderVertexSource, const std::string& shaderFragmentSource, const std::string& shaderName)
{
    ShaderID cachedID = m_shaderCache.getShaderID(shaderName, "");
    if (cachedID == ShaderCache::NO_SHADER_ID)
    {
        Shader* shader = ShaderLoader::loadFromSource(shaderVertexSource, shaderFragmentSource, m_allocator);
        if (shader)
        {
            cachedID = m_shaderCache.addShader(shader, shaderName, "");
        }
    }
    return cachedID;
}

ShaderID RenderCore::getShaderIDFromSource(const std::string& shaderGeometrySource, const std::string& shaderVertexSource, const std::string& shaderFragmentSource, const std::string& shaderName)
{
    ShaderID cachedID = m_shaderCache.getShaderID(shaderName, "");
    if (cachedID == ShaderCache::NO_SHADER_ID)
    {
        Shader* shader = ShaderLoader::loadFromSource(shaderGeometrySource, shaderVertexSource, shaderFragmentSource, m_allocator);
        if (shader)
        {
            cachedID = m_shaderCache.addShader(shader, shaderName, "");
        }
    }
    return cachedID;
}

const Shader* RenderCore::getShaderByID(const ShaderID shaderID)
{
    return m_shaderCache.getShaderByID(shaderID);
}

void RenderCore::removeShader(const ShaderID shaderID)
{
    Shader* shader = m_shaderCache.getShaderByID(shaderID);
    if (!shader)
    {
        return;
    }
    CUSTOM_DELETE(shader, m_allocator);
    m_shaderCache.removeShader(shaderID);
}

TextAtlasID RenderCore::getTextAtlasID(const std::string& textAtlasName, const uint8_t fontHeight)
{
    char buf[256];
#ifdef _WIN32   /* Windows... ಠ_ಠ I don't even */
    sprintf_s(buf, "%s%i", textAtlasName.c_str(), fontHeight);
#else
    sprintf(buf, "%s%i", textAtlasName.c_str(), fontHeight);
#endif
    // Check if we already have an atlas for that font at that size
    const std::string fontNameAndSize = std::string(buf);

    TextAtlasID cachedID = m_textAtlasCache.getTextAtlasID(fontNameAndSize);
    if (cachedID == TextAtlasCache::NO_ATLAS_ID)
    {
        TextAtlas* atlas = TextAtlasLoader::load(textAtlasName, fontHeight, m_allocator, m_textureCache);
        if (atlas)
        {
            cachedID = m_textAtlasCache.addTextAtlas(atlas, fontNameAndSize);
        }
    }
    return cachedID;
}

TextAtlas* RenderCore::getTextAtlasByID(const TextAtlasID textAtlasID)
{
    return m_textAtlasCache.getTextAtlasByID(textAtlasID);
}

void RenderCore::removeTextAtlas(const TextAtlasID textAtlasID)
{
    TextAtlas* atlas = m_textAtlasCache.getTextAtlasByID(textAtlasID);
    if (!atlas)
    {
        return;
    }
    CUSTOM_DELETE(atlas, m_allocator);
    m_textAtlasCache.removeTextureAtlas(textAtlasID);
}

DrawDataID RenderCore::createDrawData(const VertexConfig& config)
{
    return m_drawDataCache.createDrawData(config);
}

DrawDataID RenderCore::createInstancedDrawData(const VertexConfig& instanceConfig, const VertexConfig& config)
{
    return m_drawDataCache.createInstancedDrawData(instanceConfig, config);
}

DrawData& RenderCore::getDrawData(const DrawDataID dataID)
{
    return m_drawDataCache.getDrawData(dataID);
}

void RenderCore::setupTempVertBuffer(TempVertBuffer& buffer, const size_t capacity, const uint32_t dataSize)
{
    buffer.data = allocFrameData(capacity * dataSize);
    buffer.capacity = capacity;
    buffer.count = 0;
}

void RenderCore::clearTempVertBuffer(TempVertBuffer& buffer)
{
    buffer.data = nullptr;
    buffer.capacity = 0;
    buffer.count = 0;
}

void* RenderCore::allocFrameData(const size_t size)
{
    return m_frameAllocator.allocate(size);
}

void RenderCore::upload(const DrawDataID drawDataID, const void* data, const size_t count)
{
    DrawData& drawData = getDrawData(drawDataID);
    const bool dynamic = drawData.handleIBO == 0;
    glBindVertexArray(drawData.handleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, drawData.handleVBO);
    glBufferData(GL_ARRAY_BUFFER, drawData.vertexDataSize * count, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindVertexArray(0);
    drawData.vertexCount = count;
}

void RenderCore::draw(
    const ShaderID shaderID,
    const TextureID textureID,
    const DrawDataID drawDataID,
    const glm::mat4 matrix,
    const DrawMode drawMode,
    const void* data,
    const uint32_t rangeStart,
    const uint32_t rangeEnd,
    const BlendMode blendMode,
    const DepthMode depthMode)
{
    const Shader* shader = getShaderByID(shaderID);
    DrawData& drawData = getDrawData(drawDataID);

    GLUtils::setBlendMode(blendMode);
    GLUtils::setDepthMode(depthMode);

    glBindVertexArray(drawData.handleVAO);
    if (data)
    {
        glBindBuffer(GL_ARRAY_BUFFER, drawData.handleVBO);
        if (!drawData.handleIBO)
        {
            glBufferData(GL_ARRAY_BUFFER, drawData.vertexDataSize * rangeEnd, data, GL_DYNAMIC_DRAW);
            drawData.vertexCount = rangeEnd;
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, drawData.handleIBO);
            glBufferData(GL_ARRAY_BUFFER, drawData.instanceDataSize * rangeEnd, data, GL_DYNAMIC_DRAW);
            drawData.instanceCount = rangeEnd;
        }
    }
    shader->begin();
    if (shader->hasUniform("viewProjection"))
    {
        shader->setUniformM4fv("viewProjection", matrix);
    }
    if (textureID != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureCache.getTextureByID(textureID)->getGLTextureID());
        shader->setUniform1iv("textureMap", 0);
    }
    if (drawData.handleIBO)
    {
        glDrawArraysInstanced(GL_DRAW_MODES[(uint32_t)drawMode], rangeStart, drawData.vertexCount, drawData.instanceCount);
    }
    else
    {
        glDrawArrays(GL_DRAW_MODES[(uint32_t)drawMode], rangeStart, rangeEnd);
    }
}

void RenderCore::draw(
    const DrawParameters& drawParams,
    const glm::mat4& matrix,
    const DrawMode drawMode,
    const void* data,
    const size_t count)
{
    const Shader* shader = getShaderByID(drawParams.shaderID);
    DrawData& drawData = getDrawData(drawParams.drawDataID);

    GLUtils::setBlendMode(drawParams.blendMode);
    GLUtils::setDepthMode(drawParams.depthMode);

    glBindVertexArray(drawData.handleVAO);
    if (data)
    {
        glBindBuffer(GL_ARRAY_BUFFER, drawData.handleVBO);
        if (!drawData.handleIBO)
        {
            glBufferData(GL_ARRAY_BUFFER, drawData.vertexDataSize * count, data, GL_DYNAMIC_DRAW);
            drawData.vertexCount = count;
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, drawData.handleIBO);
            glBufferData(GL_ARRAY_BUFFER, drawData.instanceDataSize * count, data, GL_DYNAMIC_DRAW);
            drawData.instanceCount = count;
        }
    }
    shader->begin();
    if (shader->hasUniform("viewProjection"))
    {
        shader->setUniformM4fv("viewProjection", matrix);
    }
    for (uint8_t i = 0; i < drawParams.textureCount; i++)
    {
        const Texture2D* texture = getTextureByID(drawParams.textureIDs[i]);
        if (!texture)
        {
            continue;
        }
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture->getGLTextureID());
    }
    if (drawData.handleIBO)
    {
        glDrawArraysInstanced(GL_DRAW_MODES[(uint32_t)drawMode], 0, drawData.vertexCount, drawData.instanceCount);
    }
    else
    {
        glDrawArrays(GL_DRAW_MODES[(uint32_t)drawMode], 0, drawData.vertexCount);
    }
}

void RenderCore::beginFrame()
{
    if (m_textureLoader.isLoading())
    {
        m_textureLoader.processQueue();
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderCore::endFrame()
{
    m_frameAllocator.clear();
}

