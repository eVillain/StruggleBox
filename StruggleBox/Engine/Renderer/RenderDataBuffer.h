#pragma once

#include "MathUtils.h"
#include "RenderCore.h"
#include <map>

template <class KeyType, class DataType>
class RenderDataBuffer
{
public:
    RenderDataBuffer(RenderCore& renderCore) : m_renderCore(renderCore) {}

	DataType* buffer(const size_t count, const KeyType key)
	{
        auto it = m_data.find(key);
        if (it == m_data.end())
        {
            const size_t bufferCount = MathUtils::Max(count, (size_t)1024);
            TempVertBuffer buffer;
            m_renderCore.setupTempVertBuffer<DataType>(buffer, bufferCount);
            m_data[key] = buffer;
        }
        const TempVertBuffer& oldBuffer = m_data.at(key);
        if (oldBuffer.count + count > oldBuffer.capacity)
        {
            const uint32_t nextBufferSize = MathUtils::round_up_to_power_of_2(oldBuffer.count + count);
            TempVertBuffer newBuffer;
            m_renderCore.setupTempVertBuffer<DataType>(newBuffer, nextBufferSize);
            memcpy(newBuffer.data, oldBuffer.data, oldBuffer.count * sizeof(DataType));
            newBuffer.count = oldBuffer.count;
            m_data[key] = newBuffer;
            Log::Warn("[RendererDataBuffer::buffer] Trying to buffer too many verts, increasing buffer size to %lu!", nextBufferSize);
        }

        TempVertBuffer& buffer = m_data.at(key);
        DataType* dataPtr = (DataType*)buffer.data;
        dataPtr += buffer.count;
        buffer.count += count;
        return dataPtr;
	}

    void clear()
    {
        m_data.clear();
    }

    const std::map<KeyType, TempVertBuffer>& getData() const { return m_data; }
private:
    RenderCore& m_renderCore;
	std::map<KeyType, TempVertBuffer> m_data;
};
