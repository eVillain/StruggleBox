#ifndef LINEARALLOCATOR_H
#define LINEARALLOCATOR_H

#include "Allocator.h"

// A Linear Allocator is the simplest and fastest type of allocator.
// Pointers to the start of the allocator, to the first free address and the total size of the allocator are maintained.
// New allocations simply move the pointer to the first free address forward.
// Individual deallocations cannot be made in linear allocators, instead use clear() to completely clear the memory used by the allocator.

class LinearAllocator : public Allocator
{
public:
	LinearAllocator(size_t size, void* start);
	~LinearAllocator();

	void* allocate(size_t size, uint8_t alignment = 4) override;
		
	void deallocate(void* p) override;

	void clear();

private:
	LinearAllocator(const LinearAllocator&); //Prevent copies because it might cause errors
	LinearAllocator& operator=(const LinearAllocator&);

	void* _current_pos;
};

namespace allocator
{
	inline LinearAllocator* newLinearAllocator(size_t size, Allocator& allocator)
	{
		void* p = allocator.allocate(size+sizeof(LinearAllocator), __alignof(LinearAllocator));
		return new (p) LinearAllocator(size, pointer_math::add(p, sizeof(LinearAllocator)));
	}

	inline void deleteLinearAllocator(LinearAllocator& linearAllocator, Allocator& allocator)
	{
		linearAllocator.~LinearAllocator();

		allocator.deallocate(&linearAllocator);
	}
};

#endif