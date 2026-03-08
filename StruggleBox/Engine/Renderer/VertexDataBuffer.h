#pragma once

#include "MathUtils.h"
#include "RenderCore.h"

template <class DataType>
class VertexDataBuffer
{
public:
    VertexDataBuffer(RenderCore& renderCore) : m_renderCore(renderCore) { m_data = { nullptr, 0, 0 }; }

    DataType* buffer(const size_t count)
    {
        if (m_data.count + count > m_data.capacity)
        {
            const uint32_t nextBufferSize = MathUtils::round_up_to_power_of_2(m_data.count + count);
            TempVertBuffer newBuffer;
            m_renderCore.setupTempVertBuffer<DataType>(newBuffer, nextBufferSize);
            memcpy(newBuffer.data, m_data.data, m_data.count * sizeof(DataType));
            newBuffer.count = m_data.count;
            m_data = newBuffer;
            //Log::Warn("[VertexDataBuffer::buffer] Trying to buffer too many verts, increasing buffer size to %lu!", nextBufferSize);
        }

        DataType* dataPtr = (DataType*)m_data.data;
        dataPtr += m_data.count;
        m_data.count += count;
        return dataPtr;
    }

    void clear()
    {
        m_renderCore.clearTempVertBuffer(m_data);
    }

    const TempVertBuffer& getData() const { return m_data; }
private:
    RenderCore& m_renderCore;
    TempVertBuffer m_data;
};
