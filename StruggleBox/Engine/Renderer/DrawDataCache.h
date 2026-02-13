#pragma once

#include "RendererDefines.h"
#include <map>

struct DrawData {
    uint32_t handleVAO;
    uint32_t handleVBO;
    uint32_t vertexCount;
    uint32_t vertexDataSize;
    uint32_t handleIBO;
    uint32_t instanceCount;
    uint32_t instanceDataSize;
};

class DrawDataCache
{
public:
    DrawDataCache();
    ~DrawDataCache();

    void terminate();

    //DrawDataID createDrawData(const DrawDataType type);
    DrawDataID createDrawData(const VertexConfig& config);
    DrawDataID createInstancedDrawData(const VertexConfig& instanceConfig, const VertexConfig& config);
    //DrawDataID createInstancedDrawData(const DrawDataType type, const DrawDataType instanceType);
    void destroyDrawData(const DrawDataID dataID);

    DrawData& getDrawData(const DrawDataID dataID);

    bool isInstancedDrawData(const DrawData& data) const;
private:
    DrawDataID m_nextMeshDataID = 0;

    std::map<DrawDataID, DrawData> m_meshDataCache;
    //void configureVBO(const DrawDataType type);
    void configureVBO(const VertexConfig& config, uint32_t& vertexDataSize);
    void configureIBO(const VertexConfig& config, uint32_t& instanceDataSize, uint8_t vertexAttributeCount);
};
