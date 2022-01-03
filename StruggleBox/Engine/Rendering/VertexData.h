#pragma once

#include "Allocator.h"
#include "ArenaOperators.h"
#include "RendererDefines.h"
#include <memory>

class BaseVertexData
{
public:
	BaseVertexData(const size_t size, const VertexDataType type)
		: m_size(size)
		, m_count(0)
		, m_type(type)
	{}

	virtual bool isAccessible() const { return false; }

	void clear() { m_count = 0; }
	const size_t getSize() const { return m_size; }
	const size_t getCount() const { return m_count; }
	const VertexDataType getType() const { return m_type; }
protected:
	size_t m_size;
	size_t m_count;
	VertexDataType m_type;
};

template <typename T>
class VertexData : public BaseVertexData
{
public:
	VertexData(const size_t size, const VertexDataType type, Allocator& allocator)
		: BaseVertexData(size, type)
		, m_allocator(allocator)
		, m_data(nullptr)
	{ 
		if (size > 0)
		{
			m_data = CUSTOM_NEW_ARRAY(T, size, m_allocator);
		}
	}

	virtual ~VertexData()
	{
		if (m_data)
		{
			CUSTOM_DELETE_ARRAY(m_data, m_allocator);
		}
	}

	bool isAccessible() const override { return true; }

	void buffer(const T* data, const size_t count)
	{
		assert(data && count);
		if (count == 0 || data == nullptr)
			return;
		if (m_count + count > m_size)
		{
			Log::Warn("VertexData tried to buffer data which does not fit");
			return;
		}
		memcpy(&m_data[m_count], data, sizeof(T)*count);
		m_count += count;
	}

	void resize(const size_t size, const bool retainData = false)
	{
		T* oldData = m_data;

		if (size == 0)
		{
			m_data = nullptr;

		}
		else
		{
			m_data = CUSTOM_NEW_ARRAY(T, size, m_allocator);
		}
		if (oldData != nullptr)
		{
			if (retainData && m_count > 0)
			{
				size_t retainCount = size >= m_count ? m_count : size;
				memcpy(m_data, oldData, sizeof(T)*retainCount);
				m_count = retainCount;
			}
			else
			{
				m_count = 0;
			}
			CUSTOM_DELETE_ARRAY(oldData, m_allocator);
		}
		else
		{
			m_count = 0;
		}
		 
		m_size = size;
	}

	void remove(const size_t removeIndex)
	{
		if (removeIndex != m_count - 1)
		{
			T* oldData = &m_data[removeIndex];
			T* lastData = &m_data[m_count - 1];
			memcpy(oldData, lastData, sizeof(T));
		}
		m_count--;
	}

	T* getData() { return m_data; }

private:
	Allocator& m_allocator;
	T* m_data;
};
