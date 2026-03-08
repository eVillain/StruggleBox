#pragma once

#include "LinearAllocator.h"
#include "ThreadPool.h"
#include "TextureLoader.h"
#include "TextureCache.h"
#include "TextureAtlasCache.h"
#include "ShaderCache.h"
#include "FrameCache.h"
#include "TextAtlasCache.h"
#include "DrawDataCache.h"
#include "RendererDefines.h"
#include "DrawParameters.h"
#include <functional>

class Allocator;
class LinearAllocator;
class Texture2D;
class TextureAtlas;
class Shader;
class TextAtlas;

struct TempVertBuffer
{
    void* data;
    size_t count;
    size_t capacity;
};

class RenderCore
{
public:
    RenderCore(Allocator& allocator, ThreadPool& threadPool);
    ~RenderCore();

    void terminate();

    TextureID getTextureID(const std::string& textureName, bool load);
    void getTextureIDAsync(const std::string& textureName, const std::function<void(TextureID)>& callback);
    const Texture2D* getTextureByID(const TextureID textureID);
    void removeTexture(const TextureID textureID);

    TextureAtlasID getTextureAtlasID(const std::string& textureAtlasName);
    const TextureAtlas* getTextureAtlasByID(const TextureAtlasID textureAtlasID);
    TextureAtlasID getTextureAtlasIDForFrame(const std::string& frameName);
    void removeTextureAtlas(const TextureAtlasID textureAtlasID);

    ShaderID getShaderID(const std::string& shaderVertexName, const std::string& shaderFragName);
    ShaderID getShaderID(const std::string& shaderGeometryName, const std::string& shaderVertexName, const std::string& shaderFragName);
    ShaderID getShaderIDFromSource(const std::string& shaderVertexSource, const std::string& shaderFragmentSource, const std::string& shaderName);
    ShaderID getShaderIDFromSource(const std::string& shaderGeometrySource, const std::string& shaderVertexSource, const std::string& shaderFragmentSource, const std::string& shaderName);
    const Shader* getShaderByID(const ShaderID shaderID);
    void removeShader(const ShaderID shaderID);

    TextAtlasID getTextAtlasID(const std::string& textAtlasName, const uint8_t fontHeight);
    TextAtlas* getTextAtlasByID(const TextAtlasID textAtlasID);
    void removeTextAtlas(const TextAtlasID textAtlasID);

    DrawDataID createDrawData(const VertexConfig& config);
    DrawDataID createInstancedDrawData(const VertexConfig& instanceConfig, const VertexConfig& config);
    DrawData& getDrawData(const DrawDataID dataID);

    template <typename T>
    void setupTempVertBuffer(TempVertBuffer& buffer, const size_t capacity)
    {
        setupTempVertBuffer(buffer, capacity, sizeof(T));
    }
    void setupTempVertBuffer(TempVertBuffer& buffer, const size_t capacity, const uint32_t dataSize);
    void clearTempVertBuffer(TempVertBuffer& buffer);

    void* allocFrameData(const size_t size);

    void upload(const DrawDataID drawDataID, const void* data, const size_t count);
    void draw(
        const ShaderID shaderID,
        const TextureID textureID,
        const DrawDataID drawDataID,
        const glm::mat4 modelMatrix,
        const DrawMode drawMode,
        const void* data,
        const uint32_t rangeStart,
        const uint32_t rangeEnd,
        const BlendMode blendMode,
        const DepthMode depthMode);
    void draw(
        const DrawParameters& drawParams,
        const glm::mat4& matrix,
        const DrawMode drawMode,
        const void* data,
        const size_t count);

    void beginFrame();
    void endFrame();

    void setRenderResolution(const glm::ivec2& resolution) { m_renderResolution = resolution; }
    const glm::ivec2& getRenderResolution() const { return m_renderResolution; }

    TextureCache& getTextureCache() { return m_textureCache; }
    Allocator& getAllocator() { return m_allocator; }

private:
    Allocator& m_allocator;
    LinearAllocator m_frameAllocator;

    TextureLoader m_textureLoader;
    TextureCache m_textureCache;
    TextureAtlasCache m_atlasCache;
    ShaderCache m_shaderCache;
    FrameCache m_frameCache;
    TextAtlasCache m_textAtlasCache;
    DrawDataCache m_drawDataCache;

    glm::ivec2 m_renderResolution;
};
