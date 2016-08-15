#ifndef VERTEX_DATA_H
#define VERTEX_DATA_H
#include <memory>

template <typename T>
class VertexData
{
public:
	VertexData(const unsigned int size) :
		_size(size),
		_count(0),
		_data(nullptr)
	{ 
		if (size > 0)
			_data = new T[size];
	}

	~VertexData()
	{
		delete [] _data;
		_data = nullptr;
	}

	void buffer(
		const T* data,
		const unsigned int count)
	{
		if (_count + count > _size)
			return;
		memcpy(&_data[_count], data, sizeof(T)*count);
		_count += count;
	}

	void resize(const unsigned int size, const bool retainData=false)
	{
		T* oldData = _data;

		if (size > 0)
			_data = new T[size];
		else
			_data = nullptr;

		if (oldData != nullptr)
		{
			if (retainData && _count > 0)
			{
				unsigned int retainCount = size >= _count ? _count : size;
				memcpy(_data, oldData, sizeof(T)*retainCount);
				_count = retainCount;
			}
			else
			{
				_count = 0;
			}
			delete[] oldData;
		}
		else
		{
			_count = 0;
		}
		 
		_size = size;
	}

	void remove(const unsigned int removeIndex)
	{
		if (removeIndex != _count - 1)
		{
			T* oldData = &_data[removeIndex];
			T* lastData = &_data[_count - 1];
			memcpy(oldData, lastData, sizeof(T));
		}
		_count--;
	}

	void clear() { _count = 0; }
	const unsigned int getSize() const { return _size; }
	const unsigned int getCount() const { return _count; }
	T* getData() { return _data; }

private:
	unsigned int _size;
	unsigned int _count;
	T* _data;
};

#endif // !VERTEX_DATA_H