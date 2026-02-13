#include "DrawDataCache.h"

#include "GLErrorUtil.h"
#include "Log.h"

static DrawData no_data = { 0, 0, 0, 0, 0, 0, 0 };

DrawDataCache::DrawDataCache()
{
}

DrawDataCache::~DrawDataCache()
{
}

void DrawDataCache::terminate()
{
    while (!m_meshDataCache.empty())
    {
        auto it = m_meshDataCache.begin();
        DrawDataID dataID = it->first;
        destroyDrawData(dataID);
    }
}
//
//DrawDataID DrawDataCache::createDrawData(const DrawDataType type)
//{
//    if (isDrawDataTypeInstanced(type))
    //{
    //    Log::Error("[DrawDataCache::createDrawData] Wrong type of data, use createInstancedDrawData instead!");
    //    return 0;
    //}
//    uint32_t VAO = 0;
//    glGenVertexArrays(1, &VAO);
//    uint32_t VBO = 0;
//    glGenBuffers(1, &VBO);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    configureVBO(type);
//    glBindVertexArray(0);
//
//    m_nextMeshDataID++;
//    DrawData data = { type, VAO, VBO, 0, type, 0, 0 };
//    m_meshDataCache[m_nextMeshDataID] = data;
//    return m_nextMeshDataID;
//}

DrawDataID DrawDataCache::createDrawData(const VertexConfig& config)
{
    if (!config.attributeCount)
    {
        Log::Error("[DrawDataCache::createDrawData] No vertex data attributes, check config!");
        return 0;
    }
    uint32_t VAO = 0;
    uint32_t VBO = 0;
    uint32_t vertexDataSize = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    configureVBO(config, vertexDataSize);
    glBindVertexArray(0);

    DrawData data;
    data.handleVAO = VAO;
    data.handleVBO = VBO;
    data.vertexCount = 0;
    data.vertexDataSize = vertexDataSize;
    data.handleIBO = 0;
    data.instanceCount = 0;
    data.instanceDataSize = 0;

    m_nextMeshDataID++;
    m_meshDataCache[m_nextMeshDataID] = data;
    return m_nextMeshDataID;
}

DrawDataID DrawDataCache::createInstancedDrawData(const VertexConfig& instanceConfig, const VertexConfig& config)
{
    if (!config.attributeCount)
    {
        Log::Error("[DrawDataCache::createInstancedDrawData] No vertex data attributes, check config!");
        return 0;
    }
    if (!instanceConfig.attributeCount)
    {
        Log::Error("[DrawDataCache::createInstancedDrawData] No instance data attributes, check config!");
        return 0;
    }
    uint32_t VAO = 0;
    uint32_t VBO = 0;
    uint32_t vertexDataSize = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    CHECK_GL_ERROR();
    configureVBO(config, vertexDataSize);
    CHECK_GL_ERROR();

    uint32_t IBO = 0;
    uint32_t instanceDataSize = 0;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ARRAY_BUFFER, IBO);
    CHECK_GL_ERROR();
    configureIBO(instanceConfig, instanceDataSize, config.attributeCount);
    CHECK_GL_ERROR();
    glBindVertexArray(0);

    DrawData data;
    data.handleVAO = VAO;
    data.handleVBO = VBO;
    data.vertexCount = 0;
    data.vertexDataSize = vertexDataSize;
    data.handleIBO = IBO;
    data.instanceCount = 0;
    data.instanceDataSize = instanceDataSize;

    m_nextMeshDataID++;
    m_meshDataCache[m_nextMeshDataID] = data;
    return m_nextMeshDataID;
}

//
//DrawDataID DrawDataCache::createInstancedDrawData(const DrawDataType type, const DrawDataType instanceType)
//{
//    if (!isDrawDataTypeInstanced(type))
//    {
//        Log::Error("[DrawDataCache::createInstancedDrawData] Wrong type of instanced data, use createDrawData instead!");
//        return 0;
//    }
//
//    uint32_t VAO = 0;
//    glGenVertexArrays(1, &VAO);
//    uint32_t VBO = 0;
//    glGenBuffers(1, &VBO);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    configureVBO(instanceType);
//    uint32_t IBO = 0;
//    glGenBuffers(1, &IBO);
//    glBindBuffer(GL_ARRAY_BUFFER, IBO);
//    if (type == DrawDataType::ColoredInstances)
//    {
//        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredInstanceData), (void*)0);
//        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredInstanceData), (void*)(4 * sizeof(float)));
//        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredInstanceData), (void*)(8 * sizeof(float)));
//        glEnableVertexAttribArray(2);
//        glEnableVertexAttribArray(3);
//        glEnableVertexAttribArray(4);
//        glVertexAttribDivisor(2, 1);
//        glVertexAttribDivisor(3, 1);
//        glVertexAttribDivisor(4, 1);
//    }
//    else if (type == DrawDataType::Instances3D)
//    {
//        const uint32_t attributes = instanceType == DrawDataType::TexturedPBRVerts ? 3 : 1;
//        glVertexAttribPointer(attributes + 1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData3D), (void*)0);
//        glVertexAttribPointer(attributes + 2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData3D), (void*)(3 * sizeof(float)));
//        glVertexAttribPointer(attributes + 3, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData3D), (void*)(7 * sizeof(float)));
//        glEnableVertexAttribArray(attributes + 1);
//        glEnableVertexAttribArray(attributes + 2);
//        glEnableVertexAttribArray(attributes + 3);
//        glVertexAttribDivisor(attributes + 1, 1);
//        glVertexAttribDivisor(attributes + 2, 1);
//        glVertexAttribDivisor(attributes + 3, 1);
//    }
//    else if (type == DrawDataType::Instances2D)
//    {
//        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ImpostorVertexData), (void*)0);
//        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ImpostorVertexData), (void*)(3 * sizeof(float)));
//        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ImpostorVertexData), (void*)(4 * sizeof(float)));
//        glEnableVertexAttribArray(2);
//        glEnableVertexAttribArray(3);
//        glEnableVertexAttribArray(4);
//        glVertexAttribDivisor(2, 1);
//        glVertexAttribDivisor(3, 1);
//        glVertexAttribDivisor(4, 1);
//    }
//    else if (type == DrawDataType::TexturedVoxelInstances)
//    {
//        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVoxelInstanceData), (void*)0);
//        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(TexturedVoxelInstanceData), (void*)(3 * sizeof(float)));
//        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(TexturedVoxelInstanceData), (void*)(4 * sizeof(float)));
//        glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVoxelInstanceData), (void*)(8 * sizeof(float)));
//        glEnableVertexAttribArray(4);
//        glEnableVertexAttribArray(5);
//        glEnableVertexAttribArray(6);
//        glEnableVertexAttribArray(7);
//        glVertexAttribDivisor(4, 1);
//        glVertexAttribDivisor(5, 1);
//        glVertexAttribDivisor(6, 1);
//        glVertexAttribDivisor(7, 1);
//    }
//
//    glBindVertexArray(0);
//
//    m_nextMeshDataID++;
//    DrawData data = { type, VAO, VBO, 0, instanceType, IBO, 0 };
//    m_meshDataCache[m_nextMeshDataID] = data;
//    return m_nextMeshDataID;
//}

void DrawDataCache::destroyDrawData(const DrawDataID dataID)
{
    auto it = m_meshDataCache.find(dataID);
    if (it == m_meshDataCache.end())
    {
        return;
    }
    DrawData& data = it->second;
    if (data.handleIBO)
    {
        glDeleteBuffers(1, &data.handleIBO);
    }
    glDeleteBuffers(1, &data.handleVBO);
    glDeleteVertexArrays(1, &data.handleVAO);
    m_meshDataCache.erase(it);
}

DrawData& DrawDataCache::getDrawData(const DrawDataID dataID)
{
    auto it = m_meshDataCache.find(dataID);
    if (it == m_meshDataCache.end())
    {
        Log::Error("[DrawDataCache::getDrawData] No DrawData found for ID %i", dataID);
        return no_data;
    }
    return it->second;
}

bool DrawDataCache::isInstancedDrawData(const DrawData& data) const
{
    return data.handleIBO != 0; // Sort of hacky but should work
}

//void DrawDataCache::configureVBO(const DrawDataType type)
//{
//    if (type == DrawDataType::ColoredVerts)
//    {
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex3DData), (void*)0);
//        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex3DData), (void*)(3 * sizeof(float)));
//        glEnableVertexAttribArray(0);
//        glEnableVertexAttribArray(1);
//    }
//    else if (type == DrawDataType::TexturedVerts)
//    {
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3DData), (void*)0);
//        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3DData), (void*)(3 * sizeof(float)));
//        glEnableVertexAttribArray(0);
//        glEnableVertexAttribArray(1);
//    }
//    else if (type == DrawDataType::TexturedVerts2D)
//    {
//        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3DData2D), (void*)0);
//        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex3DData2D), (void*)(2 * sizeof(float)));
//        glEnableVertexAttribArray(0);
//        glEnableVertexAttribArray(1);
//    }
//    else if (type == DrawDataType::ImpostorVerts)
//    {
//        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ImpostorVertexData), (void*)0);
//        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ImpostorVertexData), (void*)(4 * sizeof(float)));
//        glEnableVertexAttribArray(0);
//        glEnableVertexAttribArray(1);
//    }
//    else if (type == DrawDataType::TexturedPBRVerts)
//    {
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedPBRVertexData), (void*)0);
//        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedPBRVertexData), (void*)(3 * sizeof(float)));
//        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedPBRVertexData), (void*)(6 * sizeof(float)));
//        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedPBRVertexData), (void*)(9 * sizeof(float)));
//        glEnableVertexAttribArray(0);
//        glEnableVertexAttribArray(1);
//        glEnableVertexAttribArray(2);
//        glEnableVertexAttribArray(3);
//    }
//    else if (type == DrawDataType::LightVerts)
//    {
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LightVertexData), (void*)0);
//        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(LightVertexData), (void*)(3 * sizeof(float)));
//        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(LightVertexData), (void*)(5 * sizeof(float)));
//        glEnableVertexAttribArray(0);
//        glEnableVertexAttribArray(1);
//        glEnableVertexAttribArray(2);
//    }
//}

void DrawDataCache::configureVBO(const VertexConfig& config, uint32_t& vertexDataSize)
{
    for (uint8_t i = 0; i < config.attributeCount; i++)
    {
        vertexDataSize += sizeof(GLfloat) * config.attributeSizes[i];
    }
    uint32_t attributeOffset = 0;
    for (uint8_t i = 0; i < config.attributeCount; i++)
    {
        const uint32_t attributeSize = config.attributeSizes[i];
        glVertexAttribPointer(i, attributeSize, GL_FLOAT, GL_FALSE, vertexDataSize, (void*)attributeOffset);
        glEnableVertexAttribArray(i);
        attributeOffset += sizeof(GLfloat) * attributeSize;
    }
}
void DrawDataCache::configureIBO(const VertexConfig& config, uint32_t& instanceDataSize, uint8_t vertexAttributeCount)
{
    for (uint8_t i = 0; i < config.attributeCount; i++)
    {
        instanceDataSize += sizeof(GLfloat) * config.attributeSizes[i];
    }
    uint32_t attributeOffset = 0;
    for (uint8_t i = 0; i < config.attributeCount; i++)
    {
        const uint32_t j = i + vertexAttributeCount; // instanced attribute index
        const uint32_t attributeSize = config.attributeSizes[i];
        glVertexAttribPointer(j, config.attributeSizes[i], GL_FLOAT, GL_FALSE, instanceDataSize, (void*)attributeOffset);
        glEnableVertexAttribArray(j);
        glVertexAttribDivisor(j, 1);
        attributeOffset += sizeof(GLfloat) * attributeSize;
    }
}